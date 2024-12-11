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
    Client(const std::string &ip, int port) : server_ip(ip), server_port(port), client_fd(-1) {}

    int connect_to_server() {
        client_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (client_fd < 0) {
            std::cerr << "could not create socket\n";
            return 1;
        }

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port);

        if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
            std::cerr << "invalid address / address not supported\n";
            close(client_fd);
            return 1;
        }

        if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "connection failed\n";
            close(client_fd);
            return 1;
        }

        std::cout << "connected to server " << server_ip << ":" << server_port << "\n";
        return 0;
    }

    int send_order(const std::string &order_str) {
        ssize_t bytes_sent = send(client_fd, order_str.c_str(), order_str.size(), 0);
        if (bytes_sent < 0) {
            std::cerr << "could not send\n";
            return 1;
        }
        return 0;
    }

    void close_client() {
        close(client_fd);
    }
};

#endif