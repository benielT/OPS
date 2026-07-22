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

template <typename T, unsigned short MEM_VEC_FACTOR, unsigned int IN_ITR = 2>
void unified_mem_read_write(
    ::tapa::async_mmap<::tapa::vec_t<T, MEM_VEC_FACTOR>>& mem_in,
    ::tapa::async_mmap<::tapa::vec_t<T, MEM_VEC_FACTOR>>& mem_out,
    ::tapa::ostream<::tapa::vec_t<T, MEM_VEC_FACTOR>>& stream_out,
    ::tapa::istream<::tapa::vec_t<T, MEM_VEC_FACTOR>>& stream_in, 
    const unsigned int num_beats)
{
    #pragma HLS INLINE off

    for (unsigned int k_wr_req = 0, k_wr_resp = 0, k_rd_req = 0, k_rd_resp = 0;
            k_rd_resp < num_beats || k_wr_resp < num_beats;) {
        #pragma HLS PIPELINE II=IN_ITR

        // Process Memory Reads -> Output Stream
        if (k_rd_req < num_beats && mem_in.read_addr.try_write(k_rd_req)) {
            k_rd_req++;
        }
        if (k_rd_resp < num_beats && !mem_in.read_data.empty() && !stream_out.full()) {
            // Using the template parameters T and MEM_VEC_FACTOR
            ::tapa::vec_t<T, MEM_VEC_FACTOR> temp = mem_in.read_data.read(nullptr);
            stream_out.write(temp);
            k_rd_resp++;
        }

        // Process Input Stream -> Memory Writes
        if (k_wr_req < num_beats && !mem_out.write_addr.full() && !mem_out.write_data.full() && !stream_in.empty()) {
            mem_out.write_addr.write(k_wr_req);
            mem_out.write_data.write(stream_in.read());
            k_wr_req++;
        }
        if (!mem_out.write_resp.empty()) {
            k_wr_resp += (unsigned int)(mem_out.write_resp.read()) + 1;
        }
    }
}

