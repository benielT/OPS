// Auto-generated at 2026-11-26 15:29:06.950012 by ops-translator
#pragma once 
#include <ops_hls_rt_support.h>

void inline kernel_poisson_kernel_populate_core(
        const int * idx,
        float& arg0_0,
        float& arg1_0,
        float& arg2_0
)
{

  float x = dx * (float)(idx[0]);
  float y = dy * (float)(idx[1]);

  arg0_0 = myfun(sin(3.14159265358979323846*x),cos(2.0*3.14159265358979323846*y))-1.0;
  arg1_0 = -5.0*3.14159265358979323846*3.14159265358979323846*sin(3.14159265358979323846*x)*cos(2.0*3.14159265358979323846*y);
  arg2_0 = sin(3.14159265358979323846*x)*cos(2.0*3.14159265358979323846*y);

}

void ops_par_loop_poisson_kernel_populate(ops::hls::Block dummyBlock, int dim, int* ops_range,
            ops::hls::Grid<float>& arg1,
            ops::hls::Grid<float>& arg2,
            ops::hls::Grid<float>& arg3
)

{
    ops::hls::AccessRange range;
    opsRange2hlsRange(dim, ops_range, range, arg1.originalProperty);
    constexpr int arg1_0_stencil_offset[] = { 0, 0, 0 };
    constexpr int arg2_0_stencil_offset[] = { 0, 0, 0 };
    constexpr int arg3_0_stencil_offset[] = { 0, 0, 0 };
    getGrid(arg1);
    getGrid(arg2);
    getGrid(arg3);

    for (unsigned short bat = 0; bat < dummyBlock.batch_size; bat++)
    {
            for (unsigned short j = range.start[1]; j < range.end[1]; j++)
            {
                for (unsigned short i = range.start[0]; i < range.end[0]; i++)
                {
                    ops::hls::IdxType idx({i - arg1.originalProperty.d_m[0], j - arg1.originalProperty.d_m[1] - (bat * arg1.originalProperty.grid_size[1]), 0});
                    kernel_poisson_kernel_populate_core(
                        idx,
                        arg1.hostBuffer[getOffset(arg1_0_stencil_offset, arg1.originalProperty, i , j)],
                        arg2.hostBuffer[getOffset(arg2_0_stencil_offset, arg2.originalProperty, i , j)],
                        arg3.hostBuffer[getOffset(arg3_0_stencil_offset, arg3.originalProperty, i , j)]
                    );
                }
            }

        range.start[1] += arg1.originalProperty.grid_size[1];
        range.end[1] += arg1.originalProperty.grid_size[1];
    }

    arg1.isHostBufDirty = true;
    arg2.isHostBufDirty = true;
    arg3.isHostBufDirty = true;

}
