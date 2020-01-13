#include <sys/types.h>          /* See socket */
#include <sys/socket.h>         /* See socket */
#include <sys/un.h>             /*  sockaddr_un */
#include <unistd.h>             /* unlink */
#include <iostream>             /*cout*/
#include <thread>
#include <sys/select.h>         /*select*/
#include <algorithm>
#include "ipc.h"



server * server::pInstance = nullptr;

server::server(){
    
    // create the socket
    sfd = socket(AF_UNIX,SOCK_STREAM,0);
    if(sfd < 0)
        handle_error("SOCKET");
}

void server::create_group(const int & num_clients, const char * unix_socket_path ){
    
    //assign a unique name for your socket
    //bind creates the actual file that we unlink at cleaning up step
    sockaddr_un server_sockaddr;
    memset(&server_sockaddr,0,sizeof(sockaddr_un));
    server_sockaddr.sun_family = AF_UNIX;
    strcpy(server_sockaddr.sun_path,unix_socket_path);
    if (bind(sfd,(sockaddr *)&server_sockaddr,sizeof(sockaddr_un)) < 0 )
        handle_error("BIND");
    socket_path = unix_socket_path;

    //mark your socket as a passive socket, a socket waiting for connection, set #ofclients
    // add your socket to the pool of pasive sockets
    if(listen(sfd,num_clients) < 0 )
        handle_error("LISTEN");

    // accept is blocking , untill connections arrives, you can loop to accpet connections one by one
    // upto number of clients stated at listen
    // accept returns the file descriptor of the client requesting connction and its address struct is 
    // filled alongside with its length
    // instead of waiting on all clients first, clients can join on runtime and this could be handled through select
    // must fill client socket addr size with address size before sending

    sockaddr_un client_sockaddr;
    socklen_t client_addr_size = sizeof(sockaddr_un);
    for(char i =0 ; i < num_clients ; i++){
        //assigning id for each client (id = index) as well socket fd for each
        clients[i] = accept(sfd,(sockaddr *)&client_sockaddr,&client_addr_size);
        if(clients[i] < 0 )
            handle_error("ACCEPT");
    }
    //configure each connected device
    for(char i =0 ; i < num_clients ; i++){
        //send unique id for each client
        write(clients[i],(void *)&i,sizeof(i));
    }

    //wait for clients to get ready
    for(char i =0 ; i < num_clients ; i++){
        //receive ready signal
        char c;
        read(clients[i],(void *)&c,sizeof(c));
        if(c != READY_TO_SERVE)
            handle_error("receive ready signal");
    }
}
void server::handle_requests(){
    fd_set readfds;
    char data[MAX_PACKET_SIZE];

    while(1){
    
        //initialize fd set, should be reinitialized every loop
        FD_ZERO(&readfds);
        memset(&data,0,sizeof(data));
        int max_fd = 0;
        for (auto client : clients){
            FD_SET(client.second, &readfds);
            max_fd = client.second > max_fd ? client.second : max_fd ;
        }
        
        int req_num = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if(req_num < 0)
            handle_error("SELECT");
       
        for(auto client : clients){
            if(FD_ISSET(client.second, &readfds)){
                unsigned int packet_size;
                int cnt = read(client.second,&packet_size,4);
                //unsigned int cnt = read(clients[i].fd,&data,sizeof(data));
                if(cnt == 0){ //client closed connection and sent EOF
                    handle_error("should not be here ever");    
                }else if(cnt == 4){ // normal packet
                    cnt = read(client.second,&data,packet_size);
                }else if(cnt == -1){ // error ocurred
                    handle_error("read(packet_size)");
                }else{ // unexpected case
                    handle_error("read(packet_size) unexpected case");
                }
                if( (packet_size == 1) && (*data = CLOSE_CONNECTION)){
                    //grant close ASAP
                    char c = CLOSE_CONNECTION_GRANTED;
                    send_data(client.first,&c,sizeof(c));
                    clients.erase(client.first);
                    if(clients.empty())
                        return;
                    
                }else{
                    process_data(client.first,data,cnt);
                }
            }
        }

    }
    
}

void server::run_on_thread(){
    execution_thread = std::thread (&server::handle_requests,this);

}
void server::wait_on_thread(){
    execution_thread.join();
    delete pInstance;
}

void server::send_data(char client_id, void * data, unsigned int size){
    if(clients.find(client_id) == clients.end()) {
        printf("Warning: Client do not exist\n");
        return;
    }
    char packet[size+4];
    memcpy(packet,&size,4);
    memcpy(packet+4,data,size);
    int cnt = write(clients[client_id],&packet,size+4);
    if(cnt == -1)
        handle_error("send_data (packet_size)");
}


server::~server(){
    close(sfd);
    if(!socket_path.empty())
        unlink(socket_path.c_str());
}