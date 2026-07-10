// // Auto-generated at 2026-07-06 21:11:34.182424 by ops-translator

// #include <datamover_outerloop_0.hpp>
// static void datamover_outerloop_0_dataflow_region_read(
//         const unsigned int num_pkts,
//         const ops::hls::MemConfig& memconfig,
//         ::tapa::vec_t<stencil_type, mem_vector_factor>* arg0,
//         hls::stream<ap_axiu<vector_factor, 0, 0, 0>>& arg0_axis_out)
// {
// #pragma HLS DATAFLOW
//     ::hls::stream<::tapa::vec_t<stencil_type, mem_vector_factor>> arg0_read_mem_strm;
//     #pragma HLS STREAM variable = arg0_read_mem_strm depth = 256
//     ::hls::stream<::tapa::vec_t<stencil_type, vector_factor>> arg0_read_reduced_mem_strm;
//     #pragma HLS STREAM variable = arg0_read_reduced_mem_strm depth = 512
//     ops::hls::mem2stream<mem_vector_factor, 32, 2>(arg0, arg0_read_mem_strm, memconfig.total_xblocks);
    
//     ops::hls::stream2streamStepdown<mem_vector_factor, vector_factor>(arg0_read_mem_strm, arg0_read_reduced_mem_strm, memconfig.total_xblocks);
//     ops::hls::stream2axis<vector_factor>(arg0_read_reduced_mem_strm, arg0_axis_out, num_pkts);
// }

// static void datamover_outerloop_0_dataflow_region_write(
//         const unsigned int num_pkts,
//         const ops::hls::MemConfig& memconfig,
//         ::tapa::vec_t<stencil_type, mem_vector_factor>* arg1,
//         hls::stream<ap_axiu<vector_factor, 0, 0, 0>>& arg1_axis_in)
// {
//     ::hls::stream<::tapa::vec_t<stencil_type, mem_vector_factor>> arg1_write_mem_strm;
//     #pragma HLS STREAM variable = arg1_write_mem_strm depth = 256
//     ::hls::stream<::tapa::vec_t<stencil_type, vector_factor>> arg1_write_reduced_mem_strm;
//     #pragma HLS STREAM variable = arg1_write_reduced_mem_strm depth = 512

// #pragma HLS DATAFLOW
//     ops::hls::axis2stream<vector_factor>(arg1_axis_in, arg1_write_reduced_mem_strm, num_pkts);
    
//     ops::hls::stream2streamStepup<vector_factor, mem_vector_factor>(arg1_write_reduced_mem_strm, arg1_write_mem_strm, memconfig.total_xblocks);
//     ops::hls::stream2mem<mem_vector_factor, 32, 2>(arg1, arg1_write_mem_strm, memconfig.total_xblocks);
// }



// static void datamover_outerloop_0_dataflow_read_write_dataflow_region(
//         const unsigned int num_pkts,
//         const ops::hls::MemConfig& memconfig,
//         ::tapa::vec_t<stencil_type, mem_vector_factor>* arg0,
//         ::tapa::vec_t<stencil_type, mem_vector_factor>* arg1,
//             //u
//         hls::stream <ap_axiu<vector_factor,0,0,0>>& arg0_axis_out,
//     //u2
//         hls::stream <ap_axiu<vector_factor,0,0,0>>& arg1_axis_in
// )    
// {
//     #pragma HLS DATAFLOW
//     ::hls::stream<::tapa::vec_t<stencil_type, mem_vector_factor>> arg0_read_mem_strm;
//     #pragma HLS STREAM variable = arg0_read_mem_strm depth = 262
//     ::hls::stream<::tapa::vec_t<stencil_type, vector_factor>> arg0_read_reduced_mem_strm;
//     #pragma HLS STREAM variable = arg0_read_reduced_mem_strm depth = 518
//     ::hls::stream<::tapa::vec_t<stencil_type, vector_factor>> arg1_write_reduced_mem_strm;
//     #pragma HLS STREAM variable = arg1_write_reduced_mem_strm depth = 518
//     ::hls::stream<::tapa::vec_t<stencil_type, mem_vector_factor>> arg1_write_mem_strm;
//     #pragma HLS STREAM variable = arg1_write_mem_strm depth = 262

//         ops::hls::mem2stream<mem_vector_factor, 64, 2>(arg0, arg0_read_mem_strm, memconfig.total_xblocks);
//         ops::hls::stream2streamStepdown<mem_vector_factor, vector_factor>(arg0_read_mem_strm, arg0_read_reduced_mem_strm, memconfig.total_xblocks);
//         ops::hls::stream2axis<vector_factor>(arg0_read_reduced_mem_strm, arg0_axis_out, num_pkts);

//         ops::hls::axis2stream<vector_factor>(arg1_axis_in, arg1_write_reduced_mem_strm, num_pkts);
    
