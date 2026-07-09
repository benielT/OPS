// Auto-generated at 2026-07-06 21:11:34.184791 by ops-translator
#include <ops_tapa_kernel_support.h>
#include <kernel_outerloop_0.hpp>
 



// static void kernel_outerloop_0_dataflow_region_cascaded(const unsigned short slr_region, const ops::hls::StencilConfigCore stencilConfig,
//     ::hls::stream<ap_uint<axis_data_width>> arg0_arg1_streams[iter_par_factor + 1]
// )
// {
// #pragma HLS INLINE 

//     const unsigned short PEId_offset = slr_region;
//         ::hls::stream<ap_uint<axis_data_width>> node2_1_to_node3_0[iter_par_factor];
//     #pragma HLS STREAM variable = node2_1_to_node3_0 depth = 10


//     for (int i = 0; i < iter_par_factor; i++)
//     {
// #pragma HLS UNROLL factor=iter_par_factor
//             //const unsigned short PEId_offset_i = PEId_offset + i;
//     kernel_jac2D_kernel_stencil_PE(
//             PEId_offset, i, 
//             stencilConfig,
//             arg0_arg1_streams[i],
//             arg0_arg1_streams[i+1]
//     );

//     }
// }

// static void kernel_outerloop_0_dataflow_region(const unsigned short& slr_region, const ops::hls::StencilConfigCore& stencilConfig, const unsigned int num_pkts,
//         hls::stream <ap_axiu<axis_data_width, 0, 0, 0>>& arg0_axis_in,        hls::stream <ap_axiu<axis_data_width, 0, 0, 0>>& arg1_axis_out)
// {
// #pragma HLS DATAFLOW
//     ::hls::stream<ap_uint<axis_data_width>> arg0_arg1_streams[iter_par_factor + 1];
//     #pragma HLS STREAM variable = arg0_arg1_streams depth = 258
    
//         ops::hls::axis2stream<axis_data_width>(arg0_axis_in, arg0_arg1_streams[0], num_pkts);

//         kernel_outerloop_0_dataflow_region_cascaded(slr_region, stencilConfig, 
//  arg0_arg1_streams);

//         ops::hls::stream2axis<axis_data_width>(arg0_arg1_streams[iter_par_factor], arg1_axis_out,  num_pkts);

// }

// static void kernel_outerloop_0_main_region(const unsigned short& slr_region, const unsigned int& outer_itr, const ops::hls::StencilConfigCore& stencilConfig, 
// const unsigned int num_pkts, 
//         hls::stream <ap_axiu<axis_data_width, 0, 0, 0>>& arg0_axis_in,
//         hls::stream <ap_axiu<axis_data_width, 0, 0, 0>>& arg1_axis_out
//     )
// {

//     for (unsigned int i = 0; i < outer_itr; i++)
//     {

//             kernel_outerloop_0_dataflow_region(slr_region, stencilConfig, 
//                 num_pkts,
 
//                 arg0_axis_in, 
//                 arg1_axis_out
// );
//     }
// }
// extern "C" void kernel_outerloop_0
// (
//         const unsigned short slr_region,
//         const unsigned int outer_itr,
//         const unsigned short stencilConfig_grid_size_0,
//         const unsigned short stencilConfig_grid_size_1,
//         const unsigned short stencilConfig_dim,
//         const unsigned int stencilConfig_total_itr,
// #ifndef OPS_TILING
//         const unsigned short stencilConfig_lower_limit_0,
//         const unsigned short stencilConfig_lower_limit_1,
//         const unsigned short stencilConfig_upper_limit_0,
//         const unsigned short stencilConfig_upper_limit_1,
// #endif
//         const unsigned short stencilConfig_outer_loop_limit,
// #ifndef OPS_TILING
//         const unsigned short stencilConfig_batch_size,
// #else
//         const unsigned short tile_size_x,
//         const unsigned short last_tile_size_x,
//         const unsigned short tile_count_x,
//         const unsigned int total_xblocks,
//         const unsigned short last_tile_upper_limit_x,
// #endif
//     //u
//         hls::stream <ap_axiu<axis_data_width, 0, 0, 0>>& arg0_axis_in,
//     //u2
//         hls::stream <ap_axiu<axis_data_width, 0, 0, 0>>& arg1_axis_out
// )

// {
//     #pragma HLS INTERFACE s_axilite port = slr_region bundle = control
//     #pragma HLS INTERFACE s_axilite port = outer_itr bundle = control
//     #pragma HLS INTERFACE s_axilite port = stencilConfig_grid_size_0 bundle = control
//     #pragma HLS INTERFACE s_axilite port = stencilConfig_grid_size_1 bundle = control
//     #pragma HLS INTERFACE s_axilite port = stencilConfig_dim bundle = control
//     #pragma HLS INTERFACE s_axilite port = stencilConfig_total_itr bundle = control
// #ifndef OPS_TILING
//     #pragma HLS INTERFACE s_axilite port = stencilConfig_lower_limit_0 bundle = control
//     #pragma HLS INTERFACE s_axilite port = stencilConfig_lower_limit_1 bundle = control
//     #pragma HLS INTERFACE s_axilite port = stencilConfig_upper_limit_0 bundle = control
//     #pragma HLS INTERFACE s_axilite port = stencilConfig_upper_limit_1 bundle = control
// #endif
//     #pragma HLS INTERFACE s_axilite port = stencilConfig_outer_loop_limit bundle = control
// #ifndef OPS_TILING
//     #pragma HLS INTERFACE s_axilite port = stencilConfig_batch_size bundle = control
// #else
//     #pragma HLS INTERFACE s_axilite port = tile_size_x bundle = control
//     #pragma HLS INTERFACE s_axilite port = last_tile_size_x bundle = control
//     #pragma HLS INTERFACE s_axilite port = tile_count_x bundle = control
//     #pragma HLS INTERFACE s_axilite port = total_xblocks bundle = control
//     #pragma HLS INTERFACE s_axilite port = last_tile_upper_limit_x bundle = control
// #endif
    

