#include <iostream>
#include <cstdlib>
#include <hls_stream.h>
#include <ap_int.h>
#include "top.hpp"


// --------------------------------------------------------
// Test 1: Individual test for stream2interleave
// --------------------------------------------------------
int test_stream2interleave() {
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Running Test 1: stream2interleave       " << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    hls::stream<ap_uint<MEM_DATA_WIDTH>> in_stream[NUM_STREAMS];
    hls::stream<ap_uint<MEM_DATA_WIDTH_OUT>> out_stream[NUM_STREAMS];

    // Feed known sequential data
    for (unsigned int i = 0; i < NUM_PKTS; i++) {
        for (unsigned short n = 0; n < NUM_STREAMS; n++) {
            ap_uint<MEM_DATA_WIDTH> val = (i << 16) | n; 
            in_stream[n].write(val);
        }
    }

    // Run DUT
    dut(in_stream, out_stream);

    // Read and display output
    int err_cnt = 0;
    for (unsigned int i = 0; i < NUM_PKTS; i++) {
        for (unsigned short n = 0; n < NUM_STREAMS; n++) {
            if (!out_stream[n].empty()) {
                ap_uint<MEM_DATA_WIDTH_OUT> val = out_stream[n].read();
                // Just printing a few to visually verify overlap boundaries
                if (i < 2) {
                    std::cout << "Pkt " << i << " Stream " << n << " interleaved data: 0x" << std::hex << val << std::dec << std::endl;
                }
            } else {
                std::cout << "Error: stream2interleave output stream " << n << " empty prematurely." << std::endl;
                err_cnt++;
            }
        }
    }
    
    if (err_cnt == 0) std::cout << "Test 1 Passed (Visual Verification)!\n\n";
    return err_cnt;
}


// --------------------------------------------------------
// Main Testbench Execution
// --------------------------------------------------------
int main() {
    int total_errors = 0;

    total_errors = test_stream2interleave();

    if (total_errors == 0) {
        std::cout << "========================================" << std::endl;
        std::cout << " ALL TESTS PASSED SUCCESSFULLY!         " << std::endl;
        std::cout << "========================================" << std::endl;
        return 0; // CSim Pass
    } else {
        std::cout << "========================================" << std::endl;
        std::cout << " SIMULATION FAILED WITH " << total_errors << " ERRORS." << std::endl;
        std::cout << "========================================" << std::endl;
        return 1; // CSim Fail
    }
}