// orderbook_kernel.cpp
#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>


struct Order_AXI {
    ap_uint<1> buy;      // 1 bit for buy/sell
    ap_uint<32> price;   // 32 bits for price
    ap_uint<32> quantity; // 32 bits for quantity
};


typedef ap_axiu<65, 0, 0, 0> AXI_ORDER;


#define PRICE_RANGE 1000000
#define MIN_PRICE 1
#define MAX_PRICE 1000000
#define QUEUE_CAPACITY 100


struct Order {
    ap_uint<32> price;
    ap_uint<32> quantity;
};


struct OrderQueue {
    Order orders[QUEUE_CAPACITY];
    ap_uint<16> head;
    ap_uint<16> tail;
};


void initialize_orderbook(OrderQueue bids[PRICE_RANGE], OrderQueue asks[PRICE_RANGE]) {
    for(int i = 0; i < PRICE_RANGE; i++) {
#pragma HLS PIPELINE
        for(int j = 0; j < QUEUE_CAPACITY; j++) {
#pragma HLS UNROLL
            bids[i].orders[j].price = 0;
            bids[i].orders[j].quantity = 0;
            asks[i].orders[j].price = 0;
            asks[i].orders[j].quantity = 0;
        }
        bids[i].head = 0;
        bids[i].tail = 0;
        asks[i].head = 0;
        asks[i].tail = 0;
    }
}


extern "C" {
    void process_order(
        AXI_ORDER &input_order,
        AXI_ORDER &output_order,
        OrderQueue bids[PRICE_RANGE],
        OrderQueue asks[PRICE_RANGE]
    ) {
#pragma HLS INTERFACE mode=axis port=input_order
#pragma HLS INTERFACE mode=axis port=output_order
#pragma HLS INTERFACE mode=bram port=bids
#pragma HLS INTERFACE mode=bram port=asks
#pragma HLS INTERFACE mode=control port=return


        bool buy = input_order.buy;
        uint32_t price = input_order.price;
        uint32_t quantity = input_order.quantity;

 
        output_order.buy = buy;
        output_order.price = price;
        output_order.quantity = quantity;


        int idx = price - MIN_PRICE;
        if(idx < 0 || idx >= PRICE_RANGE) return;

        if(buy) {

            OrderQueue &askQueue = asks[idx];
            if(askQueue.head != askQueue.tail) {
                Order &topAsk = askQueue.orders[askQueue.head];
                if(topAsk.price <= price && topAsk.quantity > 0) {
                    uint32_t tradedQty = (quantity < topAsk.quantity) ? quantity : topAsk.quantity;
                    quantity -= tradedQty;
                    topAsk.quantity -= tradedQty;

                
                    output_order.quantity = quantity;

                    
                    if(topAsk.quantity == 0) {
                        askQueue.head++;
                        if(askQueue.head >= QUEUE_CAPACITY) askQueue.head = 0;
                    }

                    
                    if(quantity == 0) return;
                }
            }

           
            OrderQueue &bidQueue = bids[idx];
            if(((bidQueue.tail + 1) % QUEUE_CAPACITY) != bidQueue.head) { 
                bidQueue.orders[bidQueue.tail].price = price;
                bidQueue.orders[bidQueue.tail].quantity = quantity;
                bidQueue.tail++;
                if(bidQueue.tail >= QUEUE_CAPACITY) bidQueue.tail = 0;
            }
        }
        else {

            OrderQueue &bidQueue = bids[idx];
            if(bidQueue.head != bidQueue.tail) {
                Order &topBid = bidQueue.orders[bidQueue.head];
                if(topBid.price >= price && topBid.quantity > 0) {
                    uint32_t tradedQty = (quantity < topBid.quantity) ? quantity : topBid.quantity;
                    quantity -= tradedQty;
                    topBid.quantity -= tradedQty;

             
                    output_order.quantity = quantity;

              
                    if(topBid.quantity == 0) {
                        bidQueue.head++;
                        if(bidQueue.head >= QUEUE_CAPACITY) bidQueue.head = 0;
                    }

              
                    if(quantity == 0) return;
                }
            }

         
            OrderQueue &askQueue = asks[idx];
            if(((askQueue.tail + 1) % QUEUE_CAPACITY) != askQueue.head) { 
                askQueue.orders[askQueue.tail].price = price;
                askQueue.orders[askQueue.tail].quantity = quantity;
                askQueue.tail++;
                if(askQueue.tail >= QUEUE_CAPACITY) askQueue.tail = 0;
            }
        }
    }
}
