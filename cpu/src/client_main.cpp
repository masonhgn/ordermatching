#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include "client.h"
#include "orderbook.h" 
#include "utilities.h"


//convert order to string for sending through buffer
inline std::string orderToString(const Order &o) {
    std::string side = o.buy ? "buy" : "sell";
    double price_dollars = o.price / 100.0;
    std::ostringstream oss;
    oss << side << " " << o.quantity << " " << price_dollars;
    return oss.str();
}

// Parse an order line from the REPL
// For simplicity, we just send the line directly without validation
inline bool parseOrderLine(const std::string &line, std::string &cmd) {
    cmd = line;
    return !cmd.empty();
}

int main(int argc, char *argv[]) {
    // Usage:
    // ./client_main <server_ip> [file_name]
    // If file_name is provided, load orders from file and send them
    // If file_name is not provided, run REPL mode

    if (argc < 2) {
        std::cerr << "usage:\n"
                  << argv[0] << " <server_ip> [file_name]\n"
                  << "If file_name is provided, orders are loaded from it.\n"
                  << "If no file_name is provided, orders are read interactively.\n";
        return 1;
    }

    std::string server_ip = argv[1];
    std::string file_name;

    Client c(server_ip, 5000);
    if (c.connect_to_server() != 0) {
        std::cerr << "Could not connect to server at " << server_ip << ":5000\n";
        return 1;
    }

    if (argc == 2) {
        std::cout << "connected to " << server_ip << ":5000\n"
                  << "format: buy <quantity> <price>\n"
                  << "example: buy 100 4.56\n"
                  << "press ctrl+D (EOF) or enter an empty line to quit.\n";

        std::string line;
        while (true) {
            std::cout << "> ";
            if (!std::getline(std::cin, line)) break;
            if (line.empty()) break;

            std::string cmd;
            if (!parseOrderLine(line, cmd)) {
                std::cerr << "invalid order format, try again...\n";
                continue;
            }

            if (c.send_order(cmd + "\n") != 0) {
                std::cerr << "failed to send order: " << cmd << "\n";
            }
        }

    } else if (argc == 3) {
        file_name = argv[2];
        std::vector<Order> orders;
        if (!loadOrdersFromFile(file_name, orders)) {
            std::cerr << "failed to load orders from " << file_name << "\n";
            c.close_client();
            return 1;
        }

        for (const auto &o : orders) {
            std::string cmd = orderToString(o);
            if (c.send_order(cmd + "\n") != 0) std::cerr << "failed to send order: " << cmd << "\n";
        }
        std::cout << "finished sending " << orders.size() << " orders from file " << file_name << "\n";
    }

    c.close_client();
    return 0;
}