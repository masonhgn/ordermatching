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

        int initialize() { //gets everything ready
            bids.clear();
            bids.resize(PRICE_RANGE);
            
            asks.clear();
            asks.resize(PRICE_RANGE);

            bestBidIndex = -1, bestAskIndex = -1;

            //open log file
            logFile.open(log_file_name, std::ios::out | std::ios::binary);
            if (!logFile) {
                std::cerr << "could not open log file " << log_file_name << "\n";
                return 1;
            }
            latencyLog.clear();
            latencyLog.reserve(BATCH_SIZE); 

            //get report stats ready
            totalLatencySum = 0;
            totalOrdersProcessed = 0;
            minLatency = std::numeric_limits<long long>::max();
            maxLatency = std::numeric_limits<long long>::lowest();


            return 0;
        }


        void flushLatencyData() {
            if (!logFile.is_open()) return;
            logFile.write(reinterpret_cast<const char*>(latencyLog.data()), latencyLog.size() * sizeof(long long));
            logFile.flush(); 
            latencyLog.clear();
        }



        void insert(const Order& order) { //adds order to orderbook
            int idx = priceToIndex(order.price);
            if (order.buy) { //add order, update index
                bids[idx].push_back(order);
                if (bestBidIndex == -1 || idx > bestBidIndex) bestBidIndex = idx;
            }
            else {
                asks[idx].push_back(order);
                if (bestAskIndex == -1 || idx < bestAskIndex) bestAskIndex = idx;
            }
        }

        void cleanup() { //cleans up levels
            //decrement index until you find non empty bids index
            while (bestBidIndex >= 0 && bids[bestBidIndex].empty()) bestBidIndex--;
            //increment until we find non empty asks index
            while (bestAskIndex >= 0 && bestAskIndex < PRICE_RANGE && asks[bestAskIndex].empty()) {
                bestAskIndex++;
                if (bestAskIndex >= PRICE_RANGE) {
                    bestAskIndex = -1; // no asks
                    break;
                }
            } 
        }


        void finalize_log() { //flushes remaining log, called at the end of the program lifecycle
            if (!latencyLog.empty()) flushLatencyData();
            if (logFile.is_open()) logFile.close();
        }



        void process(Order &order) {

            auto start = std::chrono::steady_clock::now();

            if (order.buy) { //buy order, try to match with sell orders 
                while (order.quantity > 0 && bestAskIndex != -1) {
                    int askPrice = MIN_PRICE + bestAskIndex; 
                    
                    // check if buy price >= ask price. if so, we can immediately match the order
                    if (order.price >= askPrice) {
                        auto &askQueue = asks[bestAskIndex]; //get corresponding index for best sell price
                        
                        // match with orders at this price index until order is filled or no asks left at this price
                        while (order.quantity > 0 && !askQueue.empty()) {
                            Order &topAsk = askQueue.front();
                            
                            int tradedQty = std::min(order.quantity, topAsk.quantity);
                            //TODO: LOG HERE
                            order.quantity -= tradedQty;
                            topAsk.quantity -= tradedQty;
                            
                            //remove top ask if there are no more sellers at this price
                            if (topAsk.quantity == 0) {
                                askQueue.pop_front();
                            }
                        }

                        //move bestBidIndex and bestAskIndex if needed
                        cleanup();
                        
                    } else break;
                }

                // add to bids if there is still any of the order left
                if (order.quantity > 0) insert(order);


            } else { //sell order, try to match with buy orders 
                
                while (order.quantity > 0 && bestBidIndex != -1) {
                    int bidPrice = MIN_PRICE + bestBidIndex;

                    // check if sell price <= bid price. if so, we can immediately match the order
                    if (order.price <= bidPrice) {
                        auto &bidQueue = bids[bestBidIndex];

                        while (order.quantity > 0 && !bidQueue.empty()) {
                            Order &topBid = bidQueue.front();

                            int tradedQty = std::min(order.quantity, topBid.quantity);

                            //TODO: LOG HERE
                            order.quantity -= tradedQty;
                            topBid.quantity -= tradedQty;

                            if (topBid.quantity == 0) bidQueue.pop_front();
                        }

                        //move bestBidIndex and bestAskIndex if needed
                        cleanup(); 

                    } else break;
                }

                // add to asks if there is still any of the order left
                if (order.quantity > 0) insert(order);
            }

            auto end = std::chrono::steady_clock::now();
            long long latency = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            latencyLog.push_back(latency);

            //for average latency
            totalLatencySum += latency;
            totalOrdersProcessed++;
            
            //update min and max latencies
            if (latency < minLatency) minLatency = latency;
            if (latency > maxLatency) maxLatency = latency;

            if (latencyLog.size() >= BATCH_SIZE) flushLatencyData(); //flush data if necessary
            
        }




        void writeReport(const std::string &report_filename) {

            //avoid division by zero
            double averageLatency = 0.0;
            if (totalOrdersProcessed > 0) {
                averageLatency = static_cast<double>(totalLatencySum) / static_cast<double>(totalOrdersProcessed);
            }

            std::ofstream reportFile(report_filename, std::ios::out);
            if (!reportFile) {
                std::cerr << "could not open report file " << report_filename << "\n";
                return;
            }

            reportFile << "OrderBook Processing Report\n";
            reportFile << "-----------------------\n";
            reportFile << "Total Orders Processed: " << totalOrdersProcessed << "\n";
            reportFile << "Average Latency (ns): " << averageLatency << "\n";
            if (totalOrdersProcessed > 0) {
                reportFile << "Min Latency (ns): " << minLatency << "\n";
                reportFile << "Max Latency (ns): " << maxLatency << "\n";
            }
            reportFile.close();
        }





        

    };



#endif