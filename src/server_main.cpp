#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include <random>
#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "server.h" 
#include "orderbook.h"

static std::queue<Order> orderQueue; //order buffer between the producer (server) and consumer (orderbook)
static std::mutex orderQueueMutex; //mutex for variable above
static std::condition_variable orderAvailableCV; //condition variable signaling if an order is in the queue
static bool stopRequested = false; //for wrapping things up


void orderBookConsumer(OrderBook &ob) {
    while (true) {
        Order o;
        {
            std::unique_lock<std::mutex> lock(orderQueueMutex);
            orderAvailableCV.wait(lock, []{ return !orderQueue.empty() || stopRequested; });

            if (stopRequested && orderQueue.empty()) { //doesn't stop processing orders until orderQueue is empty
                break; // no more orders and stop requested
            }

            o = orderQueue.front();
            orderQueue.pop();
        }
        ob.process(o);
    }
}



inline std::string generateRandomSessionId() {

    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long long> dist(0, 9999999999LL);

    long long random_id = dist(gen);

    return std::to_string(now) + "_" + std::to_string(random_id);
    
}

int main() {
    std::string session_id = generateRandomSessionId();
    std::string log_file = "latencies_" + session_id + ".bin";
    OrderBook ob(log_file);
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

    std::thread consumerThread(orderBookConsumer, std::ref(ob));

    s.setSharedResources(&orderQueue, &orderQueueMutex, &orderAvailableCV); //set the bridge between server & orderbook
    s.listen_to_client();
    


    s.stop_server();

    {
        std::lock_guard<std::mutex> lock(orderQueueMutex);
        stopRequested = true;
    }
    orderAvailableCV.notify_all();
    consumerThread.join();

    ob.finalize_log();
    ob.writeReport("report_"+session_id+".rpt");

    return 0;
}
