
#pragma once

// #define DEBUG_LOG
#include "../../../include/device/ops_hls_kernel_support.h"

#define AXI_M_WIDTH 512
#define AXIS_WIDTH 256
#define DATA_WIDTH 32
#define MAX_P_SLR 30
#define MIN_P_SLR 1
#define MAX_SLR_NUM 3

//template <unsigned int MEM_DATA_WIDTH, unsigned int STREAM_DATA_WIDTH, unsigned int BURST_SIZE=32>
void dut(ops::hls::SizeType& gridSize,
		ops::hls::AccessRange& range,
        ops::hls::SizeType2d& tile_size,
        ops::hls::SizeType2d& tile_count,
        ops::hls::SizeType2d& overlap_size,
        ops::hls::SizeType2d& effective_tile_size,
        ops::hls::SizeType2d& last_tile_size,
        ops::hls::MemConfigTile& memconfig);