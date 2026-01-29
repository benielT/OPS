// Auto-generated at 2026-01-27 16:58:13.709899 by ops-translator
#include <ops_hls_kernel_support.h>
#include <kernel_outerloop_0.hpp>

static void joint_PE_outerloop_0(const unsigned short PEId, const ops::hls::StencilConfigCoreTiled & stencilConfig, 
        ::hls::stream<ap_uint<axis_data_width>>& arg0_hls_stream_in, 
        ::hls::stream<ap_uint<axis_data_width>>& arg1_hls_stream_out
)
{
    ::hls::stream<ap_uint<axis_data_width>> node2_1_to_node3_0;
    #pragma HLS STREAM variable = node2_1_to_node3_0 depth = 10

    kernel_jac3D_kernel_stencil_PE(PEId, stencilConfig,
            arg0_hls_stream_in,
            arg1_hls_stream_out
    );
}



static void kernel_outerloop_0_dataflow_region_cascaded(const unsigned short& slr_region, const ops::hls::StencilConfigCoreTiled& stencilConfig,
    ::hls::stream<ap_uint<axis_data_width>> arg0_arg1_streams[iter_par_factor + 1]
)
{
#pragma HLS INLINE 

    const unsigned short PEId_offset = slr_region * iter_par_factor;

    for (int i = 0; i < iter_par_factor; i++)
    {
#pragma HLS UNROLL factor=iter_par_factor
        joint_PE_outerloop_0(PEId_offset + i, stencilConfig,
                arg0_arg1_streams[i],
                arg0_arg1_streams[i+1]
        );
    }
}
static void kernel_outerloop_0_dataflow_region(const unsigned short& slr_region, const ops::hls::StencilConfigCoreTiled& stencilConfig, const unsigned int total_bytes,
hls::stream <ap_axiu<axis_data_width, 0, 0, 0>>& arg0_axis_in, hls::stream <ap_axiu<axis_data_width, 0, 0, 0>>& arg1_axis_out)
{
#pragma HLS DATAFLOW
::hls::stream<ap_uint<axis_data_width>> arg0_arg1_streams[iter_par_factor + 1];
    #pragma HLS STREAM variable = arg0_arg1_streams depth = 120
    
        ops::hls::axis2stream<axis_data_width, axis_data_width>(arg0_axis_in, arg0_arg1_streams[0], total_bytes);

        joint_PE_outerloop_0(slr_region + 0, stencilConfig,
                        arg0_arg1_streams[0],
                        arg0_arg1_streams[1]
                );
        joint_PE_outerloop_0(slr_region + 1, stencilConfig,
                        arg0_arg1_streams[1],
                        arg0_arg1_streams[2]
                );
//        kernel_outerloop_0_dataflow_region_cascaded(slr_region, stencilConfig, arg0_arg1_streams);

        ops::hls::stream2axis<axis_data_width, axis_data_width>(arg1_axis_out, arg0_arg1_streams[2], total_bytes);

}

extern "C" void kernel_outerloop_0(
        const unsigned short slr_region,
        const unsigned int outer_itr,
        const unsigned short stencilConfig_grid_size_0,
        const unsigned short stencilConfig_grid_size_1,
        const unsigned short stencilConfig_grid_size_2,
        const unsigned short stencilConfig_dim,
        const unsigned int stencilConfig_total_itr,
#ifndef OPS_TILING
        const unsigned short stencilConfig_lower_limit_0,
        const unsigned short stencilConfig_lower_limit_1,
        const unsigned short stencilConfig_lower_limit_2,
        const unsigned short stencilConfig_upper_limit_0,
        const unsigned short stencilConfig_upper_limit_1,
        const unsigned short stencilConfig_upper_limit_2,
#endif
        const unsigned short stencilConfig_outer_loop_limit,
#ifndef OPS_TILING
        const unsigned short stencilConfig_batch_size,
#else
        const unsigned short tile_size_x,
        const unsigned short tile_size_y,
        const unsigned short last_tile_size_x,
        const unsigned short last_tile_size_y,
        const unsigned short tile_count_x,
        const unsigned short tile_count_y,
        const unsigned int total_xblocks,
        const unsigned short last_tile_upper_limit_x,
#endif
    //u
        hls::stream <ap_axiu<axis_data_width, 0, 0, 0>>& arg0_axis_in,
    //u2
        hls::stream <ap_axiu<axis_data_width, 0, 0, 0>>& arg1_axis_out
)

