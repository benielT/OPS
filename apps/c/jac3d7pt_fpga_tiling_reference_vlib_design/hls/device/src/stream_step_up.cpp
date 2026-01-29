
#include <ops_hls_kernel_support.h>
#include "common_config.hpp"

extern "C" void stream_step_up(
    const unsigned int total_xblocks,
    const unsigned int num_pkts,
    const unsigned int outer_itr,
     ::hls::stream <ap_axiu<axis_data_width,0,0,0>>& strm_in,
     ::hls::stream <ap_axiu<mem_data_width,0,0,0>>& strm_out
) {
    #pragma HLS INTERFACE mode=s_axilite port=total_xblocks bundle = control
    #pragma HLS INTERFACE mode=s_axilite port=num_pkts bundle = control
    #pragma HLS INTERFACE mode=s_axilite port=outer_itr bundle = control
    #pragma HLS INTERFACE mode=axis port=strm_out register
    #pragma HLS INTERFACE mode=axis port=strm_in register

    #pragma HLS INTERFACE mode=s_axilite port=return bundle = control
    #pragma HLS INTERFACE mode=ap_ctrl_chain port=return

    ::hls::stream<ap_uint<axis_data_width>> write_reduced_mem_strm;
    #pragma HLS STREAM variable = write_reduced_mem_strm depth = 64
    ::hls::stream<ap_uint<mem_data_width>> write_mem_strm;
    #pragma HLS STREAM variable = write_mem_strm depth = 32

#ifdef DEBUG_LOG
    printf("[KERNEL_DEBUG]|%s| starting", __func__);
#endif 
    for (unsigned int itr = 0; itr < outer_itr; itr++) {
        #pragma HLS DATAFLOW
        ops::hls::axis2stream<axis_data_width>(strm_in, write_reduced_mem_strm, num_pkts);
        ops::hls::stream2streamStepup<axis_data_width, mem_data_width>(write_reduced_mem_strm, write_mem_strm, total_xblocks);
        ops::hls::stream2axis<mem_data_width>(write_mem_strm, strm_out, total_xblocks);
    }
}