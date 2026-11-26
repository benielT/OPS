/*
* Open source copyright declaration based on BSD open source template:
* http://www.opensource.org/licenses/bsd-license.php
*
* This file is part of the OPS distribution.
*
* Copyright (c) 2013, Mike Giles and others. Please see the AUTHORS file in
* the main source directory for a full list of copyright holders.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* * Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * The name of Mike Giles may not be used to endorse or promote products
* derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Mike Giles ''AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Mike Giles BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/** @Test application for fpga batched temporal blocked poisson 2D
  * @author Beniel Thileepan
  */

#pragma once

#define EPSILON 0.0001
typedef float stencil_type;
extern float dx,dy,dx_2,dy_2,dx_2_dy_2,dx_2_plus_dy_2_mult_2;
extern const unsigned short mem_vector_factor;

void poisson_kernel_populate_cpu(stencil_type* u, stencil_type* f, stencil_type* ref, int size[2], int d_m[2], int d_p[2], int range[4], int batch_size)
{
    int grid_size_y = size[1] - d_m[1] + d_p[1];
#ifdef OPS_FPGA
    int grid_size_x = ((size[0] - d_m[0] + d_p[0] + mem_vector_factor - 1) / mem_vector_factor) * mem_vector_factor;
#else
    int grid_size_x = size[0] - d_m[0] + d_p[0];
#endif
    int actual_size_x = size[0] - d_m[0] + d_p[0];

    for (int bat = 0; bat < batch_size; bat++)
    {
        int offset = bat * grid_size_x * grid_size_y;
        for (int j = range[2] - d_m[1]; j < range[3] -d_m[1]; j++)
        {
            for (int i = range[0] - d_m[0]; i < range[1] - d_m[0]; i++)
            {
                int index = j * grid_size_x + i + offset;
                stencil_type x = dx * (stencil_type)(i + d_m[0]);
                stencil_type y = dy * (stencil_type)(j + d_m[1]);

                u[index] = myfun(sin(M_PI*x),cos(2.0*M_PI*y))-1.0;
                f[index] = -5.0*M_PI*M_PI*sin(M_PI*x)*cos(2.0*M_PI*y);
                ref[index] = sin(M_PI*x)*cos(2.0*M_PI*y);
                // u[index] = myfun(sin(M_PI * x), cos(2.0 * M_PI * y))-1.0;
                // f[index] = (-5.0*M_PI*M_PI*sin(M_PI*x)*cos(2.0*M_PI*y)); // * dx_2_dy_2;
                // ref[index] = sin(M_PI*x)*cos(2.0*M_PI*y);
            }
        }
    }
}

void poisson_kernel_populate_cpu_test(stencil_type* u, stencil_type* f, stencil_type* ref, int size[2], int d_m[2], int d_p[2], int range[4], int batch_size)
{
    int grid_size_y = size[1] - d_m[1] + d_p[1];
#ifdef OPS_FPGA
    int grid_size_x = ((size[0] - d_m[0] + d_p[0] + mem_vector_factor - 1) / mem_vector_factor) * mem_vector_factor;
#else
    int grid_size_x = size[0] - d_m[0] + d_p[0];
#endif
    int actual_size_x = size[0] - d_m[0] + d_p[0];

    for (int bat = 0; bat < batch_size; bat++)
    {
        int offset = bat * grid_size_x * grid_size_y;
        for (int j = range[2] - d_m[1]; j < range[3] -d_m[1]; j++)
        {
            for (int i = range[0] - d_m[0]; i < range[1] - d_m[0]; i++)
            {
                int index = j * grid_size_x + i + offset;
                int grid_idx = j * grid_size_x + i;
                // stencil_type x = dx * (stencil_type)(i + d_m[0]);
                // stencil_type y = dy * (stencil_type)(j + d_m[1]);

                u[index] = grid_idx;
                f[index] = grid_idx * 2;
                ref[index] = grid_idx * 3;
                // u[index] = myfun(sin(M_PI * x), cos(2.0 * M_PI * y))-1.0;
                // f[index] = (-5.0*M_PI*M_PI*sin(M_PI*x)*cos(2.0*M_PI*y)); // * dx_2_dy_2;
                // ref[index] = sin(M_PI*x)*cos(2.0*M_PI*y);
            }
        }
    }
}

void poisson_kernel_initialguess_cpu(stencil_type* d, int size[2], int d_m[2], int d_p[2], int range[4], int batch_size)
{
    int grid_size_y = size[1] - d_m[1] + d_p[1];
#ifdef OPS_FPGA
    int grid_size_x = ((size[0] - d_m[0] + d_p[0] + mem_vector_factor - 1) / mem_vector_factor) * mem_vector_factor;
#else
    int grid_size_x = size[0] - d_m[0] + d_p[0];
#endif
    int actual_size_x = size[0] - d_m[0] + d_p[0];

    for (int bat = 0; bat < batch_size; bat++)
    {
        int offset = bat * grid_size_x * grid_size_y;
        for (int j = range[2] - d_m[1]; j < range[3] -d_m[1]; j++)
        {
            for (int i = range[0] - d_m[0]; i < range[1] - d_m[0]; i++)
            {
                int index = j * grid_size_x + i + offset;
                d[index] = 0.0; // Initial guess is zero
            }
        }
    }
}

