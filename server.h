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

    Server() : server_fd(-1), client_fd(-1) {}
    
    int initialize() {

        //create socket
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            std::cerr << "could not create socket\n";
            return 1;
        }

        //bind
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = INADDR_ANY;   

        if (bind(server_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "could not bind\n";
            close(server_fd);
            return 1;
        }

        //listen for connection
        if (listen(server_fd, 1) < 0) {
            std::cerr << "could not listen\n";
            close(server_fd);
            return 1;
        }

        std::cout << "listening on port " << PORT << "...\n";

        return 0;
    }

    int wait_for_client_connection() {
        //accept client connection
        socklen_t client_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            std::cerr << "could not accept\n";
            close(server_fd);
            return 1;
        }

        std::cout << "client connected...\n";
        return 0;
    }

    void listen_to_client() {
        while (true) {
            memset(buffer, 0, BUFF_SIZE); //clear buffer
            ssize_t bytes_read = recv(client_fd, buffer, BUFF_SIZE, 0); //get client input
            if (bytes_read <= 0) {
                std::cout << "client disconnected or error...\n";
                break;
            }

            std::string order_str(buffer);
            std::cout << "received order: " << order_str << "\n";
        }
    }

    void stop_server() {
        close(client_fd);
        close(server_fd);
    }


};



#endif // SERVER_H