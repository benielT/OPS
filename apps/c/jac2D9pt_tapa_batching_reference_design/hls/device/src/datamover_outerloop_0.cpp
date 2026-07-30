
#include <stdio.h>
#include <ops_tapa_kernel_support.h>
#include <datamover_outerloop_0.hpp>

// #define LOOPBACK_DESIGN
// #define DEBUG_LOG

#ifdef LOOPBACK_DESIGN

void task_mem2stream(
#ifdef ASYN_MOVER
    ::tapa::async_mmap<::tapa::vec_t<stencil_type, mem_vector_factor>>& arg0,
#else
    ::tapa::mmap<::tapa::vec_t<stencil_type, mem_vector_factor>>& arg0,
#endif
    ::tapa::ostream<::tapa::vec_t<stencil_type, mem_vector_factor>>& strm_out,
    const unsigned int num_beats) 
{
#ifdef ASYN_MOVER
    ops::tapa::mem2stream<stencil_type, mem_vector_factor, 2>(arg0, strm_out, num_beats);
#else
    ops::tapa::mem2stream_blocking<stencil_type, mem_vector_factor, 1>(arg0, strm_out, num_beats);
#endif
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
#ifdef ASYN_MOVER
    ::tapa::async_mmap<::tapa::vec_t<stencil_type, mem_vector_factor>> arg1,
#else 
    ::tapa::async_mmap<::tapa::vec_t<stencil_type, mem_vector_factor>> arg1,
#endif
    ::tapa::istream<::tapa::vec_t<stencil_type, mem_vector_factor>>& strm_in,
    const unsigned int num_beats) 
{
#ifdef ASYN_MOVER
    ops::tapa::stream2mem<stencil_type, mem_vector_factor, 2>(arg1, strm_in, num_beats);
#else
    ops::tapa::stream2mem_blocking<stencil_type, mem_vector_factor, 1>(arg1, strm_in, num_beats);
#endif
}


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
#ifdef DEBUG_LOG
                printf("[KERNEL_DEBUG]|%s|write_to_mem| forwarding trans: %d, trans val: (",__func__, i);
                for (int j = 0; j < vector_factor; j++) {
                    printf(" %f,", data[j]);
                }
                printf(")\n");
#endif
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
#else

void task_stepdown(
   ::tapa::istream<::tapa::vec_t<stencil_type, mem_vector_factor>>& strm_in,
   ::tapa::ostream<::tapa::vec_t<stencil_type, vector_factor>>& strm_out,
    const unsigned int num_beats, 
    const unsigned int outerloop_itr) 
{
    for (unsigned int itr = 0; itr < outerloop_itr; itr++)
        ops::tapa::stream2streamStepdown<stencil_type, mem_vector_factor, vector_factor>(strm_in, strm_out, num_beats);
}

void task_stepup(
   ::tapa::istream<::tapa::vec_t<stencil_type, vector_factor>>& strm_in,
   ::tapa::ostream<::tapa::vec_t<stencil_type, mem_vector_factor>>& strm_out,
    const unsigned int num_beats,
    const unsigned int outerloop_itr) 
{
    for (unsigned int itr = 0; itr < outerloop_itr; itr++)
        ops::tapa::stream2streamStepup<stencil_type, vector_factor, mem_vector_factor>(strm_in, strm_out, num_beats);
}


void task_pingpong_mem2stream(
#ifdef ASYN_MOVER
    ::tapa::async_mmap<::tapa::vec_t<float, mem_vector_factor>> arg0,
    ::tapa::async_mmap<::tapa::vec_t<float, mem_vector_factor>> arg1,
#else
    ::tapa::mmap<::tapa::vec_t<float, mem_vector_factor>> arg0,
    ::tapa::mmap<::tapa::vec_t<float, mem_vector_factor>> arg1,
#endif
    ::tapa::ostream<::tapa::vec_t<stencil_type, mem_vector_factor>>& strm_out,
    ::tapa::istream<bool>& sync_in,
    const unsigned int num_beats,
    const unsigned int outerloop_itr) 
{
    auto outerloop_itr_by_2 = outerloop_itr >> 1;
    for (unsigned int itr = 0; itr < outerloop_itr_by_2; itr++) {
#ifdef ASYN_MOVER
        ops::tapa::mem2stream<stencil_type, mem_vector_factor, 2>(arg0, strm_out, num_beats);
        sync_in.read();
        ops::tapa::mem2stream<stencil_type, mem_vector_factor, 2>(arg1, strm_out, num_beats);
        sync_in.read();
#else
        ops::tapa::mem2stream_blocking<stencil_type, mem_vector_factor, 1>(arg0, strm_out, num_beats);
        sync_in.read();
        ops::tapa::mem2stream_blocking<stencil_type, mem_vector_factor, 1>(arg1, strm_out, num_beats);
        sync_in.read();
#endif
    }
}