//         ops::hls::stream2streamStepup<vector_factor, mem_vector_factor>(arg1_write_reduced_mem_strm, arg1_write_mem_strm, memconfig.total_xblocks);
//         ops::hls::stream2mem<mem_vector_factor, 64, 2>(arg1, arg1_write_mem_strm, memconfig.total_xblocks);

// #ifdef DEBUG_LOG
// #ifndef __SYNTHESIS__
//     // -------------------------------------------------------------------------
//     // HLS SIMULATION DEBUG BLOCK: Check for hanging streams
//     // -------------------------------------------------------------------------

//     if(!arg0_read_mem_strm.empty())
//         printf("[SIM WARNING] arg0_read_mem_strm has %zu leftover elements!\n", arg0_read_mem_strm.size());
//      if(!arg0_read_reduced_mem_strm.empty())
//         printf("[SIM WARNING] arg0_read_reduced_mem_strm has %zu leftover elements!\n", arg0_read_reduced_mem_strm.size());
//     if(!arg1_write_reduced_mem_strm.empty())
//         printf("[SIM WARNING] arg1_write_reduced_mem_strm has %zu leftover elements!\n", arg1_write_reduced_mem_strm.size());
//     if(!arg1_write_mem_strm.empty())
//         printf("[SIM WARNING] arg1_write_mem_strm has %zu leftover elements!\n", arg1_write_mem_strm.size());
//     // -------------------------------------------------------------------------
// #endif
// #endif
// }


// static void datamover_outerloop_0_dataflow_read_write(
//         const unsigned int iter,
//         const unsigned int num_pkts,
//         const ops::hls::MemConfig& memconfig,
//         ::tapa::vec_t<stencil_type, mem_vector_factor>* arg0,
//         ::tapa::vec_t<stencil_type, mem_vector_factor>* arg1,
//         //u
//         hls::stream <ap_axiu<vector_factor,0,0,0>>& arg0_axis_out,
//     //u2
//         hls::stream <ap_axiu<vector_factor,0,0,0>>& arg1_axis_in
// )    
// {
//     //TODO: memconfig.start_offset need to be handled for multibank tiling if start_offset != 0. In all our cased start_offset == 0 for now
//     const unsigned int iter_by_2 = iter >> 1;
//     for (unsigned int i = 0; i < iter_by_2; i++)
//     {
//     #ifdef DEBUG_LOG
//         printf("[KERNEL_DEBUG]|%s| Calling datamover. i:%d\n", __func__, i);
//     #endif


//             datamover_outerloop_0_dataflow_read_write_dataflow_region(
//                     num_pkts,
//                     memconfig,
//                     arg0,
//                     arg1,
//                     arg0_axis_out,
//                 arg1_axis_in

//             );


//             datamover_outerloop_0_dataflow_read_write_dataflow_region(
//                     num_pkts,
//                     memconfig,
//                     arg1,
//                     arg0,
//                     arg0_axis_out,
//                 arg1_axis_in

//             );
//     }
// }

// static void datamover_outerloop_0_loopback_dataflow_region(
//         const unsigned int num_pkts
// ,
//         hls::stream<ap_axiu<vector_factor, 0, 0, 0>>& arg0_axis_out,
//         hls::stream<ap_axiu<vector_factor, 0, 0, 0>>& arg1_axis_in)
// {
//     ::hls::stream<::tapa::vec_t<stencil_type, vector_factor>> arg0_mem_strm;
//     #pragma HLS STREAM variable = arg0_mem_strm
// #pragma HLS DATAFLOW
//     ops::hls::axis2stream<vector_factor>(arg1_axis_in, arg0_mem_strm, num_pkts);
//     ops::hls::stream2axis<vector_factor>(arg0_mem_strm, arg0_axis_out, num_pkts);
// }

// static void datamover_outerloop_0_loopback(
//         const unsigned int iter,
//         const unsigned int num_pkts
// ,
//         hls::stream<ap_axiu<vector_factor, 0, 0, 0>>& arg0_axis_out,
//         hls::stream<ap_axiu<vector_factor, 0, 0, 0>>& arg1_axis_in)
// {
//     for (unsigned int i = 0; i < iter; i++)
//     {
//      //   pragma HLS PIPELINE REWIND
//     #ifdef DEBUG_LOG
//         printf("[KERNEL_DEBUG]|%s| Calling loopback. i:%d\n", __func__, i);
//     #endif
//         datamover_outerloop_0_loopback_dataflow_region(num_pkts
// ,
//         arg0_axis_out,
//         arg1_axis_in);
//     } 
// }

