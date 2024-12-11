#include "orderbook.h"



void orderbook::OrderBook::initialize() {
    return;
}


void orderbook::OrderBook::insert(const Order& order) {
    int idx = priceToIndex(order.price);
    if (order.buy) { //add order, update index
        bids[idx].push_back(order);
        if (bestBidIndex == -1 || idx > bestBidIndex) bestBidIndex = idx;
    }
    else {
        asks[idx].push_back(order);
        if (bestAskIndex == -1 || idx > bestAskIndex) bestAskIndex = idx;
    }
}


void orderbook::OrderBook::cleanup() { 
    return; 
}

void orderbook::OrderBook::processOrder(Order& order) {
    return;
}




