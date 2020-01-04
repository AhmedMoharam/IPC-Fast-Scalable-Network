#include <vector>
#include <cstdint>
#include <thread>
#include <mutex>
#include <string>
#include "common.h"

class client {
public:
    client();
    ~client();
    void join_group(const char* unix_socket_path = DEFAULT_SOCKET_PATH);
    char get_id();
    void send_data(void * data,unsigned int size);
    virtual void process_data(void * data,unsigned int size){};
    
private:
    static void sigioHandler(int sig);
    void handle_requests(char data[MAX_PACKET_SIZE],unsigned int packet_size);

private:
    int fd;
    char client_id;
    std::vector<std::thread> threads;
    std::mutex read_mtx;
    static client * pInstance;
};

class server {
 public:
    template < class T=server>
    static T&  instance();
    virtual ~server();
    void create_group(const int & num_clients,const char* unix_socket_path = DEFAULT_SOCKET_PATH);
    void run_on_thread();
    void wait_on_thread();
    void send_data(char client_id, void * data, unsigned int size);
    virtual void process_data(char client_id, void * data,unsigned int size){};
    

 private:
    class client{
        public:
        client():
            fd(-1),id(-1),isConnected(false)
        {}
            int fd;
            char id;
            bool isConnected;
    };
protected:
    server();
private:
    server(const server &); //private copy constructor
    server & operator=(const server &); //private copy assigment operator
    void handle_requests();
    int sfd;
    static server * pInstance;
    std::string socket_path;
    std::thread server_thread;
    std::vector<server::client> clients;
};


template < class T>
inline T&  server::instance(){
    if(!pInstance)
        pInstance = new T;
    // new will assure that the Template is a derived class at compile time
    return *dynamic_cast<T*>(pInstance);
}