// void datamover_outerloop_0(
//         const unsigned short range_start_0,
//         const unsigned short range_end_0,
//         const unsigned short range_start_1,
//         const unsigned short range_end_1,
//         const unsigned short gridSize_0,
//         const unsigned short gridSize_1,
//         const unsigned int outer_itr,
//         const unsigned short batch_size,
//     //u
//         async_mmap<::tapa::vec_t<stencil_type, mem_vector_factor>>& arg0,
//     //u2
//         async_mmap<::tapa::vec_t<stencil_type, mem_vector_factor>>& arg1,
//     //u
//        ::tapa::ostream<::tapa::vec_t<stencil_type, vector_factor>>& arg0_axis_out,
//     //u2
//        ::tapa::istream<::tapa::vec_t<stencil_type, vector_factor>>& arg1_axis_in
//     )
// {
//     ops::hls::AccessRange range;
//     range.start[0] = range_start_0;
//     range.end[0] = range_end_0;
//     range.start[1] = range_start_1;
//     range.end[1] = range_end_1;
//     range.dim = 2;

//     ops::hls::SizeType read_gridSize = { gridSize_0, gridSize_1, 1 };
//     unsigned int loopback_itr = outer_itr - 1 >= 0 ? outer_itr - 1 : 0;

// #ifdef DEBUG_LOG
//     printf("[KERNEL_DEBUG]|%s| starting datamover TOP range:(%d,%d,%d) ---> (%d,%d,%d)\n", __func__,
//             range.start[0], range.start[1], range.start[2], range.end[0], range.end[1], range.end[2]);
//     printf("[KERNEL_DEBUG]|%s| read_gridSize: (%d, %d, %d), \n", __func__,
//             read_gridSize[0], read_gridSize[1], read_gridSize[2]);
// #endif 

//     ops::hls::MemConfig config;
//     ops::hls::genMemConfig<mem_vector_factor, vector_factor, data_width>(read_gridSize, range, config, batch_size);
//     const unsigned int num_beats = config.total_xblocks;
//     const unsigned int num_pkts = num_of_pkts_per_beat * num_beats;
// #ifdef DEBUG_LOG
//     printf("[KERNEL_DEBUG]|%s| REALIZED numbers: num_beats: %d, num_pkts: %d,\n", __func__,
//          num_beats, num_pkts);

//     printf("[KERNEL_DEBUG]|%s| batch_size: %d \n", __func__, batch_size);
// #endif 
//         datamover_outerloop_0_dataflow_region_read(
//                 num_pkts,
//                 config,
//                 arg0,
//                 arg0_axis_out);

//         datamover_outerloop_0_loopback(loopback_itr, num_pkts
// ,
//                 arg0_axis_out,
//                 arg1_axis_in);

//         datamover_outerloop_0_dataflow_region_write(
//                 num_pkts,
//                 config,
//                 arg1,
//                 arg1_axis_in);
// }

#include <stdio.h>
#include <ops_tapa_kernel_support.h>
#include <datamover_outerloop_0.hpp>

// #define DEBUG_LOG

void hybrid_datamover_router(const unsigned int num_trans,
        const unsigned int outerloop_itr,
       ::tapa::istream<::tapa::vec_t<stencil_type, vector_factor>>& arg0_mem_in,
       ::tapa::istream<::tapa::vec_t<stencil_type, vector_factor>>& arg1_axis_in,
       ::tapa::ostream<::tapa::vec_t<stencil_type, vector_factor>>& arg0_axis_out,
       ::tapa::ostream<::tapa::vec_t<stencil_type, vector_factor>>& arg1_mem_out)
{
#ifdef DEBUG_LOG
    printf("[KERNEL_DEBUG]|%s| num_trans: %d, outerloop_itr: %d\n", __func__, num_trans, outerloop_itr);
#endif
    for (unsigned int itr = 0; itr < outerloop_itr+1; itr++) {
        if (itr == 0) {
            read_from_mem: for (unsigned int i = 0; i < num_trans; i++) {
                #pragma HLS PIPELINE II=1
                auto data = arg0_mem_in.read();
                arg0_axis_out.write(data);
#ifdef DEBUG_LOG
                printf("[KERNEL_DEBUG]|%s|read_from_mem| forwarding trans: %d, trans val: (",__func__, i);
                for (int j = 0; j < vector_factor; j++) {
                    printf(" %f,", data[j]);
                }
                printf(")\n");
#endif
            }
        }

        else if (itr == outerloop_itr) {
            write_to_mem: for (unsigned int i = 0; i < num_trans; i++) {
                #pragma HLS PIPELINE II=1
                auto data = arg1_axis_in.read();
                auto data_regged = register_it(data);
                arg1_mem_out.write(data_regged);
// #ifdef DEBUG_LOG
                printf("[KERNEL_DEBUG]|%s|write_to_mem| forwarding trans: %d, trans val: (",__func__, i);
                for (int j = 0; j < vector_factor; j++) {
                    printf(" %f,", data[j]);
                }
                printf(")\n");
// #endif
            }
        }
        else {
            loopback: for (unsigned int i = 0; i < num_trans; i++) {
                #pragma HLS PIPELINE II=1
                auto data = arg1_axis_in.read();
                arg0_axis_out.write(data);
#ifdef DEBUG_LOG
                printf("[KERNEL_DEBUG]|%s|loopback| forwarding iter: %d, trans: %d, trans val: (",__func__, itr, i);
                for (int j = 0; j < vector_factor; j++) {
                    printf(" %f,", data[j]);
                }
                printf(")\n");
#endif
            }
        }
    }
}   

