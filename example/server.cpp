#include "../ipc.h"
#include <string>
#include <iostream>
#include <unistd.h>

struct io_request {
    uint64_t start_addr;
    int size;
    bool is_read;
};


class myserver: public server{
public:
    myserver():a(1),b(1){}
    virtual void process_data(char client_id, void * data,unsigned int size){  
       std::cout << "server received: ";
       std::string s((char *)data,size);
       printf("%s",s.c_str());
       std::cout << " count=";
       client_id == 'A' ? std::cout << a++ : std::cout << b++;
       std::cout << std::endl;
       /* std::cout<< "[SERVER] processing CLIENT[" << client_id << "] request:" ;
        if(size == sizeof(io_request)){
            io_request * req = reinterpret_cast<io_request *>(data);
            std::cout << "start_addr:" << req->start_addr << " size:" << req->size << std::endl;
            send_data(client_id,data,size);
        }
        */
        
    };
private:
    int a;
    int b;

};



int main(int argc, char * argv[]){
    myserver& server_inst = server::instance<myserver>();
    int num_clients = std::stoi(argv[1]);
    server_inst.create_group(num_clients);
    server_inst.run_on_thread();
     char buffer[] = "Hello World";
    //std::cout << "[CLIENT]" << "sending reuqest: start_addr=" << request.start_addr << " size=" << request.size << std::endl;
    for(int i=0; i < 3 ; i++){
        server_inst.send_data('A',buffer, sizeof(buffer));
        sleep(1);
    }
   /* for(int i=0; i < 32 ; i++){
        server_inst.send_data('B',buffer, sizeof(buffer));
    }*/
    
   // sleep(100);
    server_inst.wait_on_thread();
    return 0;
}