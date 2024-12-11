#include <vector>
#include <fstream>
#include <random>
#include <algorithm>
#include <iostream> 
#include "orderbook.h"
#include "utilities.h"



int main(int argc, char** argv) {

    // usage: ./order_generator [num_orders] [file_name]
    // example: ./order_generator 5000000 orders.bin

    if (argc < 3) {
        std::cerr << "usage: " << argv[0] << " [num_orders] [file_name]\n";
        return 1;
    }

    size_t num_orders;
    try {
        num_orders = static_cast<size_t>(std::stoull(argv[1]));
    } catch (const std::exception &e) {
        std::cerr << "invalid number of orders: " << argv[1] << "\n";
        return 1;
    }

    std::string file_name = argv[2];


    auto orders = generateRandomOrders(num_orders);

    bool success = saveOrdersToFile(file_name, orders);
    if (!success) {
        std::cerr << "could not save orders to file: " << file_name << "\n";
        return 1;
    }

    std::cout << "generated & saved " << num_orders << " orders to " << file_name << "\n";

    return 0;
}



