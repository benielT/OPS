// Auto-generated at 2026-11-26 15:29:08.157846 by ops-translator
#pragma once
#include "PE_poisson_kernel_stencil.hpp"

extern "C" void kernel_outerloop_0_0(
    const unsigned short slr_region, const unsigned int outer_itr,
    const unsigned short stencilConfig_grid_size_0,
    const unsigned short stencilConfig_grid_size_1,
    const unsigned short stencilConfig_dim,
    const unsigned int stencilConfig_total_itr,
#ifndef OPS_TILING
    const unsigned short stencilConfig_lower_limit_0,
    const unsigned short stencilConfig_lower_limit_1,
    const unsigned short stencilConfig_upper_limit_0,
    const unsigned short stencilConfig_upper_limit_1,
#endif
    const unsigned short stencilConfig_outer_loop_limit,
#ifndef OPS_TILING
    const unsigned short stencilConfig_batch_size,
#else
    const unsigned short tile_size_x, const unsigned short last_tile_size_x,
    const unsigned short tile_count_x, const unsigned int total_xblocks,
    const unsigned short last_tile_upper_limit_x,
#endif
    // u
    hls::stream<ap_axiu<axis_data_width, 0, 0, 0>> &arg0_axis_in,
    // u2
    hls::stream<ap_axiu<axis_data_width, 0, 0, 0>> &arg1_axis_out);
