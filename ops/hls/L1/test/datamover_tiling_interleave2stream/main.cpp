#include <iostream>
#include <cstdlib>
#include <hls_stream.h>
#include <ap_int.h>
#include "top.hpp"


// --------------------------------------------------------
// Include the design under test (DUT)
// --------------------------------------------------------
// Assuming your provided code is saved in "interleave_modules.hpp"
// #include "interleave_modules.hpp" 

// (For the sake of this self-contained file, the template functions 
// stream2interleave and interleave2stream should be available here).

// --------------------------------------------------------
// Test 2: Individual test for interleave2stream
// --------------------------------------------------------
int test_interleave2stream() {
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Running Test 2: interleave2stream       " << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    hls::stream<ap_uint<MEM_DATA_WIDTH_IN>> in_stream[NUM_STREAMS];
    #pragma HLS STREAM variable = in_stream depth=16
    hls::stream<ap_uint<MEM_DATA_WIDTH>> out_stream[NUM_STREAMS];
    #pragma HLS STREAM variable = out_stream depth=16
    // Feed dummy interleaved data
    for (unsigned int i = 0; i < NUM_PKTS; i++) {
        for (unsigned short n = 0; n < NUM_STREAMS; n++) {
            ap_uint<MEM_DATA_WIDTH_IN> val = 0;
            // Place original payload in the expected ranges based on stream index
            if (n % 2 == 0) {
                val.range(MEM_DATA_WIDTH_IN - 1, REALISED_OVERLAP) = (i * 10) + n; // Even streams
            } else {
                val.range(MEM_DATA_WIDTH - 1, 0) = (i * 10) + n; // Odd streams
            }
            in_stream[n].write(val);
        }
    }

    // Run DUT
    dut(in_stream, out_stream);

    // Verify Output
    int err_cnt = 0;
    for (unsigned int i = 0; i < NUM_PKTS; i++) {
        for (unsigned short n = 0; n < NUM_STREAMS; n++) {
            ap_uint<MEM_DATA_WIDTH> expected_val = (i * 10) + n;
            ap_uint<MEM_DATA_WIDTH> actual_val = out_stream[n].read();
            
            if (actual_val != expected_val) {
                std::cout << "Mismatch at Pkt " << i << " Stream " << n 
                          << ": Expected " << expected_val 
                          << " Got " << actual_val << std::endl;
                err_cnt++;
            }
        }
    }

    if (err_cnt == 0) std::cout << "Test 2 Passed!\n\n";
    return err_cnt;
}

// --------------------------------------------------------
// Main Testbench Execution
// --------------------------------------------------------
int main() {
    int total_errors = 0;

    total_errors = test_interleave2stream();

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