void task_pingpong_stream2mem(
#ifdef ASYN_MOVER
    ::tapa::async_mmap<::tapa::vec_t<float, mem_vector_factor>> arg0,
    ::tapa::async_mmap<::tapa::vec_t<float, mem_vector_factor>> arg1,
#else
    ::tapa::mmap<::tapa::vec_t<float, mem_vector_factor>> arg0,
    ::tapa::mmap<::tapa::vec_t<float, mem_vector_factor>> arg1,
#endif
    ::tapa::istream<::tapa::vec_t<stencil_type, mem_vector_factor>>& strm_in,
    ::tapa::ostream<bool>& sync_out,
    const unsigned int num_beats,
    const unsigned int outerloop_itr) 
{
    auto outerloop_itr_by_2 = outerloop_itr >> 1;
    for (unsigned int itr = 0; itr < outerloop_itr_by_2; itr++) {
#ifdef ASYN_MOVER
        ops::tapa::stream2mem<stencil_type, mem_vector_factor, 2>(arg1, strm_in, num_beats);
        sync_out.write(true);
        ops::tapa::stream2mem<stencil_type, mem_vector_factor, 2>(arg0, strm_in, num_beats);
        sync_out.write(true);
#else
        ops::tapa::stream2mem_blocking<stencil_type, mem_vector_factor, 1>(arg1, strm_in, num_beats);
        sync_out.write(true);
        ops::tapa::stream2mem_blocking<stencil_type, mem_vector_factor, 1>(arg0, strm_in, num_beats);
        sync_out.write(true);
#endif
    }

}
#endif



void datamover_outerloop_0(
    const unsigned int num_beats,
    const unsigned int num_axis_trans,
    const unsigned int outerloop_itr,
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
) {

    #ifdef LOOPBACK_DESIGN
    // 1. Declare internal TAPA streams with specified FIFO depths
   ::tapa::stream<::tapa::vec_t<stencil_type, mem_vector_factor>,2,4096>  arg0_read_mem_strm("arg0_read_mem_strm");
   ::tapa::stream<::tapa::vec_t<stencil_type, vector_factor>,2,4096> arg0_read_reduced_mem_strm("arg0_read_reduced_mem_strm");
    
   ::tapa::stream<::tapa::vec_t<stencil_type, vector_factor>,2,4096> arg1_write_reduced_mem_strm("arg1_write_reduced_mem_strm");
   ::tapa::stream<::tapa::vec_t<stencil_type, mem_vector_factor>,2,4096>  arg1_write_mem_strm("arg1_write_mem_strm");

   ::tapa::task()
        // Read Pipeline
        .invoke(task_mem2stream, arg0, arg0_read_mem_strm, num_beats)
        .invoke(task_stepdown, arg0_read_mem_strm, arg0_read_reduced_mem_strm, num_beats)
        .invoke(hybrid_datamover_router, num_axis_trans, outerloop_itr,
                arg0_read_reduced_mem_strm, arg1_axis_in, arg0_axis_out, arg1_write_reduced_mem_strm)
        .invoke(task_stepup, arg1_write_reduced_mem_strm, arg1_write_mem_strm, num_beats)
        .invoke(task_stream2mem, arg1, arg1_write_mem_strm, num_beats);
    #else
        ::tapa::stream<::tapa::vec_t<stencil_type, mem_vector_factor>, 4, 4096> arg0_read_mem_strm("arg0_read_mem_strm");
        ::tapa::stream<::tapa::vec_t<stencil_type, mem_vector_factor>, 4, 4096> arg1_write_mem_strm("arg1_write_mem_strm");
        ::tapa::stream<bool> sync_strm("sync_strm");
        
        ::tapa::task()
            .invoke(task_pingpong_mem2stream, arg0, arg1, arg0_read_mem_strm, sync_strm, num_beats, outerloop_itr)
            .invoke(task_stepdown, arg0_read_mem_strm, arg0_axis_out, num_beats, outerloop_itr) 
            .invoke(task_stepup, arg1_axis_in, arg1_write_mem_strm, num_beats, outerloop_itr)
            .invoke(task_pingpong_stream2mem, arg0, arg1, arg1_write_mem_strm, sync_strm, num_beats, outerloop_itr);
    #endif    
}