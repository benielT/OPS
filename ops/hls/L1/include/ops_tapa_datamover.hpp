#pragma once

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/** @file
  * @brief TAPA HLS specific L1 data mover functions.
  * @author Beniel Thileepan
  * @details Implements of the templatised data mover functions with 
  * protcol conversion and data width conversions.
  */

#include <ap_int.h>
#include <tapa.h>
// #include <ap_axi_sdata.h>
// #include <hls_stream.h>
#include "../../common/include/ops_hls_defs.hpp"
#include "../../common/include/ops_hls_utils.hpp"
#include <math.h>
#include <stdio.h>
#include <tuple>
// #define DEBUG_LOG

#ifndef __SYNTHESIS__
#include <thread>
#endif

#ifdef DEBUG_LOG
	#ifndef __SYNTHESIS__
		#ifndef DEBUG_LOG_SIZE_OF
			#define DEBUG_LOG_SIZE_OF 4
		#endif
		#define DEBUG_LOG_PRINT
	#endif
#endif

namespace ops {
namespace tapa {

/**
 * @brief   mem2stream reads from global memory to a TAPA ostream using async_mmap.
 * This guarantees maximum AXI throughput without manual burst calculations.
 * 
 * @tparam T : standard C++ type 
 * @tparam MEM_VEC_FACTOR : Number of elements mem_in and strm_out
 * @tparam IN_ITR: II of the mem read
 * @param mem_in : Asynchronous TAPA memory mapped interface
 * @param strm_out : Output TAPA ostream
 * @param num_beats : Number of beats (data words) to read
 */
template <typename T, unsigned int MEM_VEC_FACTOR, unsigned int IN_ITR=2>
void mem2stream(::tapa::async_mmap<::tapa::vec_t<T, MEM_VEC_FACTOR>>& mem_in,
                ::tapa::ostream<::tapa::vec_t<T, MEM_VEC_FACTOR>>& strm_out,
                const unsigned int num_beats)
{
#ifndef __SYNTHESIS__
    static_assert(MEM_VEC_FACTOR >= min_mem_data_width/sizeof(T) && MEM_VEC_FACTOR <= max_mem_data_width/sizeof(T),
            "MEM_VEC_FACTOR failed limit check");
#endif

#ifdef DEBUG_LOG_PRINT
    printf("|HLS DEBUG_LOG||%s| Starting reading async_memmap. num_beats: %d\n"
                , __func__, num_beats);
#endif

    for (unsigned int i_req = 0, i_resp = 0; i_resp < num_beats; )
    {
    #pragma HLS PIPELINE II=IN_ITR
        
        bool made_progress = false;

        // Async read request
        if ((i_req < num_beats) && !mem_in.read_addr.full()) {
            mem_in.read_addr.try_write(i_req);
            i_req++;
            made_progress = true;
        }
    
        // As soon as available read and write to stream
        if (!strm_out.full() && !mem_in.read_data.empty()) {
            ::tapa::vec_t<T, MEM_VEC_FACTOR> tmp;
            mem_in.read_data.try_read(tmp);
            strm_out.try_write(tmp); 
            i_resp++; 
            made_progress = true;
            
#ifdef DEBUG_LOG_PRINT
            printf("|HLS DEBUG_LOG||%s| ===============================================================\n", __func__);
            printf("|HLS DEBUG_LOG||%s| reading index: %d, val=(\n", __func__, i_resp - 1);
            for (unsigned k = 0; k < MEM_VEC_FACTOR; k++){
                printf("     %f,\n", tmp[k]); 
            }
            printf("|HLS DEBUG_LOG||%s| ===============================================================\n\n", __func__);
#endif
        }

#ifndef __SYNTHESIS__
        // Prevent coroutine starvation in software emulation
        if (!made_progress) {
            std::this_thread::yield(); 
        }
#endif
    }
}


/**
 * @brief   mem2stream_blocking reads from global memory to a TAPA ostream using async_mmap.
 * This guarantees maximum AXI throughput without manual burst calculations.
 * 
 * @tparam T : standard C++ type 
 * @tparam MEM_VEC_FACTOR : Number of elements mem_in and strm_out
 * @tparam IN_ITR: II of the mem read
 * * @param mem_in : Synchronous TAPA memory mapped interface
 * @param strm_out : Output TAPA ostream
 * @param num_beats : Number of beats (data words) to read
 */
template <typename T, unsigned int MEM_VEC_FACTOR, unsigned int IN_ITR=2>
void mem2stream_blocking(::tapa::mmap<::tapa::vec_t<T, MEM_VEC_FACTOR>>& mem_in,
                ::tapa::ostream<::tapa::vec_t<T, MEM_VEC_FACTOR>>& strm_out,
                const unsigned int num_beats)
{
#ifndef __SYNTHESIS__
    static_assert(MEM_VEC_FACTOR >= min_mem_data_width/sizeof(T) && MEM_VEC_FACTOR <= max_mem_data_width/sizeof(T),
            "MEM_VEC_FACTOR failed limit check");
#endif

#ifdef DEBUG_LOG_PRINT
    printf("|HLS DEBUG_LOG||%s| Starting reading memmap. num_beats: %d\n"
                , __func__, num_beats);
#endif

    for (unsigned int i_req = 0; i_req < num_beats; i_req++)
    {
    #pragma HLS PIPELINE II=IN_ITR
        ::tapa::vec_t<T, MEM_VEC_FACTOR> tmp = mem_in[i_req];
        strm_out.write(tmp); 

#ifdef DEBUG_LOG_PRINT
        printf("|HLS DEBUG_LOG||%s| ===============================================================\n", __func__);
        printf("|HLS DEBUG_LOG||%s| reading index: %d, val=(\n", __func__, i_req);

        // for (unsigned k = 0; k < MEM_VEC_FACTOR/(DEBUG_LOG_SIZE_OF * 8); k++){
        //     DataConv conv;
        //     conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
        //     printf("     %f,\n", conv.f); 
        // }
        for (unsigned k = 0; k < MEM_VEC_FACTOR; k++){
            printf("     %f,\n", tmp[k]); 
        }
        printf("|HLS DEBUG_LOG||%s| ===============================================================\n\n", __func__);
#endif
    }
    // strm_out.close(); //EoT token this is not necessary as we know exact transactions
}

/**
 * @brief   stream2mem reads from a TAPA istream and writes to global memory using async_mmap.
 * This guarantees maximum AXI throughput without manual burst calculations.
 * 
 * @tparam T : C++ base element type of the vectorized interfaces
 * @tparam MEM_VEC_FACTOR : Number of vectorized elements of AXI4 port and the TAPA stream port
 * @tparam IN_ITR: II configuration of mem write
 *
 * @param mem_out : Asynchronous TAPA memory mapped output interface
 * @param strm_in : Input TAPA istream
 * @param num_beats : Number of beats (data words) to write
 */
template <typename T, unsigned int MEM_VEC_FACTOR, unsigned int IN_ITR=2>
void stream2mem(::tapa::async_mmap<::tapa::vec_t<T, MEM_VEC_FACTOR>>& mem_out,
                ::tapa::istream<::tapa::vec_t<T, MEM_VEC_FACTOR>>& strm_in,
                const unsigned int num_beats)
{
#ifndef __SYNTHESIS__
    static_assert(MEM_VEC_FACTOR >= min_mem_data_width/sizeof(T) && MEM_VEC_FACTOR <= max_mem_data_width/sizeof(T),
            "MEM_VEC_FACTOR failed limit check");
#endif

    for (unsigned int i_req = 0, i_resp = 0; i_resp < num_beats; )
    {
        #pragma HLS PIPELINE II=IN_ITR
        bool made_progress = false;

        // 1. Bundle address issue and data write together
        // Only proceed if both the source stream has data and the AXI channels can accept it
        if ((i_req < num_beats) && !strm_in.empty() && 
            !mem_out.write_addr.full() && !mem_out.write_data.full()) {
            
            mem_out.write_addr.try_write(i_req);
            
            ::tapa::vec_t<T, MEM_VEC_FACTOR> tmp;
            strm_in.try_read(tmp);
            mem_out.write_data.try_write(tmp);
            
            ++i_req;
            made_progress = true;
        }

        // 2. Consume AXI write responses to track completed transactions
        // TAPA encodes the burst length minus one in the response token
        uint8_t n_resp;
        if (mem_out.write_resp.try_read(n_resp)) {
            i_resp += int(n_resp) + 1;
            made_progress = true;
        }

#ifndef __SYNTHESIS__
        // Prevent coroutine starvation in software emulation
        if (!made_progress) {
            std::this_thread::yield(); 
        }
#endif
    }
}

/**
 * @brief   stream2mem_blocking reads from a TAPA istream and writes to global memory using tapa mmap.
 * This guarantees maximum AXI throughput without manual burst calculations.
 * 
 * @tparam T : C++ base element type of the vectorized interfaces
 * @tparam MEM_VEC_FACTOR : Number of vectorized elements of AXI4 port and the TAPA stream port
 * @tparam IN_ITR: II configuration of mem write
 *
 * @param mem_out : Synchronous TAPA memory mapped output interface
 * @param strm_in : Input TAPA istream
 * @param num_beats : Number of beats (data words) to write
 */
template <typename T, unsigned int MEM_VEC_FACTOR, unsigned int IN_ITR=2>
void stream2mem_blocking(::tapa::mmap<::tapa::vec_t<T, MEM_VEC_FACTOR>>& mem_out,
                ::tapa::istream<::tapa::vec_t<T, MEM_VEC_FACTOR>>& strm_in,
                const unsigned int num_beats)
{
#ifndef __SYNTHESIS__
    static_assert(MEM_VEC_FACTOR >= min_mem_data_width/sizeof(T) && MEM_VEC_FACTOR <= max_mem_data_width/sizeof(T),
            "MEM_VEC_FACTOR failed limit check");
#endif

#ifdef DEBUG_LOG_PRINT
#ifndef __SYNTHESIS__
    printf("|HLS DEBUG_LOG| %s | Starting writing memmap. num_beats: %d\n", 
            __func__, num_beats);
    printf("====================================================================================\n");
#endif
#endif

    // i_req tracks addresses issued
    // i_data tracks data payload sent
    // i_resp tracks write acknowledgments received
    for (unsigned int i_req = 0; i_req < num_beats; i_req++)
    {
        #pragma HLS PIPELINE II=IN_ITR

        auto tmp = strm_in.read();
        mem_out[i_req] = tmp;

#ifdef DEBUG_LOG_PRINT
#ifndef __SYNTHESIS__
            printf("|HLS DEBUG_LOG| %s | writing index: %d, val=(\n", __func__, i_req);

            // for (unsigned k = 0; k < MEM_VEC_FACTOR/(DEBUG_LOG_SIZE_OF * 8); k++)
            // {
            //     DataConv conv;
            //     conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
            //     printf("%f,", conv.f);
            // }
            for (unsigned k = 0; k < MEM_VEC_FACTOR; k++)
            {
                printf("%f,", tmp[k]);
            }
            printf(")\n");
#endif
#endif
    }

#ifdef DEBUG_LOG_PRINT
#ifndef __SYNTHESIS__
    printf("|HLS DEBUG_LOG|%s| exiting.\n", __func__);
#endif
#endif
}

/**
 * @brief stream2streamStepdown Converts from one tapa-stream to another with a smaller size.
 *
 * @tparam T : C++ base element type of the vectorized interfaces
 * @tparam STREAM1_VEC_FACTOR : Number of vectorized elements of the input stream
 * @tparam STREAM2_VEC_FACTOR : Number of vectorized elements of the output stream
 *
 * @param strm_in : Input TAPA istream (wider)
 * @param strm_out : Output TAPA ostream (narrower)
 * @param num_big_pkts : Number of pkts of the wider stream
 */
template <typename T, unsigned int STREAM1_VEC_FACTOR, unsigned int STREAM2_VEC_FACTOR>
void stream2streamStepdown(::tapa::istream<::tapa::vec_t<T, STREAM1_VEC_FACTOR>>& strm_in,
                ::tapa::ostream<::tapa::vec_t<T, STREAM2_VEC_FACTOR>>& strm_out,
                const unsigned int num_big_pkts)
{
#ifndef __SYNTHESIS__
    static_assert(STREAM1_VEC_FACTOR > STREAM2_VEC_FACTOR,
            "STREAM1_VEC_FACTOR has to be bigger than STREAM2_VEC_FACTOR");
    static_assert(STREAM1_VEC_FACTOR % STREAM2_VEC_FACTOR == 0, 
            "STREAM1_VEC_FACTOR has to be fully divisible by STREAM2_VEC_FACTOR");
#endif

    constexpr unsigned short STREAM1_DATA_WIDTH = STREAM1_VEC_FACTOR * ::tapa::widthof<T>();
    constexpr unsigned short STREAM2_DATA_WIDTH = STREAM2_VEC_FACTOR * ::tapa::widthof<T>();

    constexpr unsigned short FACTOR = STREAM1_VEC_FACTOR / STREAM2_VEC_FACTOR;
    const unsigned int num_small_pkts = num_big_pkts * FACTOR;

#ifdef DEBUG_LOG_PRINT
#ifndef __SYNTHESIS__
    printf("|HLS DEBUG_LOG| %s | Starting. num_big_pkts: %d, num_small_pkts: %d\n"
            , __func__, num_big_pkts, num_small_pkts);
    printf("====================================================================================\n");
#endif
#endif

    ::tapa::vec_t<T, STREAM1_VEC_FACTOR> shift_reg;
    ap_uint<STREAM1_DATA_WIDTH> shift_reg_raw;
    ap_uint<STREAM2_DATA_WIDTH> output_reg_raw;
    unsigned short n = 0;
    
    for (unsigned int pkt = 0; pkt < num_small_pkts; pkt++)
    {
        #pragma HLS PIPELINE II=1

        if (n == 0)
        {
            shift_reg = strm_in.read();
            shift_reg_raw = ::tapa::bit_cast<ap_uint<STREAM1_DATA_WIDTH>>(shift_reg);

#ifdef DEBUG_LOG_PRINT
#ifndef __SYNTHESIS__
            printf("   |HLS DEBUG_LOG||%s| receiving pkt: %d, val=(", __func__, pkt / FACTOR);

            for (unsigned i = 0; i < STREAM1_VEC_FACTOR; i++)
            {
                printf("%f,", shift_reg[i]);
            }
            printf(")\n");
#endif
#endif
        }

        output_reg_raw = shift_reg_raw; //This will store least significant slice.
        ::tapa::vec_t<T, STREAM2_VEC_FACTOR> tmp2 = ::tapa::bit_cast<::tapa::vec_t<T, STREAM2_VEC_FACTOR>>(output_reg_raw); 
        strm_out.write(tmp2);

#ifdef DEBUG_LOG_PRINT
#ifndef __SYNTHESIS__
        printf("   |HLS DEBUG_LOG||%s| writing pkt: %d, val=(", __func__, pkt);

        for (unsigned k = 0; k < STREAM2_VEC_FACTOR; k++)
        {
            printf("%f,", tmp2[k]);
        }
        printf(")\n");
#endif
#endif

        shift_reg_raw >>= STREAM2_DATA_WIDTH;

        if (n == FACTOR - 1) {
            n = 0;
        } else {
            n++;
        }
    }

#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| exiting.\n"
			, __func__);
#endif
}


/**
 * @brief stream2streamStepup Converts from a narrow TAPA stream to a wider TAPA stream.
 *
 * @tparam T : C++ base element type of the vectorized interfaces
 * @tparam STREAM1_VEC_FACTOR : NUmber of the vector element of the input stream (narrow)
 * @tparam STREAM2_VEC_FACTOR : NUmber of the vector element of the output stream (wide)
 *
 * @param strm_in : Input TAPA istream
 * @param strm_out : Output TAPA ostream
 * @param num_big_pkts : Number of pkts of the wider stream
 */
template <typename T, unsigned int STREAM1_VEC_FACTOR, unsigned int STREAM2_VEC_FACTOR>
void stream2streamStepup(::tapa::istream<::tapa::vec_t<T, STREAM1_VEC_FACTOR>>& strm_in,
                ::tapa::ostream<::tapa::vec_t<T, STREAM2_VEC_FACTOR>>& strm_out,
                const unsigned int num_big_pkts)
{
#ifndef __SYNTHESIS__
    static_assert(STREAM1_VEC_FACTOR < STREAM2_VEC_FACTOR,
            "STREAM1_VEC_FACTOR has to be smaller than STREAM2_VEC_FACTOR");
    static_assert(STREAM2_VEC_FACTOR % STREAM1_VEC_FACTOR == 0, 
            "STREAM2_VEC_FACTOR has to be fully divisible by STREAM1_VEC_FACTOR");
#endif

    constexpr unsigned short FACTOR = STREAM2_VEC_FACTOR / STREAM1_VEC_FACTOR;
    constexpr unsigned short STREAM1_DATA_WIDTH = STREAM1_VEC_FACTOR * ::tapa::widthof<T>();
    constexpr unsigned short STREAM2_DATA_WIDTH = STREAM2_VEC_FACTOR * ::tapa::widthof<T>();

    const unsigned int total_small_pkts = num_big_pkts * FACTOR;

#ifdef DEBUG_LOG_PRINT
#ifndef __SYNTHESIS__
    printf("|HLS DEBUG_LOG| %s | num_big_pkts: %d, total_small_pkts: %d\n"
            , __func__, num_big_pkts, total_small_pkts);
    printf("====================================================================================\n");
#endif
#endif

    ::tapa::vec_t<T, STREAM2_VEC_FACTOR> shift_reg;
    ap_uint<STREAM2_DATA_WIDTH> shift_reg_raw = 0;
    ap_uint<STREAM1_DATA_WIDTH> input_reg_raw;
    unsigned short n = 0;
    
    // Manually flattened into a single loop for guaranteed II=1
    for (unsigned int pkt = 0; pkt < total_small_pkts; pkt++)
    {
        #pragma HLS PIPELINE II=1

        ::tapa::vec_t<T, STREAM1_VEC_FACTOR> tmp1 = strm_in.read();
        input_reg_raw = ::tapa::bit_cast<ap_uint<STREAM1_DATA_WIDTH>>(tmp1);

#ifdef DEBUG_LOG_PRINT
#ifndef __SYNTHESIS__
        printf("   |HLS DEBUG_LOG||%s| reading pkt: %d, val=(", __func__, pkt);

        for (unsigned k = 0; k < STREAM1_VEC_FACTOR; k++)
        {
            printf("%f,", tmp1[k]);
        }
        printf(")\n");
#endif
#endif
        
        // Hardware optimized accumulation: Shift right, then insert new data at the top.
        // This naturally packs the words from LSB to MSB over FACTOR cycles.
        shift_reg_raw >>= STREAM1_DATA_WIDTH;
        shift_reg_raw |= (ap_uint<STREAM2_DATA_WIDTH>(input_reg_raw) << (STREAM2_DATA_WIDTH - STREAM1_DATA_WIDTH));

        if (n == FACTOR - 1)
        {
            shift_reg = ::tapa::bit_cast<::tapa::vec_t<T, STREAM2_VEC_FACTOR>>(shift_reg_raw);

#ifdef DEBUG_LOG_PRINT
#ifndef __SYNTHESIS__
            printf("   |HLS DEBUG_LOG||%s| writing pkt: %d, val=(", __func__, pkt / FACTOR);

            for (unsigned k = 0; k < STREAM2_VEC_FACTOR; k++)
            {
                printf("%f,", shift_reg[k]);
            }
            printf(")\n");
#endif
#endif
            strm_out.write(shift_reg);
            n = 0;
        }
        else
        {
            n++;
        }
    }

#ifdef DEBUG_LOG_PRINT
#ifndef __SYNTHESIS__
    printf("|HLS DEBUG_LOG|%s| exiting.\n", __func__);
#endif
#endif
}

/**
 * @brief   stream_terminate reads from a TAPA istream and intentionally discards the data.
 * This acts as a sink to consume stream transactions and prevent upstream deadlocks.
 * * @tparam T : C++ base element type of the vectorized interfaces
 * @tparam VEC_FACTOR : Number of vectorized elements of the TAPA stream port
 * @tparam IN_ITR: II configuration of the read loop (default 1 for maximum throughput)
 *
 * @param strm_in : Input TAPA istream
 * @param num_trans : Number of stream transactions to read and discard
 */
template <typename T, unsigned int VEC_FACTOR, unsigned int IN_ITR=1>
void terminate(::tapa::istream<::tapa::vec_t<T, VEC_FACTOR>>& strm_in,
                      const unsigned int num_trans)
{
#ifdef DEBUG_LOG_PRINT
#ifndef __SYNTHESIS__
    printf("|HLS DEBUG_LOG| %s | Starting stream termination. num_transaction: %d\n", 
            __func__, num_trans);
    printf("====================================================================================\n");
#endif
#endif

    for (unsigned int i = 0; i < num_trans; i++)
    {
        #pragma HLS PIPELINE II=IN_ITR
        auto tmp = strm_in.read();

#ifdef DEBUG_LOG_PRINT
#ifndef __SYNTHESIS__
        printf("|HLS DEBUG_LOG| %s | discarding index: %d, val=(\n", __func__, i);

        for (unsigned k = 0; k < VEC_FACTOR; k++)
        {
            printf("%f,", static_cast<double>(tmp[k]));
        }
        printf(")\n");
#endif
#endif
    }

#ifdef DEBUG_LOG_PRINT
#ifndef __SYNTHESIS__
    printf("|HLS DEBUG_LOG|%s| exiting.\n", __func__);
#endif
#endif
}

}
}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */