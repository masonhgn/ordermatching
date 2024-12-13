// orderbook_test.cpp

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <vector>
#include <cstdlib>


#include "experimental/xrt_bo.h"
#include "experimental/xrt_device.h"
#include "experimental/xrt_kernel.h"


#include "orderbook.h"
#include "utilities.h"


struct Order_AXI {
    ap_uint<1> buy;      // 1 bit for buy/sell
    ap_uint<32> price;   // 32 bits for price
    ap_uint<32> quantity; // 32 bits for quantity
};

int main(int argc, char *argv[]) {

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <orders_file>\n";
        std::cerr << "Example: " << argv[0] << " orders.bin\n";
        return EXIT_FAILURE;
    }

    std::string orders_file = argv[1];


    std::vector<Order> orders;
    if (!loadOrdersFromFile(orders_file, orders)) {
        std::cerr << "Error: Failed to load orders from file: " << orders_file << "\n";
        return EXIT_FAILURE;
    }

    std::cout << "Loaded " << orders.size() << " orders from " << orders_file << "\n";

 
    int device_index = 0;
    xrt::device device(device_index);
    std::string xclbinFilename = "orderbook_kernel.xclbin"; 


    std::cout << "Loading xclbin " << xclbinFilename << " on device " << device_index << "\n";
    auto uuid = device.load_xclbin(xclbinFilename);

    std::string kernel_name = "process_order"; 
    xrt::kernel kernel(device, uuid, kernel_name);




    constexpr int PRICE_RANGE_K = PRICE_RANGE;
    constexpr int QUEUE_CAPACITY_K = 100;


    size_t bids_size = PRICE_RANGE_K * QUEUE_CAPACITY_K * sizeof(int);
    size_t asks_size = PRICE_RANGE_K * QUEUE_CAPACITY_K * sizeof(int);


    auto bo_bids = xrt::bo(device, bids_size, kernel.group_id(3)); // Assuming group_id for bids
    auto bo_asks = xrt::bo(device, asks_size, kernel.group_id(4)); // Assuming group_id for asks


    auto bids_map = bo_bids.map<int*>();
    auto asks_map = bo_asks.map<int*>();


    std::fill(bids_map, bids_map + (PRICE_RANGE_K * QUEUE_CAPACITY_K), 0);
    std::fill(asks_map, asks_map + (PRICE_RANGE_K * QUEUE_CAPACITY_K), 0);


    bo_bids.sync(XCL_BO_SYNC_BO_TO_DEVICE, bids_size, 0);
    bo_asks.sync(XCL_BO_SYNC_BO_TO_DEVICE, asks_size, 0);




    auto start_time = std::chrono::high_resolution_clock::now();
    int orders_processed = 0;


    for(auto &order : orders) {

        Order_AXI input_order;
        input_order.buy = order.buy ? 1 : 0;
        input_order.price = order.price;
        input_order.quantity = order.quantity;


        Order_AXI output_order;


        auto bo_input = xrt::bo(device, sizeof(Order_AXI), kernel.group_id(0));
        auto bo_output = xrt::bo(device, sizeof(Order_AXI), kernel.group_id(1));


        auto input_map = bo_input.map<Order_AXI*>();
        auto output_map = bo_output.map<Order_AXI*>();


        *input_map = input_order;


        bo_input.sync(XCL_BO_SYNC_BO_TO_DEVICE, sizeof(Order_AXI), 0);


        auto run = kernel(bo_output, bo_input, bo_bids, bo_asks);
        run.wait(); 

  
        bo_output.sync(XCL_BO_SYNC_BO_FROM_DEVICE, sizeof(Order_AXI), 0);

 
        output_order = *output_map;


        order.quantity = output_order.quantity.to_int();

        orders_processed++;
    }

    auto end_time = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end_time - start_time;
    double total_seconds = duration.count();
    double average_latency = (orders_processed > 0) ? (total_seconds / orders_processed) : 0.0;

    std::cout << "Processed " << orders_processed << " orders in " << total_seconds << " seconds.\n";
    std::cout << "Average latency per order: " << average_latency * 1e9 << " nanoseconds.\n"; // Convert to ns

    // Finalize log (if applicable)
    // ob.finalize_log();

    // Generate report (if applicable)
    // ob.writeReport("report.rpt");

    return EXIT_SUCCESS;
}