void task_mem2stream(
   ::tapa::async_mmap<::tapa::vec_t<stencil_type, mem_vector_factor>>& arg0,
   ::tapa::ostream<::tapa::vec_t<stencil_type, mem_vector_factor>>& strm_out,
    const unsigned int num_beats) 
{
    ops::tapa::mem2stream<stencil_type, mem_vector_factor, 2>(arg0, strm_out, num_beats);
}

void task_stepdown(
   ::tapa::istream<::tapa::vec_t<stencil_type, mem_vector_factor>>& strm_in,
   ::tapa::ostream<::tapa::vec_t<stencil_type, vector_factor>>& strm_out,
    const unsigned int num_beats) 
{
    ops::tapa::stream2streamStepdown<stencil_type, mem_vector_factor, vector_factor>(strm_in, strm_out, num_beats);
}

void task_stepup(
   ::tapa::istream<::tapa::vec_t<stencil_type, vector_factor>>& strm_in,
   ::tapa::ostream<::tapa::vec_t<stencil_type, mem_vector_factor>>& strm_out,
    const unsigned int num_beats) 
{
    ops::tapa::stream2streamStepup<stencil_type, vector_factor, mem_vector_factor>(strm_in, strm_out, num_beats);
}

void task_stream2mem(
   ::tapa::async_mmap<::tapa::vec_t<stencil_type, mem_vector_factor>>& arg1,
   ::tapa::istream<::tapa::vec_t<stencil_type, mem_vector_factor>>& strm_in,
    const unsigned int num_beats) 
{
    ops::tapa::stream2mem<stencil_type, mem_vector_factor, 2>(arg1, strm_in, num_beats);
}

void datamover_outerloop_0(
    const unsigned int num_beats,
    const unsigned int num_axis_trans,
    const unsigned int outerloop_itr,
    // u
   ::tapa::mmap<::tapa::vec_t<float,mem_vector_factor>> arg0,
    // u2
   ::tapa::mmap<::tapa::vec_t<float,mem_vector_factor>> arg1,
    // u (External Output)
   ::tapa::ostream<::tapa::vec_t<float,vector_factor>>& arg0_axis_out,
    // u2 (External Input)
   ::tapa::istream<::tapa::vec_t<float,vector_factor>>& arg1_axis_in
) {

    // 1. Declare internal TAPA streams with specified FIFO depths
   ::tapa::stream<::tapa::vec_t<stencil_type, mem_vector_factor>>  arg0_read_mem_strm("arg0_read_mem_strm");
   ::tapa::stream<::tapa::vec_t<stencil_type, vector_factor>> arg0_read_reduced_mem_strm("arg0_read_reduced_mem_strm");
    
   ::tapa::stream<::tapa::vec_t<stencil_type, vector_factor>> arg1_write_reduced_mem_strm("arg1_write_reduced_mem_strm");
   ::tapa::stream<::tapa::vec_t<stencil_type, mem_vector_factor>>  arg1_write_mem_strm("arg1_write_mem_strm");

    // // 2. Build the static hardware topology
    // void hybrid_datamover_router(const unsigned int num_trans,
    //     const unsigned int loopback_itr,
    //    ::tapa::istream<::tapa::vec_t<stencil_type, vector_factor>>& arg0_mem_in,
    //    ::tapa::istream<::tapa::vec_t<stencil_type, vector_factor>>& arg1_axis_in,
    //    ::tapa::ostream<::tapa::vec_t<stencil_type, vector_factor>>& arg0_axis_out,
    //    ::tapa::ostream<::tapa::vec_t<stencil_type, vector_factor>>& arg1_mem_out)
   ::tapa::task()
        // Read Pipeline
        .invoke(task_mem2stream, arg0, arg0_read_mem_strm, num_beats)
        .invoke(task_stepdown, arg0_read_mem_strm, arg0_read_reduced_mem_strm, num_beats)
        .invoke(hybrid_datamover_router, num_axis_trans, outerloop_itr,
                arg0_read_reduced_mem_strm, arg1_axis_in, arg0_axis_out, arg1_write_reduced_mem_strm)
        .invoke(task_stepup, arg1_write_reduced_mem_strm, arg1_write_mem_strm, num_beats)
        .invoke(task_stream2mem, arg1, arg1_write_mem_strm, num_beats);
}