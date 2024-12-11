#include <vector>
#include <deque>


#ifndef ORDERBOOK_H
#define ORDERBOOK_H
namespace orderbook {

    struct Order {
        bool buy;           // true for buy, false for sell
        int price;          // integer price
        int quantity;       // quantity remaining
        int client_id;      //id of client placing order (so if we match, we know who to tell)
    };


    class OrderBook {

    private:
        static const int SCALE_FACTOR = 100;  // 1 = 1 cent
        static const int MIN_PRICE = 1; //1 means min price of $0.01 per security
        static const int MAX_PRICE = 100000000; //100M means a max price of $1M per security
        static const int PRICE_RANGE = MAX_PRICE - MIN_PRICE + 1;

        int bestBidIndex = -1; 
        int bestAskIndex = -1; 
    

        std::vector<std::deque<Order>> bids; // array of queues for buy orders
        std::vector<std::deque<Order>> asks; // array of queues for sell orders


    public:
        // convert price in regular form (4.56) to cents (456)
        inline int priceToCents(double priceDollars) {
            return static_cast<int>(priceDollars * SCALE_FACTOR + 0.5);
        }

        //convert price in dollars to index in orderbook
        inline int priceToIndex(double priceDollars) {
            return priceToCents(priceDollars) - MIN_PRICE;
        }

        void initialize(); //gets everything ready

        void insert(const Order& order); //adds order to orderbook

        void cleanup(); //cleans up levels

        void processOrder(Order& order); //processes a single order

    };

}

#endif