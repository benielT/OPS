
// #define DEBUG_LOG
#include "../../../include/device/ops_hls_kernel_support.h"

#define MEM_VECTOR_SIZE 4
#define AXI_M_WIDTH 128
#define AXIS_WIDTH 64
#define HLS_STREAM_WIDTH 128

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

