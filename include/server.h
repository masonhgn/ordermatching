#include <iostream>
#include <cstring>
#include <queue>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "orderbook.h"

#ifndef SERVER_H
#define SERVER_H



class Server {
private:
    static const int PORT = 5000;
    static const int BUFF_SIZE = 1024;
    char buffer[BUFF_SIZE];
    int server_fd, client_fd;
    sockaddr_in server_addr;
    sockaddr_in client_addr;

    std::queue<Order>* orderQueue;
    std::mutex* orderMutex;
    std::condition_variable* orderCV;

public:

    Server();
    
    int initialize();

    void setSharedResources(std::queue<Order>* q,std::mutex* m,std::condition_variable* cv);

    bool parseOrderLine(const std::string &line, Order &o);

    int wait_for_client_connection();

    void listen_to_client();


    void stop_server();


};



#endif // SERVER_H