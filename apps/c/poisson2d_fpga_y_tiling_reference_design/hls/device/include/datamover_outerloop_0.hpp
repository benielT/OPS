// Auto-generated at 2026-11-26 15:29:08.153240 by ops-translator


#pragma once
#include <ops_hls_kernel_support.h>
#include "../../common/include/common_config.hpp"


extern "C" void datamover_outerloop_0(
        const unsigned short range_start_0,
        const unsigned short range_end_0,
        const unsigned short range_start_1,
        const unsigned short range_end_1,
        const unsigned short gridSize_0,
        const unsigned short gridSize_1,
        const unsigned int outer_itr,
        const unsigned short tile_size_x,
        const unsigned short overlap_size_x,
        const unsigned short effective_tile_size_x,
        const unsigned short last_tile_size_x,
        const unsigned short tile_count_x,
        const unsigned int total_xblocks,
    //u
        ap_uint<mem_data_width>* arg0_b0,
        ap_uint<mem_data_width>* arg0_b1,
        ap_uint<mem_data_width>* arg0_b2,
        ap_uint<mem_data_width>* arg0_b3,
        ap_uint<mem_data_width>* arg0_b4,
        ap_uint<mem_data_width>* arg0_b5,
        ap_uint<mem_data_width>* arg0_b6,
        ap_uint<mem_data_width>* arg0_b7,
    //u2
        ap_uint<mem_data_width>* arg1_b0,
        ap_uint<mem_data_width>* arg1_b1,
        ap_uint<mem_data_width>* arg1_b2,
        ap_uint<mem_data_width>* arg1_b3,
        ap_uint<mem_data_width>* arg1_b4,
        ap_uint<mem_data_width>* arg1_b5,
        ap_uint<mem_data_width>* arg1_b6,
        ap_uint<mem_data_width>* arg1_b7,
        //u
        hls::stream <ap_axiu<axis_data_width,0,0,0>>& arg0_axis_out,
    //u2
        hls::stream <ap_axiu<axis_data_width,0,0,0>>& arg1_axis_in

    )
;
