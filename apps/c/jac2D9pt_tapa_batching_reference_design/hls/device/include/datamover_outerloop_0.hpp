// Auto-generated at 2026-07-06 21:11:34.180908 by ops-translator


#pragma once
#include <ops_tapa_kernel_support.h>
#include "../../common/include/common_config.hpp"


void datamover_outerloop_0(
    const unsigned int num_beats,
    const unsigned int num_axis_trans,
    const unsigned int outerloop_itr,
    // u
// #if defined(TAPA_SW_EMU) || defined(TAPA_HW_EMU)
//    ::tapa::mmap<::tapa::vec_t<float,mem_vector_factor>>& arg0,
//     // u2
//    ::tapa::mmap<::tapa::vec_t<float,mem_vector_factor>>& arg1,
// #else
   ::tapa::mmap<::tapa::vec_t<float,mem_vector_factor>> arg0,
    // u2
   ::tapa::mmap<::tapa::vec_t<float,mem_vector_factor>> arg1,
// #endif
    // u (External Output)
   ::tapa::ostream<::tapa::vec_t<float,vector_factor>>& arg0_axis_out,
    // u2 (External Input)
   ::tapa::istream<::tapa::vec_t<float,vector_factor>>& arg1_axis_in
);
