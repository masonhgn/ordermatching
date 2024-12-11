#include <iostream>
#include <cstring>

#ifndef SERVER_H
#define SERVER_H

#include <unistd.h>
#include <arpa/inet.h>


class Server {
private:
    static const int PORT = 5000;
    static const int BUFF_SIZE = 1024;
    char buffer[BUFF_SIZE];
    int server_fd;
    sockaddr_in server_addr;
    sockaddr_in client_addr;
public:

    Server();
    
    int initialize();

    int wait_for_client_connection();

    void listen_to_client();

    void stop_server();


};



#endif // SERVER_H