{
    #pragma HLS INTERFACE s_axilite port = slr_region bundle = control
    #pragma HLS INTERFACE s_axilite port = outer_itr bundle = control
    #pragma HLS INTERFACE s_axilite port = stencilConfig_grid_size_0 bundle = control
    #pragma HLS INTERFACE s_axilite port = stencilConfig_grid_size_1 bundle = control
    #pragma HLS INTERFACE s_axilite port = stencilConfig_grid_size_2 bundle = control
    #pragma HLS INTERFACE s_axilite port = stencilConfig_dim bundle = control
    #pragma HLS INTERFACE s_axilite port = stencilConfig_total_itr bundle = control
#ifndef OPS_TILING
    #pragma HLS INTERFACE s_axilite port = stencilConfig_lower_limit_0 bundle = control
    #pragma HLS INTERFACE s_axilite port = stencilConfig_lower_limit_1 bundle = control
    #pragma HLS INTERFACE s_axilite port = stencilConfig_lower_limit_2 bundle = control
    #pragma HLS INTERFACE s_axilite port = stencilConfig_upper_limit_0 bundle = control
    #pragma HLS INTERFACE s_axilite port = stencilConfig_upper_limit_1 bundle = control
    #pragma HLS INTERFACE s_axilite port = stencilConfig_upper_limit_2 bundle = control
#endif
    #pragma HLS INTERFACE s_axilite port = stencilConfig_outer_loop_limit bundle = control
#ifndef OPS_TILING
    #pragma HLS INTERFACE s_axilite port = stencilConfig_batch_size bundle = control
#else
    #pragma HLS INTERFACE s_axilite port = tile_size_x bundle = control
    #pragma HLS INTERFACE s_axilite port = tile_size_y bundle = control
    #pragma HLS INTERFACE s_axilite port = last_tile_size_x bundle = control
    #pragma HLS INTERFACE s_axilite port = last_tile_size_y bundle = control
    #pragma HLS INTERFACE s_axilite port = tile_count_x bundle = control
    #pragma HLS INTERFACE s_axilite port = tile_count_y bundle = control
    #pragma HLS INTERFACE s_axilite port = total_xblocks bundle = control
    #pragma HLS INTERFACE s_axilite port = last_tile_upper_limit_x bundle = control
#endif
    

    #pragma HLS INTERFACE axis port = arg0_axis_in register
    #pragma HLS INTERFACE axis port = arg1_axis_out register

    #pragma HLS INTERFACE ap_ctrl_chain port = return
    #pragma HLS INTERFACE s_axilite port = return bundle = control   

    ops::hls::StencilConfigCoreTiled stencilConfig;

    stencilConfig.dim = stencilConfig_dim;
    stencilConfig.grid_size[0] = stencilConfig_grid_size_0;
    stencilConfig.grid_size[1] = stencilConfig_grid_size_1;
    stencilConfig.grid_size[2] = stencilConfig_grid_size_2;
    stencilConfig.outer_loop_limit = stencilConfig_outer_loop_limit;
    stencilConfig.tiling_dim = 2;
    stencilConfig.tile_size[0] = tile_size_x;
    stencilConfig.tile_size[1] = tile_size_y;
    stencilConfig.last_tile_size[0] = last_tile_size_x;
    stencilConfig.last_tile_size[1] = last_tile_size_y;
    stencilConfig.tile_count[0] = tile_count_x;
    stencilConfig.tile_count[1] = tile_count_y;
    stencilConfig.last_tile_upper_limit_x = last_tile_upper_limit_x;
    constexpr unsigned short num_of_pkts_per_beat = mem_data_width / axis_data_width;
    constexpr unsigned short bytes_per_pkt = sizeof(stencil_type) * vector_factor;
    const unsigned int num_beats = total_xblocks;
    const unsigned int num_pkts = num_beats * num_of_pkts_per_beat;
    const unsigned int total_bytes = num_pkts * bytes_per_pkt;

#ifdef DEBUG_LOG
    printf("[KERNEL_DEBUG]|%s| num_beats: %d, num_pkts: %d, total_bytes: %d\n", __func__,
         num_beats, num_pkts, total_bytes);

    printf("[KERNEL_DEBUG]|%s| stencilConfig: dim: %hu, grid_size:(%d,%d,%d), outer_loop_limit: %d, tiling_dim: %hu, tile_size:(%d,%d),\
        last_tile_size:(%d,%d), tile_count:(%d,%d)\n, total_xblocks: %d\n", __func__,
        stencilConfig.dim,
        stencilConfig.grid_size[0], stencilConfig.grid_size[1], stencilConfig.grid_size[2],
        stencilConfig.outer_loop_limit,
        stencilConfig.tiling_dim,
        stencilConfig.tile_size[0], stencilConfig.tile_size[1],
        stencilConfig.last_tile_size[0], stencilConfig.last_tile_size[1],
        stencilConfig.tile_count[0], stencilConfig.tile_count[1], total_xblocks);
#endif

#ifdef DEBUG_LOG
    printf("[KERNEL_DEBUG]|%s| Starting outerloop_0 kernel TOP \n", __func__);
#endif

    for (unsigned int i = 0; i < outer_itr; i++)
    {
        kernel_outerloop_0_dataflow_region(slr_region, stencilConfig, total_bytes, arg0_axis_in, arg1_axis_out);
    }

#ifdef DEBUG_LOG
    printf("[KERNEL_DEBUG]|%s| Ending outerloop_0 kernel TOP \n", __func__);
#endif
}
