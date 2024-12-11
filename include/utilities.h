#include <vector>
#include <fstream>
#include <random>
#include <algorithm>
#include <iostream> 
#include "orderbook.h"





inline std::vector<orderbook::Order> generateRandomOrders(
    size_t count, 
    int minPrice = 100, 
    int maxPrice = 100000, 
    int minQty = 1, 
    int maxQty = 1000, 
    int clientCount = 10000
) {
    std::vector<Order> orders;
    orders.reserve(count);

    //randomness
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> priceDist(minPrice, maxPrice);
    std::uniform_int_distribution<int> qtyDist(minQty, maxQty);
    // std::uniform_int_distribution<int> clientDist(0, clientCount - 1);
    std::uniform_int_distribution<int> sideDist(0, 1); // 0 for sell, 1 for buy

    for (size_t i = 0; i < count; ++i) {
        Order o;
        o.buy = (sideDist(gen) == 1);
        o.price = priceDist(gen);
        o.quantity = qtyDist(gen);
        //o.client_id = clientDist(gen);
        orders.push_back(o);
    }

    return orders;
}

//write orders to binary file
inline bool saveOrdersToFile(const std::string &filename, const std::vector<Order> &orders) {
    std::ofstream outFile(filename, std::ios::binary | std::ios::out);
    if (!outFile) {
        std::cerr << "could not open file to write: " << filename << "\n";
        return false;
    }

    //# of orders
    size_t count = orders.size();
    outFile.write(reinterpret_cast<const char*>(&count), sizeof(count));

    //all orders
    outFile.write(reinterpret_cast<const char*>(orders.data()), count * sizeof(Order));

    outFile.close();
    return true;
}

//load orders from binary file
inline bool loadOrdersFromFile(const std::string &filename, std::vector<Order> &orders) {
    std::ifstream inFile(filename, std::ios::binary | std::ios::in);
    if (!inFile) {
        std::cerr << "could not open file to read: " << filename << "\n";
        return false;
    }

    size_t count;
    inFile.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (!inFile) {
        std::cerr << "could not read count from file\n";
        return false;
    }

    orders.resize(count);
    inFile.read(reinterpret_cast<char*>(orders.data()), count * sizeof(Order));
    if (!inFile) {
        std::cerr << "could not read orders from file\n";
        return false;
    }

    inFile.close();
    return true;
}
