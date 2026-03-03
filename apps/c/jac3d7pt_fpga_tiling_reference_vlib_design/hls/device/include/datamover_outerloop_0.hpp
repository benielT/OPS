// Auto-generated at 2026-01-27 16:58:13.705323 by ops-translator

#pragma once
#include <ops_hls_kernel_support.h>
#include "../../common/include/common_config.hpp"
#include "xf_data_mover/pl_data_mover.hpp"

extern "C" void datamover_outerloop_0(
        const unsigned short range_start_0,
        const unsigned short range_end_0,
        const unsigned short range_start_1,
        const unsigned short range_end_1,
        const unsigned short range_start_2,
        const unsigned short range_end_2,
        const unsigned short gridSize_0,
        const unsigned short gridSize_1,
        const unsigned short gridSize_2,
        const unsigned int outer_itr,
        const unsigned short tile_size_x,
        const unsigned short tile_size_y,
        const unsigned short overlap_size_x,
        const unsigned short overlap_size_y,
        const unsigned short effective_tile_size_x,
        const unsigned short effective_tile_size_y,
        const unsigned short last_tile_size_x,
        const unsigned short last_tile_size_y,
        const unsigned short tile_count_x,
        const unsigned short tile_count_y,
        const unsigned int total_xblocks,
	//u-b1
		::hls::burst_maxi<ap_uint<mem_data_width> > arg0,
	//u2-b1
		::hls::burst_maxi<ap_uint<mem_data_width> > arg1,
    //u
        hls::stream <ap_axiu<mem_data_width,0,0,0>>& arg0_axis_out,
    //u2
        hls::stream <ap_axiu<mem_data_width,0,0,0>>& arg1_axis_in
    )
;
