
#pragma once
// #define DEBUG_LOG
#include "../../../include/device/ops_hls_kernel_support.h"


#define MEM_VECTOR_SIZE 4
#define AXI_M_WIDTH 128
#define AXIS_WIDTH 128

#define STENCIL_SIZE 3
#define MAX_SLR_NUM 3
#define MIN_P_SLR 2
#define MAX_P_SLR 3

typedef float stencil_type;
constexpr unsigned short data_width = sizeof(stencil_type) * 8;
constexpr unsigned short max_depth = max_depth_bytes / data_width;
constexpr unsigned short line_buff_2d_depth = max_depth;
constexpr unsigned short line_buff_3d_depth = max_depth / 2;

constexpr unsigned short shift_bits = 2; // log2(MEM_VECTOR_SIZE)
constexpr unsigned short axis_data_width = AXIS_WIDTH;
constexpr unsigned short mem_data_width = AXI_M_WIDTH;
// constexpr unsigned short data_width = 32; // float
constexpr unsigned short iter_par_factor = 5; // number of parallel iterations
constexpr unsigned short vector_factor = MEM_VECTOR_SIZE; // number of elements processed in parallel within a single iteration

void dut(ap_uint<AXI_M_WIDTH>* mem_in_b1,
        ap_uint<AXI_M_WIDTH>* mem_in_b2,
		ap_uint<AXI_M_WIDTH>* mem_out_b1,
        ap_uint<AXI_M_WIDTH>* mem_out_b2,
        const unsigned short dim,
        const unsigned short grid_x_size,
        const unsigned short grid_y_size,
        const unsigned short grid_z_size,
        const unsigned short range_start_0,
        const unsigned short range_end_0,
        const unsigned short range_start_1,
        const unsigned short range_end_1,
        const unsigned short range_start_2,
        const unsigned short range_end_2,
        const unsigned short tile_size_x,
        const unsigned short tile_size_y,
        const unsigned short overlap_size_x,
        const unsigned short overlap_size_y,
        const unsigned short effective_tile_size_x,
        const unsigned short effective_tile_size_y,
        const unsigned short last_tile_size_x,
        const unsigned short last_tile_size_y,
        const unsigned short tile_count_x,
        const unsigned short tile_count_y);