void poisson_kernel_stencil_cpu(stencil_type* u, stencil_type* f, stencil_type* u2, int size[2], int d_m[2], int d_p[2], int range[4], int batch_size)
{
    int grid_size_y = size[1] - d_m[1] + d_p[1];
#ifdef OPS_FPGA
    int grid_size_x = ((size[0] - d_m[0] + d_p[0] + mem_vector_factor - 1) / mem_vector_factor) * mem_vector_factor;
#else
    int grid_size_x = size[0] - d_m[0] + d_p[0];
#endif
    int actual_size_x = size[0] - d_m[0] + d_p[0];

    for (int bat = 0; bat < batch_size; bat++)
    {
        int offset = bat * grid_size_x * grid_size_y;

        for (int j = range[2] - d_m[1]; j < range[3] -d_m[1]; j++)
        {
            for (int i = range[0] - d_m[0]; i < range[1] - d_m[0]; i++)
            {
                int index = j * grid_size_x + i + offset;

                if(i == 0 || j == 0 || i == actual_size_x -1  || j==grid_size_y-1)
                {
                    u2[index] = u[index];
                } 
                else 
                {
                    float tmp1_1 = u[index - 1]+ u[index + 1];
                    float tmp1_2 = u[index + grid_size_x] + u[index - grid_size_x];
                    float tmp1 = tmp1_1 + tmp1_2;
                    float tmp2 = ldexpf(u[index],-1); //equalent div 0.5f
                    float tmp3 = ldexpf(tmp1,-3); //equalent div 0.125f
                    u2[index] = tmp2 + tmp3;
                }
            }
        }
    }
}

void poisson_kernel_stencil_cpu_test(stencil_type* u, stencil_type* f, stencil_type* u2, int size[2], int d_m[2], int d_p[2], int range[4], int batch_size)
{
    int grid_size_y = size[1] - d_m[1] + d_p[1];
#ifdef OPS_FPGA
    int grid_size_x = ((size[0] - d_m[0] + d_p[0] + mem_vector_factor - 1) / mem_vector_factor) * mem_vector_factor;
#else
    int grid_size_x = size[0] - d_m[0] + d_p[0];
#endif
    int actual_size_x = size[0] - d_m[0] + d_p[0];

    for (int bat = 0; bat < batch_size; bat++)
    {
        int offset = bat * grid_size_x * grid_size_y;

        for (int j = range[2] - d_m[1]; j < range[3] -d_m[1]; j++)
        {
            for (int i = range[0] - d_m[0]; i < range[1] - d_m[0]; i++)
            {
                int index = j * grid_size_x + i + offset;

                if(i == 0 || j == 0 || i == actual_size_x -1  || j==grid_size_y-1)
                {
                    u2[index] = u[index];
                } 
                else 
                {
                    u2[index] = u[index] + 1;
                }
            }
        }
    }
}

void poisson_kernel_update_cpu(stencil_type* u2, stencil_type* u, int size[2], int d_m[2], int d_p[2], int range[4], int batch_size)
{
    int grid_size_y = size[1] - d_m[1] + d_p[1];
#ifdef OPS_FPGA
    int grid_size_x = ((size[0] - d_m[0] + d_p[0] + mem_vector_factor - 1) / mem_vector_factor) * mem_vector_factor;
#else
    int grid_size_x = size[0] - d_m[0] + d_p[0];
#endif
    int actual_size_x = size[0] - d_m[0] + d_p[0];

    for (int bat = 0; bat < batch_size; bat++)
    {
        int offset = bat * grid_size_x * grid_size_y;
        for (int j = range[2] - d_m[1]; j < range[3] -d_m[1]; j++)
        {
            for (int i = range[0] - d_m[0]; i < range[1] - d_m[0]; i++)
            {
                int index = j * grid_size_x + i + offset;
                u[index] = u2[index];
            }
        }
    }
}


bool verify(stencil_type * grid_data1, stencil_type *  grid_data2, int size[2], int d_m[2], int d_p[2], int range[4], int batch_size)
{
    bool passed = true;
    int grid_size_y = size[1] - d_m[1] + d_p[1];
#ifdef OPS_FPGA
    int grid_size_x = ((size[0] - d_m[0] + d_p[0] + mem_vector_factor - 1) / mem_vector_factor) * mem_vector_factor;
#else
    int grid_size_x = size[0] - d_m[0] + d_p[0];
#endif

    for (int bat = 0; bat < batch_size; bat++)
    {
        int offset = bat * grid_size_x * grid_size_y;
        for (int j = range[2] - d_m[1]; j < range[3] -d_m[1]; j++)
        {
            for (int i = range[0] - d_m[0]; i < range[1] - d_m[0]; i++)
            {
                int index = j * grid_size_x + i + offset;

                if (abs(grid_data1[index] - grid_data2[index]) > EPSILON)
                {
                    std::cerr << "[ERROR] value Mismatch index: (" << i << ", " << j << "), grid_data1: "
                            << grid_data1[index] << ", and grid_data2: " << grid_data2[index] << std::endl;
                    passed = false;
                }
            }
        }
    }

    return passed;
}