//     #pragma HLS INTERFACE axis port = arg0_axis_in register register_mode=both
//     #pragma HLS INTERFACE axis port = arg1_axis_out register register_mode=both

//     #pragma HLS INTERFACE ap_ctrl_chain port = return
//     #pragma HLS INTERFACE s_axilite port = return bundle = control   

//     ops::hls::StencilConfigCore stencilConfig;

//     stencilConfig.dim = stencilConfig_dim;
//     stencilConfig.grid_size[0] = stencilConfig_grid_size_0;
//     stencilConfig.grid_size[1] = stencilConfig_grid_size_1;
//     stencilConfig.lower_limit[0] = stencilConfig_lower_limit_0;
//     stencilConfig.lower_limit[1] = stencilConfig_lower_limit_1;
//     stencilConfig.upper_limit[0] = stencilConfig_upper_limit_0;
//     stencilConfig.upper_limit[1] = stencilConfig_upper_limit_1;
//     stencilConfig.total_itr = stencilConfig_total_itr;
//     stencilConfig.outer_loop_limit = stencilConfig_outer_loop_limit;
//     stencilConfig.batch_size = stencilConfig_batch_size;
//     //constexpr unsigned short num_of_pkts_per_beat = mem_data_width / axis_data_width;
//     unsigned int tmp1 = stencilConfig_batch_size;
//     unsigned int tmp2 = stencilConfig_total_itr;
//     unsigned int num_pkts = tmp1 * tmp2;

// #ifdef DEBUG_LOG
//     unsigned int num_beats = num_pkts / num_of_pkts_per_beat;
//     printf("[KERNEL_DEBUG]|%s| num_beats: %d, num_pkts: %d\n", __func__,
//          num_beats, num_pkts);
//     printf("[KERNEL_DEBUG]|%s| stencilConfig: dim: %hu, grid_size:(%d,%d,%d), lower_limit:(%d,%d,%d), upper_limit:(%d,%d,%d), \
//             total_itr: %d, outer_loop_limit: %d, batch_size: %d\n", __func__,
//         stencilConfig.dim,
//         stencilConfig.grid_size[0], stencilConfig.grid_size[1], stencilConfig.grid_size[2],
//         stencilConfig.lower_limit[0], stencilConfig.lower_limit[1], stencilConfig.lower_limit[2],
//         stencilConfig.upper_limit[0], stencilConfig.upper_limit[1], stencilConfig.upper_limit[2],
//         stencilConfig.total_itr,
//         stencilConfig.outer_loop_limit,
//         stencilConfig.batch_size);
// #endif

// #ifdef DEBUG_LOG
//     printf("[KERNEL_DEBUG]|%s| Starting outerloop_0 kernel TOP \n", __func__);
// #endif

//     kernel_outerloop_0_main_region(slr_region, outer_itr, stencilConfig, 
// num_pkts, 
//                 arg0_axis_in, 
//                 arg1_axis_out
// );

// #ifdef DEBUG_LOG
//     printf("[KERNEL_DEBUG]|%s| Ending outerloop_0 kernel TOP \n", __func__);
// #endif
// }

// just a tapa topfunction return read and write back with number of packats

void axis_to_stream(const unsigned int outer_iter, const unsigned int stencilConfig_total_itr,::tapa::istream<::tapa::vec_t<float,vector_factor>>& arg0_axis_in, ::tapa::ostream<::tapa::vec_t<float,vector_factor>>& arg0_hls_out) {
    size_t total_iteration = outer_iter * stencilConfig_total_itr;
    // for (size_t i = 0; i < total_iteration; ) {
    //     // non-blocking tapa api used
    //     ap_uint<axis_data_width> data;
    //     bool ok = arg0_axis_in.try_read(data);
    //     if (ok) {
    //         while (!arg0_hls_out.try_write(data)){};
    //         i++;
    //     }
    // }
    for (size_t i = 0; i < total_iteration; i++) {
        ::tapa::vec_t<float,vector_factor> data;
        arg0_axis_in.read(data);
        arg0_hls_out.write(data);
    }
}

void stream_to_axis(const unsigned int outer_itr, const unsigned int stencilConfig_total_itr,::tapa::istream<::tapa::vec_t<float,vector_factor>>& arg0_hls_in,::tapa::ostream<::tapa::vec_t<float,vector_factor>>& arg0_axis_out) {
    size_t total_iteration = outer_itr * stencilConfig_total_itr;
    
    for (size_t i = 0; i < total_iteration; i++) {
        ::tapa::vec_t<float,vector_factor> data;
        arg0_hls_in.read(data);
        arg0_axis_out.write(data);
    }
}


void kernel_outerloop_0(
    const unsigned int outer_iter,
    const unsigned int stencilConfig_total_itr,
    ::tapa::istream<::tapa::vec_t<float,vector_factor>>& arg0_axis_in,
    ::tapa::ostream<::tapa::vec_t<float,vector_factor>>& arg1_axis_out
) {
   ::tapa::stream<::tapa::vec_t<float,vector_factor>> arg0_arg1_internal_stream("arg0_arg1_internal_stream");

   ::tapa::task()
        .invoke(axis_to_stream, outer_iter, stencilConfig_total_itr, arg0_axis_in, arg0_arg1_internal_stream)
        .invoke(stream_to_axis, outer_iter, stencilConfig_total_itr, arg0_arg1_internal_stream, arg1_axis_out);

}