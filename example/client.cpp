#include "../ipc.h"
#include <stdlib.h>
#include <time.h>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <iostream>

struct io_request {
    uint64_t start_addr;
    int size;
    bool is_read;
};

class myclient: public client {
public:
    myclient(uint64_t s,uint64_t e) : start_addr(s),end_addr(e),counter(1)
    {}
    virtual void process_data(void * data,unsigned int size){
        std::cout << "client received: ";
        std::string s((char *)data,size);
        printf("%s",s.c_str());
        std::cout << " count="<< counter++ << std::endl;
        /*
        io_request * req = reinterpret_cast<io_request *>(data);
        std::cout << "start_addr:" << req->start_addr << " size:" << req->size << std::endl;
        */
    }
private:
    uint64_t start_addr;
    uint64_t end_addr;
    int counter;

};

int main(int argc, char *argv[]){
    if(argc != 3)
        exit(EXIT_FAILURE);
    uint64_t  start_address = std::stoi(argv[1]);
    uint64_t  end_address = std::stoi(argv[2]);
    myclient client(start_address,end_address);
    client.join_group();
    io_request request;
    sleep(client.get_id()%3);
    srand (time(NULL));
    request.start_addr = rand() % 100;
    request.size = rand() % 20;
    request.is_read = rand() % 1;
    char buffer[] = "Hello World";
    //std::cout << "[CLIENT]" << "sending reuqest: start_addr=" << request.start_addr << " size=" << request.size << std::endl;
   /* for(int i=0; i < 100 ; i++){
        printf("sending %s\n",buffer);
        client.send_data(buffer, sizeof(buffer));
    }*/

    printf("sleeping for 10 sec\n");
    sleep(10);
    
    return 0;
}