#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

class Client {
private:
    static const int BUFF_SIZE = 1024;
    char buffer[BUFF_SIZE];
    int client_fd;
    sockaddr_in server_addr;
    std::string server_ip;
    int server_port;

public:
    Client(const std::string &ip, int port);

    int connect_to_server();

    int send_order(const std::string &order_str);

    void close_client();
};

#endif