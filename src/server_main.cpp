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
#include <atomic>
#include <csignal>
#include "server.h" 
#include "orderbook.h"

static std::queue<Order> orderQueue; //order buffer between the producer (server) and consumer (orderbook)
static std::mutex orderQueueMutex; //mutex for variable above
static std::condition_variable orderAvailableCV; //condition variable signaling if an order is in the queue
static std::atomic<bool> stopRequested(false); //for wrapping things up


//handle Ctrl+C gracefully
void signalHandler(int signum) {
    if (signum == SIGINT) {
        stopRequested.store(true);
        orderAvailableCV.notify_all();
    }
}


void orderBookConsumer(OrderBook &ob) {
    while (true) {
        Order o;
        {
            std::unique_lock<std::mutex> lock(orderQueueMutex);
            orderAvailableCV.wait(lock, []{ return !orderQueue.empty() || stopRequested.load(); });

            if (stopRequested.load() && orderQueue.empty()) { //doesn't stop processing orders until orderQueue is empty
                break; // no more orders and stop requested
            }

            if (!orderQueue.empty()) {
                o = orderQueue.front();
                orderQueue.pop();
            } else continue;
        }
        ob.process(o);
    }
    std::cout << "order feed thread exited\n";
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

    //handle signal
    std::signal(SIGINT, signalHandler);

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

    std::cout << "waiting for client connection...\n";
    if (s.wait_for_client_connection() != 0) {
        std::cerr << "failed to accept client connection\n";
        s.stop_server();
        return 1;
    }


    s.setSharedResources(&orderQueue, &orderQueueMutex, &orderAvailableCV); //set the bridge between server & orderbook

    std::thread consumerThread(orderBookConsumer, std::ref(ob));
    std::cout << "order feed thread started\n";

    std::thread serverThread([&s]() {
        std::cout << "server now listening to client...\n";
        s.listen_to_client();
        std::cout << "server stopped listening to client\n";
    });


    while (!stopRequested.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "\nreceived shutdown signal (Ctrl+C)\n";

    s.stop_server();
    std::cout << "\nserver stopped\n";
    {
        std::lock_guard<std::mutex> lock(orderQueueMutex);
    }

    orderAvailableCV.notify_all(); //wake up order feed thread

    if (serverThread.joinable()) {
        serverThread.join();
        std::cout << "\nserver thread joined\n";
    }
    if (consumerThread.joinable()) {
        consumerThread.join();
        std::cout << "\norder feed thread joined\n";
    }
    if (ob.getTotalOrdersProcessed() > 0) {
        ob.finalize_log();
        ob.writeReport("report_"+session_id+".rpt");
        std::cout << "report generated: report_" + session_id + ".rpt\n";
    }
    return 0;
}
