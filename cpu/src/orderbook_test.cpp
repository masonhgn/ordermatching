// orderbook_test.cpp

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <vector>
#include <cstdlib>

// Include the OrderBook class and utilities
#include "orderbook.h"
#include "utilities.h"




int main(int argc, char *argv[]) {
    // Check for correct usage
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <orders_file>\n";
        std::cerr << "Example: " << argv[0] << " orders.bin\n";
        return EXIT_FAILURE;
    }

    std::string orders_file = argv[1];

    //load orders from binary file
    std::vector<Order> orders;
    if (!loadOrdersFromFile(orders_file, orders)) {
        std::cerr << "Error: Failed to load orders from file: " << orders_file << "\n";
        return EXIT_FAILURE;
    }

    std::cout << "Loaded " << orders.size() << " orders from " << orders_file << "\n";


    auto now = std::chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();

    std::string session_id = std::to_string(millis);
    std::string log_file = "latencies_" + session_id + ".bin";

    OrderBook ob(log_file);
    if (ob.initialize() != 0) {
        std::cerr << "Failed to initialize OrderBook.\n";
        return EXIT_FAILURE;
    }

    std::cout << "OrderBook initialized with log file: " << log_file << "\n";

    //to record performance stats
    auto start_time = std::chrono::high_resolution_clock::now();
    int orders_processed = 0;


    for (auto &order : orders) {
        ob.process(order);
        orders_processed++;
    }

    auto end_time = std::chrono::high_resolution_clock::now();


    std::chrono::duration<double> duration = end_time - start_time;
    double total_seconds = duration.count();
    double average_latency = (orders_processed > 0) ? (total_seconds / orders_processed) : 0.0;

    std::cout << "Processed " << orders_processed << " orders in " << total_seconds << " seconds.\n";
    std::cout << "Average latency per order: " << average_latency * 1e6 << " microseconds.\n";

    ob.finalize_log();


    std::string report_file = "report_" + session_id + ".rpt";
    ob.writeReport(report_file);
    std::cout << "Report generated: " << report_file << "\n";

    return EXIT_SUCCESS;
}
