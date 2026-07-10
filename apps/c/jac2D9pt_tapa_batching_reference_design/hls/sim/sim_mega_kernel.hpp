#pragma once
#include <ops_tapa_kernel_support.h>
#include "../../common/include/common_config.hpp"
#include <datamover_outerloop_0.hpp>
#include <kernel_outerloop_0.hpp>


void sim_mega_kernel(const unsigned int num_beats,
    const unsigned int num_axis_trans,
    const unsigned int loopback_itr,
    const unsigned int outer_iter,
    const unsigned int stencilConfig_total_itr,
    // u
   ::tapa::mmap<::tapa::vec_t<float,mem_vector_factor>> arg0,
    // u2
   ::tapa::mmap<::tapa::vec_t<float,mem_vector_factor>> arg1);