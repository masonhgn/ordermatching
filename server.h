#include <iostream>
#include <cstring>
#include <unistd.h>
#include<arpa/inet.h>


class Server {
private:
    static const int PORT = 5000;
    static const int BUFF_SIZE = 1024;
    int server_fd;
    sockaddr_in server_addr;

public:

    
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
    }
};