// Auto-generated at 2026-11-26 15:29:08.195946 by ops-translator

#pragma once
#include <ops_hls_kernel_support.h>
#include "../../common/include/common_config.hpp"

extern "C" void repeater_outerloop_0(
    //u
        hls::stream <ap_axiu<axis_data_width, 0, 0, 0>>& arg0_axis_in,
    //u2
        hls::stream <ap_axiu<axis_data_width, 0, 0, 0>>& arg1_axis_out
)
{
    #pragma HLS INTERFACE axis port = arg0_axis_in register     #pragma HLS INTERFACE axis port = arg1_axis_out register 
    #pragma HLS INTERFACE ap_ctrl_none port=return
    
    #pragma HLS PIPELINE II=1 style=flp
    
    arg1_axis_out.write(arg0_axis_in.read());
}