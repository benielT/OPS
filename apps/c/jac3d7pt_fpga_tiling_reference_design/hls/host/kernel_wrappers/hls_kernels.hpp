// Auto-generated at 2025-09-06 21:18:49.555818 by ops-translator

#pragma once

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <ops_hls_rt_support.h>

typedef float stencil_type;
constexpr unsigned short vector_factor = 8;
constexpr unsigned short mem_vector_factor = 16;
constexpr unsigned int iter_par_factor = 9;

// user kernel files
#include "kernel_populate_kernel.hpp"
#include "kernel_initialguess_kernel.hpp"
#include "kernel_copy_kernel.hpp"
#include "outerloop_0.hpp"

