// Auto-generated at 2026-11-26 15:29:07.054446 by ops-translator

#pragma once

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <ops_hls_rt_support.h>

extern float dx;
extern float dy;
extern float dy_2;
extern float dx_2;
extern float dx_2_plus_dy_2_mult_2;
extern float dx_2_dy_2;

// user kernel files
#include "poisson_kernel_populate_kernel.hpp"
#include "poisson_kernel_update_kernel.hpp"
#include "poisson_kernel_initialguess_kernel.hpp"
#include "outerloop_0.hpp"

