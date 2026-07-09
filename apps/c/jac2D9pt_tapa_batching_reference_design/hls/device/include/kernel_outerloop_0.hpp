// Auto-generated at 2026-07-06 21:11:34.183613 by ops-translator
#pragma once
#include <ops_tapa_kernel_support.h>
#include "../../common/include/common_config.hpp"




void kernel_outerloop_0(
    const unsigned int outer_iter,
    const unsigned int stencilConfig_total_itr,
    ::tapa::istream<::tapa::vec_t<float,vector_factor>>& arg0_axis_in,
    ::tapa::ostream<::tapa::vec_t<float,vector_factor>>& arg1_axis_out
);