/**
 * @brief stream2streamStepdown Converts from one tapa-stream to another with a smaller size with
 *          blocking API. 
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
void stream2streamStepdown_blocking(::tapa::istream<::tapa::vec_t<T, STREAM1_VEC_FACTOR>>& strm_in,
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
 * @brief stream2streamStepdown Converts from one tapa-stream to another with a smaller size (Non-blocking).
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
    unsigned int write_cnt = 0;
    bool data_valid = false;
    
    while (write_cnt < num_small_pkts)
    {
        #pragma HLS PIPELINE II=1

        // Attempt to read if we need new data
        if (!data_valid)
        {
            if (strm_in.try_read(shift_reg))
            {
                shift_reg_raw = ::tapa::bit_cast<ap_uint<STREAM1_DATA_WIDTH>>(shift_reg);
                data_valid = true;

#ifdef DEBUG_LOG_PRINT
#ifndef __SYNTHESIS__
                printf("   |HLS DEBUG_LOG||%s| receiving pkt: %d, val=(", __func__, write_cnt / FACTOR);
                for (unsigned i = 0; i < STREAM1_VEC_FACTOR; i++) {
                    printf("%f,", shift_reg[i]);
                }
                printf(")\n");
#endif
#endif
            }
        }

        // Attempt to write if we have valid data in the shift register
        if (data_valid)
        {
            output_reg_raw = shift_reg_raw; // Stores least significant slice
            ::tapa::vec_t<T, STREAM2_VEC_FACTOR> tmp2 = ::tapa::bit_cast<::tapa::vec_t<T, STREAM2_VEC_FACTOR>>(output_reg_raw); 
            
            if (strm_out.try_write(tmp2))
            {
#ifdef DEBUG_LOG_PRINT
#ifndef __SYNTHESIS__
                printf("   |HLS DEBUG_LOG||%s| writing pkt: %d, val=(", __func__, write_cnt);
                for (unsigned k = 0; k < STREAM2_VEC_FACTOR; k++) {
                    printf("%f,", tmp2[k]);
                }
                printf(")\n");
#endif
#endif
                shift_reg_raw >>= STREAM2_DATA_WIDTH;
                write_cnt++;

                if (n == FACTOR - 1) {
                    n = 0;
                    data_valid = false; // Trigger new read next cycle
                } else {
                    n++;
                }
            }
        }
    }

#ifdef DEBUG_LOG_PRINT
    printf("|HLS DEBUG_LOG|%s| exiting.\n"
            , __func__);
#endif
}

/**
 * @brief stream2streamStepup_blocking Converts from a narrow TAPA stream to a wider TAPA stream (bloking)
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
void stream2streamStepup_blocking(::tapa::istream<::tapa::vec_t<T, STREAM1_VEC_FACTOR>>& strm_in,
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
 * @brief stream2streamStepup Converts from a narrow TAPA stream to a wider TAPA stream (Non-blocking).
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
    unsigned int read_cnt = 0;
    bool pending_write = false;
    
    while (read_cnt < total_small_pkts || pending_write)
    {
        #pragma HLS PIPELINE II=1

        if (pending_write)
        {
            // Backpressure recovery: Retrying the blocked write
            if (strm_out.try_write(shift_reg))
            {
                pending_write = false;
            }
        }
        else
        {
            ::tapa::vec_t<T, STREAM1_VEC_FACTOR> tmp1;
            
            // Standard read and accumulate phase
            if (strm_in.try_read(tmp1))
            {
                input_reg_raw = ::tapa::bit_cast<ap_uint<STREAM1_DATA_WIDTH>>(tmp1);

#ifdef DEBUG_LOG_PRINT
#ifndef __SYNTHESIS__
                printf("   |HLS DEBUG_LOG||%s| reading pkt: %d, val=(", __func__, read_cnt);
                for (unsigned k = 0; k < STREAM1_VEC_FACTOR; k++) {
                    printf("%f,", tmp1[k]);
                }
                printf(")\n");
#endif
#endif
                
                shift_reg_raw >>= STREAM1_DATA_WIDTH;
                shift_reg_raw |= (ap_uint<STREAM2_DATA_WIDTH>(input_reg_raw) << (STREAM2_DATA_WIDTH - STREAM1_DATA_WIDTH));
                read_cnt++;

                if (n == FACTOR - 1)
                {
                    shift_reg = ::tapa::bit_cast<::tapa::vec_t<T, STREAM2_VEC_FACTOR>>(shift_reg_raw);

#ifdef DEBUG_LOG_PRINT
#ifndef __SYNTHESIS__
                    printf("   |HLS DEBUG_LOG||%s| attempting write pkt: %d, val=(", __func__, (read_cnt-1) / FACTOR);
                    for (unsigned k = 0; k < STREAM2_VEC_FACTOR; k++) {
                        printf("%f,", shift_reg[k]);
                    }
                    printf(")\n");
#endif
#endif
                    if (!strm_out.try_write(shift_reg)) {
                        pending_write = true; // Downstream is full, stall and retry next cycle
                    }
                    n = 0;
                }
                else
                {
                    n++;
                }
            }
        }
    }

#ifdef DEBUG_LOG_PRINT
#ifndef __SYNTHESIS__
    printf("|HLS DEBUG_LOG|%s| exiting.\n", __func__);
#endif
#endif
}

/**
 * @brief   stream_terminate reads from a TAPA istream and intentionally discards the data with blocking API.
 * This acts as a sink to consume stream transactions and prevent upstream deadlocks.
 
 * @tparam T : C++ base element type of the vectorized interfaces
 * @tparam VEC_FACTOR : Number of vectorized elements of the TAPA stream port
 * @tparam IN_ITR: II configuration of the read loop (default 1 for maximum throughput)
 *
 * @param strm_in : Input TAPA istream
 * @param num_trans : Number of stream transactions to read and discard
 */
