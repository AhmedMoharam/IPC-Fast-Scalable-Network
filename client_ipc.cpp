#include <stdio.h>              /*common.h perror*/
#include <cstdlib>              /*common.h exit*/
#include "common.h"
#include "ipc.h"
#include <sys/types.h>          /* See socket */
#include <sys/socket.h>         /* See socket */
#include <sys/un.h>             /*  sockaddr_un */
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <thread>

client* client::pInstance = nullptr;

client::client()
        : fd(-1),client_id(-1){
    fd = socket(AF_UNIX,SOCK_STREAM,0);
    if(fd < 0)
        handle_error("SOCKET");    
    printf("my fd is %d\n",fd);

 }
 client::~client(){
    //clean up the socket by reading all data sent from server that is still on the socket so that when
    //closing the socket no rubbish would be sent to server as if we sent a request
    char c;
    while (read(fd,&c,1) != -1 && errno != EWOULDBLOCK);
    close(fd);
    //wait for threads before close
    for(unsigned int i=0; i < threads.size(); i++)
        if(threads[i].joinable())
            threads[i].join();
    pInstance = nullptr;
 }
void client::join_group(const char* unix_socket_path){
    //connect your created socket fd to the passive server by knowing its address structure
    sockaddr_un server_sockaddr;
    memset(&server_sockaddr,0,sizeof(server_sockaddr));
    server_sockaddr.sun_family = AF_UNIX;
    strcpy(server_sockaddr.sun_path,unix_socket_path);
    while(connect(fd,(sockaddr *)&server_sockaddr,sizeof(server_sockaddr)) < 0){
        int errsv = errno;
        if(errsv != ENOENT && errsv != ECONNREFUSED) /*wait for the server to bind and listen*/
            handle_error("CONNECT");
    }
    printf("connected\n");

    // getting unique id from server
    unsigned int cnt = read(fd,&client_id,1);
    if(cnt < sizeof(client_id))
        handle_error("READING ID");
    
    // Establish handler for "I/O possible" signal
        //RealTime Signals vals = 32:64 and are queued signals
    int sig = SIGRTMIN;
    struct sigaction sa;
    
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = &client::sigioHandler;
        //mask all other signals to prevent handlers recursively interrupting each other (which would make the output hard to read).
    //sigfillset(&sa.sa_mask);  
    if(sigaction(sig,&sa,NULL) == -1)
        handle_error("SIGACTION");
    
    //Set alternative signal that should be delivered when I/O is possible on fd instead of SIGIO
     if (fcntl(fd, F_SETSIG, sig) == -1)
        handle_error("FCNTL(F_SETSIG)");
    

    //Set owner process that is to receive "I/O possible" signal
    if(fcntl(fd,F_SETOWN, getpid()) == -1)
        handle_error("FCNTL(F_SETOWN)");
    
    // Enable "I/O possible" signaling and make I/O nonblocking for file descriptor
    int flags = fcntl(fd, F_GETFL);
    if (fcntl(fd, F_SETFL, flags | O_ASYNC | O_NONBLOCK) == -1)
        handle_error("FCNTL(F_SETFL)");
    printf("my fd is %d\n",fd);
    pInstance = this;
}
void client::send_data(void * data,unsigned int size){
    //write the size of the packet in 4 bytes
    //so the server knows where the end of the packet is
    write(fd,&size,4);
    write(fd,data,size);
}

char client::get_id(){
    return client_id;
}

void client::sigioHandler(int sig){
    char data[MAX_PACKET_SIZE];
    unsigned int packet_size;
    int cnt = read(pInstance->fd,&packet_size,4);

    if(cnt == -1 && (errno == EWOULDBLOCK || errno== EAGAIN)){
        return;
    }else if(cnt == -1){ //error
        handle_error("read(packet_size)");
    }else if(cnt == 0){
        handle_error("server closed unexpectedly");
    }else if(cnt == 4){ //normal packet read
        cnt = read(pInstance->fd,&data,packet_size);
    }else{
        handle_error("read(packet_size) unexpected case");
    }
    pInstance->threads.push_back(std::thread(&client::handle_requests,pInstance,data,packet_size));

}

void client::handle_requests(char data[MAX_PACKET_SIZE],unsigned int packet_size){
    printf("handling request of size %d\n",packet_size);
    process_data(data,packet_size);
    
}

