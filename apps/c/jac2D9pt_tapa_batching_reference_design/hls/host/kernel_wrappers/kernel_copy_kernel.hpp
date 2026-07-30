// Auto-generated at 2026-07-06 21:11:33.426472 by ops-translator
#pragma once 
#include <ops_hls_rt_support.h>

void inline kernel_kernel_copy_core(
        const float& arg0_0,
        float& arg1_0
)
{

  arg1_0 = arg0_0;

}

void ops_par_loop_kernel_copy(ops::hls::Block dummyBlock, int dim, int* ops_range,
            ops::hls::Grid<float>& arg0,
            ops::hls::Grid<float>& arg1
)

{
    ops::hls::AccessRange range;
    opsRange2hlsRange(dim, ops_range, range, arg0.originalProperty);
    constexpr int arg0_0_stencil_offset[] = { 0, 0, 0 };
    constexpr int arg1_0_stencil_offset[] = { 0, 0, 0 };
    getGrid(arg0);
    getGrid(arg1);

    for (unsigned short bat = 0; bat < dummyBlock.batch_size; bat++)
    {
            for (unsigned short j = range.start[1]; j < range.end[1]; j++)
            {
                for (unsigned short i = range.start[0]; i < range.end[0]; i++)
                {
                    kernel_kernel_copy_core(
                        arg0.hostBuffer[getOffset(arg0_0_stencil_offset, arg0.originalProperty, i , j)],
                        arg1.hostBuffer[getOffset(arg1_0_stencil_offset, arg1.originalProperty, i , j)]
                    );
                }
            }

        range.start[1] += arg0.originalProperty.grid_size[1];
        range.end[1] += arg0.originalProperty.grid_size[1];
    }

    arg1.isHostBufDirty = true;

}
