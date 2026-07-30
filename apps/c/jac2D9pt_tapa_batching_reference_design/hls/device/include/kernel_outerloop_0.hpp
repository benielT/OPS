// Auto-generated at 2026-07-06 21:11:34.183613 by ops-translator
#pragma once
#include <ops_tapa_kernel_support.h>
#include "../../common/include/common_config.hpp"




void kernel_outerloop_0(
    const unsigned short slr_region,
    const unsigned int outer_itr,
    const unsigned short stencilConfig_grid_size_0,
    const unsigned short stencilConfig_grid_size_1,
    const unsigned short stencilConfig_dim,
    const unsigned int stencilConfig_total_itr,
    const unsigned short stencilConfig_lower_limit_0,
    const unsigned short stencilConfig_lower_limit_1,
    const unsigned short stencilConfig_upper_limit_0,
    const unsigned short stencilConfig_upper_limit_1,
    const unsigned short stencilConfig_outer_loop_limit,
    const unsigned short stencilConfig_batch_size,
    //u
    ::tapa::istream<::tapa::vec_t<float,vector_factor>>& arg0_axis_in,
    //u2
    ::tapa::ostream<::tapa::vec_t<float,vector_factor>>& arg1_axis_out
);