template <typename T, unsigned int VEC_FACTOR, unsigned int IN_ITR=1>
void terminate_blocking(::tapa::istream<::tapa::vec_t<T, VEC_FACTOR>>& strm_in,
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

/**
 * @brief stream_terminate reads from a TAPA istream and intentionally discards the data (Non-blocking).
 * 
 * @tparam T : C++ base element type of the vectorized interfaces
 * @tparam STREAM1_VEC_FACTOR : NUmber of the vector element of the input stream (narrow)
 * @tparam STREAM2_VEC_FACTOR : NUmber of the vector element of the output stream (wide)
 *
 * @param strm_in : Input TAPA istream
 * @param strm_out : Output TAPA ostream
 * @param num_big_pkts : Number of pkts of the wider stream
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

    unsigned int read_cnt = 0;

    while (read_cnt < num_trans)
    {
        #pragma HLS PIPELINE II=IN_ITR
        
        ::tapa::vec_t<T, VEC_FACTOR> tmp;
        
        if (strm_in.try_read(tmp))
        {
#ifdef DEBUG_LOG_PRINT
#ifndef __SYNTHESIS__
            printf("|HLS DEBUG_LOG| %s | discarding index: %d, val=(\n", __func__, read_cnt);
            for (unsigned k = 0; k < VEC_FACTOR; k++) {
                printf("%f,", static_cast<double>(tmp[k]));
            }
            printf(")\n");
#endif
#endif
            read_cnt++;
        }
    }

#ifdef DEBUG_LOG_PRINT
#ifndef __SYNTHESIS__
    printf("|HLS DEBUG_LOG|%s| exiting.\n", __func__);
#endif
#endif
}

/**
 * @brief axis_to_strm reads from an AXI-Stream (represented as a TAPA istream) and writes to a TAPA ostream (Non-blocking).
 *
 * @tparam T : C++ base element type of the vectorized interfaces
 * @tparam VEC_FACTOR : Number of vectorized elements of the TAPA stream ports
 * @tparam IN_ITR : II configuration of the pipeline loop (default 1 for maximum throughput)
 *
 * @param outer_itr : Number of outer iterations
 * @param total_itr : Number of inner iterations per block
 * @param bsize : Block size multiplier for the outer iterations
 * @param axis_in : Input TAPA istream (acting as AXI-Stream)
 * @param hls_out : Output TAPA ostream
 */
template <typename T, unsigned int VEC_FACTOR, unsigned int IN_ITR=1>
void axis_to_strm(const unsigned int outer_itr, const unsigned int total_itr, const unsigned short bsize,
        ::tapa::istream<::tapa::vec_t<T, VEC_FACTOR>>& axis_in, 
        ::tapa::ostream<::tapa::vec_t<T, VEC_FACTOR>>& hls_out) {

    const unsigned int total_outer_itr = outer_itr * bsize;
    const unsigned int total_transactions = total_outer_itr * total_itr;

#ifdef DEBUG_LOG
    printf("[KERNEL_DEBUG]|%s| Starting axis_to_strm. outer_iter: %d, bsize: %d, total_outer_itr: %d, total_itr: %d, total_transactions: %d\n",
            __func__, outer_itr, bsize, total_outer_itr, total_itr, total_transactions);
#endif

    unsigned int trans_cnt = 0;
    bool data_valid = false;
    ::tapa::vec_t<T, VEC_FACTOR> buffer;

    while (trans_cnt < total_transactions) {
        #pragma HLS PIPELINE II=IN_ITR

        // Attempt to read if we don't have valid data
        if (!data_valid) {
            if (axis_in.try_read(buffer)) {
                data_valid = true;
#ifdef DEBUG_LOG
                printf("[KERNEL_DEBUG]|%s| Read trans_id: %d, in_trans val: (", __func__, trans_cnt);
                for (int j = 0; j < VEC_FACTOR; j++) {
                    // Cast to double for generic printing to avoid format string warnings
                    printf(" %f,", static_cast<double>(buffer[j])); 
                }
                printf(")\n");
#endif  
            }
        }

        // Attempt to write if we have valid data
        if (data_valid) {
            if (hls_out.try_write(buffer)) {
                data_valid = false;
                trans_cnt++;
            }
        }
    }
}

/**
 * @brief strm_to_axis reads from a TAPA istream and writes to an AXI-Stream (represented as a TAPA ostream) (Non-blocking).
 *
 * @tparam T : C++ base element type of the vectorized interfaces
 * @tparam VEC_FACTOR : Number of vectorized elements of the TAPA stream ports
 * @tparam IN_ITR : II configuration of the pipeline loop (default 1 for maximum throughput)
 *
 * @param outer_itr : Number of outer iterations
 * @param total_itr : Number of inner iterations per block
 * @param bsize : Block size multiplier for the outer iterations
 * @param hls_in : Input TAPA istream
 * @param axis_out : Output TAPA ostream (acting as AXI-Stream)
 */
template <typename T, unsigned int VEC_FACTOR, unsigned int IN_ITR=1>
void strm_to_axis(const unsigned int outer_itr, const unsigned int total_itr, const unsigned short bsize, 
        ::tapa::istream<::tapa::vec_t<T, VEC_FACTOR>>& hls_in,
        ::tapa::ostream<::tapa::vec_t<T, VEC_FACTOR>>& axis_out) {
    
    const unsigned int total_outer_itr = outer_itr * bsize;
    const unsigned int total_transactions = total_outer_itr * total_itr;

#ifdef DEBUG_LOG
    printf("[KERNEL_DEBUG]|%s| Starting strm_to_axis. outer_iter: %d, bsize: %d, total_outer_itr: %d, total_itr: %d, total_transactions: %d\n",
            __func__, outer_itr, bsize, total_outer_itr, total_itr, total_transactions);
#endif

    unsigned int trans_cnt = 0;
    bool data_valid = false;
    ::tapa::vec_t<T, VEC_FACTOR> buffer;

    while (trans_cnt < total_transactions) {
        #pragma HLS PIPELINE II=IN_ITR

        // Attempt to read if we don't have valid data
        if (!data_valid) {
            if (hls_in.try_read(buffer)) {
                data_valid = true;
            }
        }

        // Attempt to write if we have valid data
        if (data_valid) {
            if (axis_out.try_write(buffer)) {
#ifdef DEBUG_LOG
                printf("[KERNEL_DEBUG]|%s| write trans_id: %d, trans val: (", __func__, trans_cnt);
                for (int j = 0; j < VEC_FACTOR; j++) {
                    printf(" %f,", static_cast<double>(buffer[j]));
                }
                printf(")\n");
#endif
                data_valid = false;
                trans_cnt++;
            }
        }
    }    
}

/**
 * @brief hybrid_datamover_router routes data between memory streams and inter-PE streams (Non-blocking).
 * It handles the initial memory fetch, intermediate stream loopbacks, and final memory writeback seamlessly.
 *
 * @tparam T : C++ base element type of the vectorized interfaces
 * @tparam VEC_FACTOR : Number of vectorized elements of the TAPA stream ports
 * @tparam IN_ITR : II configuration of the pipeline loop (default 1 for maximum throughput)
 *
 * @param mem_in : Input TAPA istream from memory
 * @param axis_in : Input TAPA istream from upstream PE (loopback)
 * @param axis_out : Output TAPA ostream to downstream PE (loopback)
 * @param mem_out : Output TAPA ostream to memory
 * @param num_trans : Number of stream transactions per outer loop iteration
 * @param outerloop_itr : Number of intermediate loopback iterations (total passes = outerloop_itr + 1)
 */
template <typename T, unsigned int VEC_FACTOR, unsigned int IN_ITR=1>
void hybrid_datamover_router(
        ::tapa::istream<::tapa::vec_t<T, VEC_FACTOR>>& mem_in,
        ::tapa::istream<::tapa::vec_t<T, VEC_FACTOR>>& axis_in,
        ::tapa::ostream<::tapa::vec_t<T, VEC_FACTOR>>& axis_out,
        ::tapa::ostream<::tapa::vec_t<T, VEC_FACTOR>>& mem_out,
        const unsigned int num_trans,
        const unsigned int outerloop_itr
#ifdef DEBUG_LOG
        , const char* debg_str = ""
#endif
        )
{
#ifdef DEBUG_LOG
    printf("[KERNEL_DEBUG]|%s|%s| Starting. num_trans: %d, outerloop_itr: %d\n", 
            __func__, debg_str, num_trans, outerloop_itr);
#endif

    unsigned int outer_cnt = 0;
    unsigned int trans_cnt = 0;
    bool data_valid = false;
    ::tapa::vec_t<T, VEC_FACTOR> buffer;

    // Total outer loops = outerloop_itr + 1
    while (outer_cnt <= outerloop_itr) {
        #pragma HLS PIPELINE II=IN_ITR

        // 1. Read Phase (Fetch new data if we don't have any pending)
        if (!data_valid) {
            if (outer_cnt == 0) {
                // Initial pass: Fetch from memory
                if (mem_in.try_read(buffer)) {
                    data_valid = true;
#ifdef DEBUG_LOG
                    printf("[KERNEL_DEBUG]|%s|%s|read_from_mem| forwarding trans: %d, trans val: (",
                            __func__, debg_str, trans_cnt);
                    for (int j = 0; j < VEC_FACTOR; j++) {
                        printf(" %f,", static_cast<double>(buffer[j]));
                    }
                    printf(")\n");
#endif
                }
            } else {
                // Subsequent passes: Read from axis_in (loopback)
                if (axis_in.try_read(buffer)) {
                    data_valid = true;
#ifdef DEBUG_LOG
                    printf("[KERNEL_DEBUG]|%s|%s|loopback| forwarding iter: %d, trans: %d, trans val: (",
                            __func__, debg_str, outer_cnt, trans_cnt);
                    for (int j = 0; j < VEC_FACTOR; j++) {
                        printf(" %f,", static_cast<double>(buffer[j]));
                    }
                    printf(")\n");
#endif
                }
            }
        }

        // 2. Write Phase (Attempt to dispatch valid data downstream)
        if (data_valid) {
            bool write_success = false;

            if (outer_cnt == outerloop_itr) {
                // Final pass: Write back to memory
                if (mem_out.try_write(buffer)) {
                    write_success = true;
#ifdef DEBUG_LOG
                    printf("[KERNEL_DEBUG]|%s|%s|write_to_mem| forwarding trans: %d, trans val: (",
                            __func__, debg_str, trans_cnt);
                    for (int j = 0; j < VEC_FACTOR; j++) {
                        printf(" %f,", static_cast<double>(buffer[j]));
                    }
                    printf(")\n");
#endif
                }
            } else {
                // Initial & Intermediate passes: Forward to axis_out (loopback)
                if (axis_out.try_write(buffer)) {
                    write_success = true;
                }
            }

            // 3. State Update (Advance counters on successful write)
            if (write_success) {
                data_valid = false;
                trans_cnt++;

                if (trans_cnt == num_trans) {
                    trans_cnt = 0;
                    outer_cnt++;
                }
            }
        }
    }
}

}
}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */