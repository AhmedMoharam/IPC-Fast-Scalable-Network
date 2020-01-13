#include <vector>
#include <cstdint>
#include <thread>
#include <mutex>
#include <string>
#include <unordered_map>
#include <list>
#include "common.h"

class client {

struct request {
    unsigned int size;
    std::vector<char> data;
};
typedef std::list<request> request_queue;
public:
    client();
    ~client();
    void join_group(const char* unix_socket_path = DEFAULT_SOCKET_PATH);
    void close_connection();
    char get_id();
    void send_data(void * data,unsigned int size);
    virtual void process_data(void * data,unsigned int size){};
    
private:
    static void sigioHandler(int sig);
    void handle_requests();

private:
    int fd;
    char client_id;
    bool close_granted;
    request_queue requests;
    bool is_thread_active;
    std::thread execution_thread;
    static client * pInstance;
};

class server {
    typedef std::unordered_map<char,int> clientsMap;
 public:
    template < class T=server>
    static T&  instance();
    virtual ~server();
    void create_group(const int & num_clients,const char* unix_socket_path = DEFAULT_SOCKET_PATH);
    void run_on_thread();
    void wait_on_thread();
    void send_data(char client_id, void * data, unsigned int size);
    virtual void process_data(char client_id, void * data,unsigned int size){};
    
protected:
    server();
private:
    server(const server &); //private copy constructor
    server & operator=(const server &); //private copy assigment operator
    void handle_requests();
    int sfd;
    static server * pInstance;
    std::string socket_path;
    std::thread execution_thread;
    clientsMap clients;
};


template < class T>
inline T&  server::instance(){
    if(!pInstance)
        pInstance = new T;
    // new will assure that the Template is a derived class at compile time
    return *dynamic_cast<T*>(pInstance);
}

