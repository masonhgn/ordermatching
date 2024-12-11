#include <vector>
#include <deque>
#include <chrono>
#include <fstream>
#include <algorithm> // for std::min
#include <iostream> 


#ifndef ORDERBOOK_H
#define ORDERBOOK_H




    struct Order {
        bool buy;           // true for buy, false for sell
        int price;          // integer price
        int quantity;       // quantity remaining
        // int client_id;      //id of client placing order (so if we match, we know who to tell)
    };


    class OrderBook {

    private:
        static const int SCALE_FACTOR = 100;  // 1 = 1 cent
        static const int MIN_PRICE = 1; //1 means min price of $0.01 per security
        static const int MAX_PRICE = 1000000; //1M means a max price of $10k per security
        static const int PRICE_RANGE = MAX_PRICE - MIN_PRICE + 1;

        int bestBidIndex = -1; 
        int bestAskIndex = -1; 
    

        std::vector<std::deque<Order>> bids; // array of queues for buy orders
        std::vector<std::deque<Order>> asks; // array of queues for sell orders

        std::vector<long long> latencyLog; 
        static const size_t BATCH_SIZE = 10000;
        std::ofstream logFile;
        std::string log_file_name;


        //these are for generating a report
        long long totalLatencySum = 0;
        long long totalOrdersProcessed = 0;
        long long minLatency = std::numeric_limits<long long>::max();
        long long maxLatency = std::numeric_limits<long long>::lowest();



    public:
        //default constructor
        OrderBook(std::string& log_file) : log_file_name(log_file) {}

        // convert price in regular form (4.56) to cents (456)
        inline int priceToCents(double priceDollars) {
            return static_cast<int>(priceDollars * SCALE_FACTOR + 0.5);
        }

        //convert price in dollars to index in orderbook
        inline int priceToIndex(double priceDollars) {
            return priceToCents(priceDollars) - MIN_PRICE;
        }

        int initialize(); //gets everything ready

        void flushLatencyData();

        void insert(const Order& order); //adds order to orderbook

        void cleanup(); //cleans up levels

        void finalize_log();

        void process(Order &order);

        void writeReport(const std::string &report_filename);

    };



#endif