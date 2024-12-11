#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include <random>
#include <chrono>
#include "server.h" 
#include "orderbook.h"


inline std::string generateRandomSessionId() {

    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long long> dist(0, 9999999999LL);

    long long random_id = dist(gen);

    return std::to_string(now) + "_" + std::to_string(random_id);
    
}


//parses a single order line
bool parseOrderLine(const std::string &line, orderbook::Order &o) {
    std::istringstream iss(line);
    std::string side;
    int quantity;
    double price;

    if (!(iss >> side >> quantity >> price)) return false; // parsing failed

    if (side != "buy" && side != "sell") return false; //invalid side

    
    //create order
    o.buy = (side == "buy");
    int intPrice = static_cast<int>(price * 100 + 0.5);
    o.price = intPrice;
    o.quantity = quantity;
    return true;
}

int main() {
    std::string session_id = generateRandomSessionId();
    std::string log_file = "latencies_" + session_id + ".bin";
    orderbook::OrderBook ob(log_file);
    if (ob.initialize() != 0) {
        std::cerr << "failed to initialize orderbook\n";
        return 1;
    }

    Server s;
    if (s.initialize() != 0) {
        std::cerr << "failed to initialize server\n";
        return 1;
    }

    if (s.wait_for_client_connection() != 0) {
        std::cerr << "failed to accept client connection\n";
        s.stop_server();
        return 1;
    }

    auto handleLine = [&](const std::string &line) {
        orderbook::Order o;
        if (!parseOrderLine(line, o)) {
            std::cerr << "invalid order format: " << line << "\n";

            return;
        }
        ob.process(o);

    };

    s.listen_to_client(handleLine);


    s.stop_server();
    ob.finalize_log();
    ob.writeReport("report_"+session_id+".rpt");

    return 0;
}
