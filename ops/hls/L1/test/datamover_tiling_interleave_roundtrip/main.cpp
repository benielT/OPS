#include <iostream>
#include <cstdlib>
#include <hls_stream.h>
#include <ap_int.h>
#include "top.hpp"


int test_combined() {
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Running Test 3: Combined Datapath       " << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    hls::stream<ap_uint<MEM_DATA_WIDTH>> in_stream[NUM_STREAMS];
    hls::stream<ap_uint<MEM_DATA_WIDTH>> out_stream[NUM_STREAMS];

    ap_uint<MEM_DATA_WIDTH> gold_ref[NUM_PKTS][NUM_STREAMS];

    // 1. Generate random input data and push to in_stream
    for (unsigned int i = 0; i < NUM_PKTS; i++) {
        for (unsigned short n = 0; n < NUM_STREAMS; n++) {
            ap_uint<MEM_DATA_WIDTH> val = rand();
            in_stream[n].write(val);
            gold_ref[i][n] = val; // Store for final comparison
        }
    }

    // // 2. Process through stream2interleave
    // ops::hls::stream2interleave<MEM_DATA_WIDTH, DATA_WIDTH, NUM_STREAMS, OVERLAP_SIZE>(in_stream, mid_stream, NUM_PKTS);

    // // 3. Process through interleave2stream
    // ops::hls::interleave2stream<MEM_DATA_WIDTH, DATA_WIDTH, NUM_STREAMS, OVERLAP_SIZE>(mid_stream, out_stream, NUM_PKTS);
    dut(in_stream, out_stream);
    // 4. Check results against golden reference
    int err_cnt = 0;
    for (unsigned int i = 0; i < NUM_PKTS; i++) {
        for (unsigned short n = 0; n < NUM_STREAMS; n++) {
            if (out_stream[n].empty()) {
                std::cout << "Error: Missing data at Pkt " << i << " Stream " << n << std::endl;
                err_cnt++;
                continue;
            }
            
            ap_uint<MEM_DATA_WIDTH> actual = out_stream[n].read();
            ap_uint<MEM_DATA_WIDTH> expected = gold_ref[i][n];

            if (actual != expected) {
                std::cout << "Mismatch at Pkt " << i << " Stream " << n 
                          << " | Expected: 0x" << std::hex << expected 
                          << " | Got: 0x" << actual << std::dec << std::endl;
                err_cnt++;
            }
        }
    }

    if (err_cnt == 0) {
        std::cout << "Test 3 Passed! (Combined Datapath matches Golden Reference)\n\n";
    }
    return err_cnt;
}

// --------------------------------------------------------
// Main Testbench Execution
// --------------------------------------------------------
int main() {
    int total_errors = 0;

    total_errors = test_combined();

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