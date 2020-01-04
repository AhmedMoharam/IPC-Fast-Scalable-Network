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
    
    clients.resize(num_clients);

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
    for(int i =0 ; i < num_clients ; i++){
        clients[i].fd = accept(sfd,(sockaddr *)&client_sockaddr,&client_addr_size);
        if(clients[i].fd < 0 )
            handle_error("ACCEPT");
    }
    //configure each connected device
    for(char i =0 ; i < num_clients ; i++){
        //send unique id for each client
        clients[i].id = i+ 'A';
        clients[i].isConnected = true;
        write(clients[i].fd,(void *)&i,sizeof(i));
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
        for (unsigned int i = 0; i < clients.size() ; i++){
            FD_SET(clients[i].fd, &readfds);
            max_fd = clients[i].fd > max_fd ? clients[i].fd : max_fd ;
        }
        
        int req_num = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if(req_num < 0)
            handle_error("SELECT");
       
        for(unsigned int i = 0; i < clients.size() ; i++){
            if(FD_ISSET(clients[i].fd, &readfds)){
                unsigned int packet_size;
                int cnt = read(clients[i].fd,&packet_size,4);
                //unsigned int cnt = read(clients[i].fd,&data,sizeof(data));
                if(cnt == 0){ //client closed connection and sent EOF
                    clients.erase(clients.begin()+i);
                    i--;
                    if(clients.empty())
                        return;
                    break;
                }else if(cnt == 4){ // normal packet
                    cnt = read(clients[i].fd,&data,packet_size);
                }else if(cnt == -1){ // error ocurred
                    handle_error("read(packet_size)");
                }else{ // unexpected case
                    handle_error("read(packet_size) unexpected case");
                }
                std::cout << "[SERVER] CLIENT[" << clients[i].id << "] sent request with size=" << cnt << std::endl;
                process_data(clients[i].id,data,cnt);
            }
        }

    }
    
}

void server::run_on_thread(){
    server_thread = std::thread (&server::handle_requests,this);

}
void server::wait_on_thread(){
    server_thread.join();
    delete pInstance;
}

void server::send_data(char client_id, void * data, unsigned int size){
    //clients vector are oredered so we can use binary search
    std::vector<server::client>::iterator it = std::lower_bound(clients.begin(),clients.end(),client_id, 
    [](server::client lhs, char rhs) { return lhs.id < rhs; } );
    if(it->id == client_id){
       int cnt = write(it->fd,&size,4);
       if(cnt == -1)
            handle_error("send_data (packet_size)");
        printf("[SERVER] sending data to client[%c] whose fd is %d with size %d\n",it->id,it->fd,size);
        cnt = write(it->fd,data,size);
        if(cnt == -1)
            handle_error("send_data (data)");
    }else
        printf("[SERVER] Warning sending data to client %c failed! client not found!\n",client_id);
    
}


server::~server(){
    close(sfd);
    if(!socket_path.empty())
        unlink(socket_path.c_str());
}