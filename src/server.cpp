
#include "server.h"


Server::Server() : server_fd(-1), client_fd(-1), orderQueue(nullptr), orderMutex(nullptr), orderCV(nullptr) {}


void Server::setSharedResources(std::queue<Order>* q, std::mutex* m, std::condition_variable* cv) {
    orderQueue = q;
    orderMutex = m;
    orderCV = cv;
}

bool Server::parseOrderLine(const std::string &line, Order &o) {
    std::istringstream iss(line);
    std::string side;
    int quantity;
    double price;

    if (!(iss >> side >> quantity >> price)) return false; // parsing failed
    if (side != "buy" && side != "sell") return false; // invalid side

    // Create order
    o.buy = (side == "buy");
    int intPrice = static_cast<int>(price * 100 + 0.5);
    o.price = intPrice;
    o.quantity = quantity;
    return true;
}


int Server::initialize() {

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
    //server_addr.sin_addr.s_addr = INADDR_ANY; 
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
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

int Server::wait_for_client_connection() {
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





void Server::listen_to_client() {
    std::string leftover; // for incomplete orders
    char buffer[BUFF_SIZE];


    while (true) {
        memset(buffer, 0, BUFF_SIZE); //clear buffer
        ssize_t bytes_read = recv(client_fd, buffer, BUFF_SIZE, 0); //get client input


        if (bytes_read < 0) {
            std::cerr << "error: recv() failed\n";
            break;
        } else if (bytes_read == 0) {
            std::cout << "client disconnected\n";
            break;
        }

        std::string data(buffer, bytes_read);
        leftover += data;

        size_t pos;
        while ((pos = leftover.find('\n')) != std::string::npos) {
            std::string order_str = leftover.substr(0, pos);
            leftover.erase(0, pos + 1);

            std::cout << "received order: " << order_str << "\n";

            Order o;
            if (parseOrderLine(order_str, o)) {
                {
                    std::lock_guard<std::mutex> lock(*orderMutex);
                    orderQueue->push(o);
                }
                orderCV->notify_one();
            } else {
                std::cerr << "invalid order format: " << order_str << "\n";
            }
        }

    }
    std::cout << "exiting listen_to_client.\n";
}

void Server::stop_server() {
    if (client_fd >= 0) {
        shutdown(client_fd, SHUT_RDWR);
        close(client_fd);
        std::cout << "client socket closed\n";
        client_fd = -1;
    }
    if (server_fd >= 0) {
        shutdown(server_fd, SHUT_RDWR);
        close(server_fd);
        std::cout << "server socket closed\n";
        server_fd = -1;
    }
}
