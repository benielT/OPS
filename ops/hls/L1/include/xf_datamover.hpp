#pragma once

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/*
 * Copyright (C) 2019-2022, Xilinx, Inc.
 * Copyright (C) 2022-2023, Advanced Micro Devices, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


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

/** @file
  * @brief Vitis Library based datamover adoptation 
  * @author Beniel Thileepan
  * @details Implements of the templatised data mover functions with 
  * protcol conversion and data width conversions.
  */

#include <ap_int.h>
#include <ap_axi_sdata.h>
#include <hls_stream.h>
#include <hls_burst_maxi.h>

// #define DEBUG_LOG

namespace xf {
namespace data_mover {

namespace details {

/**
 * A bit more about manual burst / write design
 * Both take "request params + data" and operate on burst_maxi
 * Both will act like "request at first, close request later"
 * This means there'll be requests on the fly, and they need buffer.
 *
 * One important limit to be noticed:
 * burst maxi port will first "cut" request into sub request, if data request ran across 4KB border.
 * Outstanding are upperbound of such sub request, in pragma for burst maxi port
 * To avoid deadlock caused by sending too much request, request from HLS side need to be less than outstanding.
 *
 * In actual design, burst maxi width is no bigger than 64 Bytes (512 bits).
 * If BURSTLEN <= 64, then total size of one quest is less than 64 * 64 = 4096 Byte
 * So actual sub request is at most twice as much as request from HLS side.
 * So we could at least create (outstanding / 2) HLS requests is safe to be handled.
 *
 * But BURSTLEN should not be too small, for best bandwidth, at least 16.
 */

/** 
 * Read from stream and create burst write request to DDR/HBM
 *
 * @tparam WDATA width of MAXI port.
 * @tparam LATENCY latency of MAXI port.
 * @tparam OUTSTANDING read outstanding of MAXI port, should be less than 512.
 * @tparam BURSTLEN read burst length of MAXI port, should be less than 64.
 * @tparam II to adjust II of the read request frequency based on downstream
 *
 * @param r_offset, stream to get offset for burst write
 * @param r_burst, stream ot get length for burst write, should be no bigger BURSTLEN
 * @param e_r, end of write request
 * @param w_data, stream to read data for writing
 * @param data MAXI port for writing
 */

template <int WDATA, int LATENCY, int OUTSTANDING, int BURSTLEN, int II=2>
void manualBurstWrite( // input
    hls::stream<ap_uint<64> >& r_offset,
    hls::stream<ap_uint<10> >& r_burst,
    hls::stream<bool>& e_r,
    hls::stream<ap_uint<WDATA>>& w_data,
    // output
    hls::burst_maxi<ap_uint<WDATA> >& data) {
    ap_uint<LATENCY> check = 0;
    ap_uint<10> req_left = OUTSTANDING / 2;
    ap_uint<10> write_left = 0;
    ap_uint<10> burst_record[1024];
    ap_uint<10> rec_head = 0;
    ap_uint<10> rec_tail = 0;
    bool last = e_r.read();

// #ifdef DEBUG_LOG
    printf("|HLS DEBUG_LOG|%s| Starting manualBurstWrite, req_left=%u\n", __func__, (unsigned int)req_left);
// #endif

ACC_BURST_REQ_LOOP:
    // TODO: accumulate so many request at first might cause deadlock, to be fixed.
    while (!last && rec_tail != OUTSTANDING / 2) {
#pragma HLS pipeline II = II
        ap_uint<64> tmp_offset = r_offset.read();
        ap_uint<64> tmp_burst = r_burst.read();
        last = e_r.read();
        burst_record[rec_tail++] = tmp_burst;
        data.write_request(tmp_offset, tmp_burst);
        req_left--;
// #ifdef DEBUG_LOG
        printf("|HLS DEBUG_LOG|%s| ACC_BURST_REQ: offset=%llu, burst=%llu, rec_tail=%u\n", __func__, (unsigned long long)tmp_offset, (unsigned long long)tmp_burst, (unsigned int)rec_tail);
// #endif
    }
    if (rec_tail != 0) {
        write_left = burst_record[rec_head++];
// #ifdef DEBUG_LOG
        printf("|HLS DEBUG_LOG|%s| Initial write_left=%u\n", __func__, (unsigned int)write_left);
// #endif
    }

BURST_WRITE_LOOP:
    while (!last || write_left != 0 || rec_head != rec_tail) {
#pragma HLS pipeline II = II
        bool check_l = check[LATENCY - 1];
        check <<= 1;

        if (write_left != 0) { // load data and write if possible
            ap_uint<WDATA> tmp_data = w_data.read();
            data.write(tmp_data); // haha, too many data

            if (--write_left == 0) { // if all data of current write request has been sent
                check[0] = 1;
// #ifdef DEBUG_LOG
                printf("|HLS DEBUG_LOG|%s| Write burst complete, rec_head=%u, rec_tail=%u\n", __func__, (unsigned int)rec_head, (unsigned int)rec_tail);
// #endif
                if (rec_head != rec_tail) {
                    write_left = burst_record[rec_head++];
// #ifdef DEBUG_LOG
                    printf("|HLS DEBUG_LOG|%s| Next write_left=%u\n", __func__, (unsigned int)write_left);
// #endif
                }
            }
        }

        if (check_l) { // if a write request has become old enough
            data.write_response();
// #ifdef DEBUG_LOG
            printf("|HLS DEBUG_LOG|%s| Write response issued\n", __func__);
// #endif

            if (!last) {
                ap_uint<64> tmp_offset = r_offset.read();
                ap_uint<64> tmp_burst = r_burst.read();
                last = e_r.read();
                burst_record[rec_tail++] = tmp_burst;
                data.write_request(tmp_offset, tmp_burst);
// #ifdef DEBUG_LOG
                printf("|HLS DEBUG_LOG|%s| New request: offset=%llu, burst=%llu, rec_tail=%u\n", __func__, (unsigned long long)tmp_offset, (unsigned long long)tmp_burst, (unsigned int)rec_tail);
// #endif
            } else {
                req_left++;
// #ifdef DEBUG_LOG
                printf("|HLS DEBUG_LOG|%s| Last flag set, req_left=%u\n", __func__, (unsigned int)req_left);
// #endif
            }
        }
    }

// #ifdef DEBUG_LOG
    printf("|HLS DEBUG_LOG|%s| Entering final response loop, check=%u\n", __func__, (unsigned int)check);
// #endif

    while (check != 0) {
        bool check_l = check[LATENCY - 1];
        check <<= 1;
        if (check_l) {
            data.write_response();
            req_left--;
// #ifdef DEBUG_LOG
            printf("|HLS DEBUG_LOG|%s| Final write response, remaining check=%u\n", __func__, (unsigned int)check);
// #endif
        }
    }

// #ifdef DEBUG_LOG
    printf("|HLS DEBUG_LOG|%s| manualBurstWrite complete\n", __func__);
// #endif
}

/**
 * Create burst read request to DDR/HBM and write to stream
 *
 * @tparam WDATA width of MAXI port.
 * @tparam LATENCY latency of MAXI port.
 * @tparam OUTSTANDING read outstanding of MAXI port, should be less than 512.
 * @tparam BURSTLEN read burst length of MAXI port, should be less than 64.
 * @tparam II to adjust II of the read request frequency based on downstream
 * 
 * @param data MAXI port for reading
 * @param r_offset, stream to get offset for burst read
 * @param r_burst, stream ot get length for burst read, should be no bigger BURSTLEN
 * @param e_r, end of read request
 * @param w_data, stream to write read result
 */

template <int WDATA, int LATENCY, int OUTSTANDING, int BURSTLEN, int II=2>
void manualBurstRead( // input
    hls::burst_maxi<ap_uint<WDATA> >& data,
    hls::stream<ap_uint<64> >& r_offset,
    hls::stream<ap_uint<10> >& r_burst,
    hls::stream<bool>& e_r,
    // output
    hls::stream<ap_uint<WDATA> >& w_data) {
    ap_uint<LATENCY> check = 0;             // delay check
    ap_uint<10> req_left = OUTSTANDING / 2; // how many more request could be issued, "/2" is to avoid hang
    ap_uint<10> req_ready = 0;              // how many request should be ready, according to delay check
    ap_uint<10> burst_record[1024];         // burstlen record
    ap_uint<10> rec_head = 0;               // record head
    ap_uint<10> rec_tail = 0;               // record tail
    ap_uint<10> read_left = 0;              // read left in 1 burst
    bool last = e_r.read();

BURST_READ_LOOP:
    while (!last || req_ready != 0 || check != 0) {
#pragma HLS pipeline II = II
        //#pragma HLS pipeline II = 1

        bool check_l = check[LATENCY - 1];
        check <<= 1;

        if (req_left != 0 && !last) { // if read outstanding is not exhausted, issue more request
            ap_uint<64> tmp_offset = r_offset.read();
            ap_uint<10> tmp_burst = r_burst.read();
            last = e_r.read();

            data.read_request(tmp_offset, tmp_burst);
            check[0] = 1;
            req_left--;
            burst_record[rec_tail++] = tmp_burst;
        }

        if (req_ready != 0 || read_left != 0) { // if there's mature req
            if (read_left == 0) {
                read_left = burst_record[rec_head++] - 1;
                req_ready--;
                req_left++;
            } else {
                read_left--;
            }

            ap_uint<WDATA> tmp_data;
            tmp_data = data.read();
            w_data.write(tmp_data);
        }

        if (check_l) { // if a new request has become old enough
            req_ready++;
        }
    }

    while (read_left != 0) {
#pragma HLS pipeline II = II
        ap_uint<WDATA> tmp_data;
        tmp_data = data.read();
        read_left--;
        w_data.write(tmp_data);
    }
}

/**
 * @brief Parses configuration and generates offset and burst values for skewed 3D tiled reads
 *
 * @details Processes a command packet containing offset and stride/tile_size pairs for each dimension (X, Y, Z).
 *          Generates appropriate r_offset and r_burst values for memory access patterns.
 *          
 *          Burst calculation logic:
 *          - If innermost loop (X-dimension) stride is 1: r_burst = min(tile_x, BURSTLEN)
 *          - Otherwise: r_burst = 1
 *          
 *          Command packet format (160 bits):
 *          - bits [63:0]     - offset (64 bit)
 *          - bits [79:64]    - stride_x (16 bit)
 *          - bits [95:80]    - size_x (16 bit)
 *          - bits [111:96]   - stride_y (16 bit)
 *          - bits [127:112]  - size_y (16 bit)
 *          - bits [143:128]  - stride_z (16 bit)
 *          - bits [159:144]  - size_z (16 bit)
 *
 * @tparam BURSTLEN The maximum burst size in bytes. Must be less than or equal to the AXI interface
 *                  max_read_burst_length (for reads) or max_write_burst_length (for writes).
 *
 * @param[in]  command   Stream containing 160-bit command packets with offset and tile configuration
 * @param[out] r_offset  Stream of 64-bit memory offsets for each access
 * @param[out] r_burst   Stream of 10-bit burst length values for each access
 * @param[out] e_r       Stream of end-of-transaction flags (false for data, true for end marker)
 *
 * @note Designed for HLS synthesis with pipeline capability (II=1)
 */

template <unsigned short BURSTLEN>
void configParser3DTiled(::hls::stream<ap_uint<160>>& command,
        ::hls::stream<ap_uint<64>>& r_offset,
        ::hls::stream<ap_uint<10>>& r_burst,
        hls::stream<bool>& e_r
)
{
    ap_uint<160> cmd_pkt = command.read();
    ap_uint<64> offset = cmd_pkt.range(63,0);
    ap_uint<16> stride_x = cmd_pkt.range(79,64);
    ap_uint<16> size_x = cmd_pkt.range(95,80);
    ap_uint<16> stride_y = cmd_pkt.range(111,96);
    ap_uint<16> size_y = cmd_pkt.range(127,112);
    ap_uint<16> stride_z = cmd_pkt.range(143,128);
    ap_uint<16> size_z = cmd_pkt.range(159,144);


    ap_uint<10> x_inc = stride_x == 1 ? BURSTLEN : 1;

    for (ap_uint<16> z = 0; z < size_z; z++)
    {
        ap_uint<64> s3 = offset + z * stride_z;
        for (ap_uint<16> y = 0; y < size_y; y++)
        {
            ap_uint<64> s2 = s3 + y * stride_y;
            for (ap_uint<16> x = 0; x < size_x; x += x_inc)
            {
                #pragma HLS pipeline II = 1
                ap_uint<64> s1 = s2 + x * stride_x;

                ap_uint<16> x_cond = x + (ap_uint<16>)BURSTLEN;
                ap_uint<10> last_burst = (ap_uint<10>)(size_x - x);
                ap_uint<10> burst;
                if (stride_x == 1)
                    burst = x_cond <= size_x ? (ap_uint<10>)BURSTLEN : last_burst;
                else
                    burst = 1;
                r_offset.write(s1);
                r_burst.write(burst);
                e_r.write(false);
            #ifdef DEBUG_LOG
                printf("|HLS DEBUG_LOG|%s| s1:%llu, burst:%u\n", __func__, (unsigned long long)s1, (unsigned int)burst);
            #endif
            }
        }
    }
#ifdef DEBUG_LOG
  printf("|HLS DEBUG_LOG|%s| Sending last\n", __func__);
#endif
    e_r.write(true);
}


/**
 * @brief Generates a 3D command packet and writes it to a command stream.
 * 
 * This function constructs a 160-bit command word by packing 3D data movement parameters
 * (offset and stride/size information for X, Y, and Z dimensions) into specific bit ranges,
 * then writes the command to the output stream.
 * 
 * @param offset       The base memory offset (64-bit value). Packed into bits [63:0].
 * @param stride_x     The stride in the X dimension (16-bit value). Packed into bits [79:64].
 * @param size_x       The size in the X dimension (16-bit value). Packed into bits [95:80].
 * @param stride_y     The stride in the Y dimension (16-bit value). Packed into bits [111:96].
 * @param size_y       The size in the Y dimension (16-bit value). Packed into bits [127:112].
 * @param stride_z     The stride in the Z dimension (16-bit value). Packed into bits [143:128].
 * @param size_z       The size in the Z dimension (16-bit value). Packed into bits [159:144].
 * @param s_command    Reference to the output HLS stream that receives the 160-bit command packet.
 * 
 * @note When DEBUG_LOG is defined, prints offset and dimension information to stdout.
 * 
 * @return void
 */
static void commandGen3D(const ap_uint<64> offset, const ap_uint<16> stride_x, const ap_uint<16> size_x,
                const ap_uint<16> stride_y, const ap_uint<16> size_y,
                const ap_uint<16> stride_z, const ap_uint<16> size_z,
                ::hls::stream<ap_uint<160>>& s_command)
{
    ap_uint<160> command;
    command.range(63,0) = offset;
    command.range(79,64) = stride_x;
    command.range(95,80) = size_x;
    command.range(111,96) = stride_y;
    command.range(127,112) = size_y;
    command.range(143,128) = stride_z;
    command.range(159,144) = size_z;
    s_command << command;
// #ifdef DEBUG_LOG
    printf("|HLS DEBUG_LOG|%s| offset:%llu, stride_x:%u, size_x:%u, stride_y:%u, size_y:%u, stride_z:%u, size_z:%u\n", __func__, (unsigned long long)offset, (unsigned int)stride_x, (unsigned int)size_x, (unsigned int)stride_y, (unsigned int)size_y, (unsigned int)stride_z, (unsigned int)size_z);
// #endif
}

/** 
 * @brief Performs a 3D tiled read operation from memory with burst capabilities
 *
 * This function reads data from memory in a tiled 3D pattern using burst transfers.
 * It parses read commands and coordinates the actual burst read operations through
 * a dataflow pipeline.
 *
 * @tparam MEM_DATA_WIDTH Width of the memory data bus in bits
 * @tparam LATENCY Expected latency of memory read operations in cycles
 * @tparam OUTSTANDING Maximum number of outstanding read requests allowed
 * @tparam BURSTLEN Maximum burst length for read operations
 *
 * @param data Burst AXI interface for reading from main memory
 * @param s_command Input stream containing 160-bit command packets specifying
 *                   read offsets, burst lengths, and operation flags
 * @param s_data Output stream containing read data 
 *               (MEM_DATA_WIDTH bits per transaction)
 *
 * @note Uses HLS DATAFLOW pragma to enable pipelining between the command parser
 *       and burst read modules for improved throughput
 * @note Internal streams are sized according to OUTSTANDING parameter to maintain
 *       command-to-data correspondence
 */
template <int MEM_DATA_WIDTH, int LATENCY, int OUTSTANDING, int BURSTLEN>
void read3DTiled(
    // input
    ::hls::burst_maxi<ap_uint<MEM_DATA_WIDTH> >& data,
    ::hls::stream<ap_uint<160>>& s_command,
    // ouput
    ::hls::stream<ap_uint<MEM_DATA_WIDTH> >& s_data) {
#pragma HLS DATAFLOW
    hls::stream<ap_uint<64> > r_offset("r_offset");
#pragma HLS stream variable = r_offset depth = OUTSTANDING
    hls::stream<ap_uint<10> > r_burst("r_burst");
#pragma HLS stream variable = r_burst depth = OUTSTANDING
    hls::stream<bool> e_r("e_r");
#pragma HLS stream variable = e_r depth = OUTSTANDING

    configParser3DTiled<BURSTLEN>(s_command, r_offset, r_burst, e_r);
    manualBurstRead<MEM_DATA_WIDTH, LATENCY, OUTSTANDING, BURSTLEN>(data, r_offset, r_burst, e_r, s_data);
}

/**
 * @brief Writes data to memory in a 3D tiled pattern using burst transactions.
 * 
 * This template function orchestrates a dataflow pipeline that parses tiling commands
 * and performs manual burst writes to memory. It decouples command parsing from the
 * actual burst write operations using internal streams.
 * 
 * @tparam MEM_DATA_WIDTH   Width of memory data bus in bits
 * @tparam LATENCY          Expected write latency in cycles for performance tuning
 * @tparam OUTSTANDING      Maximum number of outstanding burst transactions allowed
 * @tparam BURSTLEN         Maximum burst length in beats
 * 
 * @param s_command         Input stream of 160-bit command words containing tiling parameters
 * @param s_data            Input stream of MEM_DATA_WIDTH-bit data words to be written
 * @param data              Output burst_maxi interface to memory for writing data
 * 
 * @note Uses HLS DATAFLOW pragma for pipeline parallelization
 * @note Internal streams are sized to OUTSTANDING depth to match transaction buffer capacity
 * @note The command stream format is parsed by configParser to extract offset, burst length, and end flags
 */
template<int MEM_DATA_WIDTH, int LATENCY, int OUTSTANDING, int BURSTLEN>
void write3DTiled(
    // input
    ::hls::stream<ap_uint<MEM_DATA_WIDTH>>& s_data,
    ::hls::stream<ap_uint<160>>& s_command,
    // output
    ::hls::burst_maxi<ap_uint<MEM_DATA_WIDTH>>& data
)
{
// #ifdef DEBUG_LOG
  printf("|HLS DEBUG_LOG|%s| starting\n", __func__);
// #endif
#pragma HLS DATAFLOW
    hls::stream<ap_uint<64> > r_offset("r_offset");
#pragma HLS stream variable = r_offset depth = OUTSTANDING
    hls::stream<ap_uint<10> > r_burst("r_burst");
#pragma HLS stream variable = r_burst depth = OUTSTANDING
    hls::stream<bool> e_r("e_r");
#pragma HLS stream variable = e_r depth = OUTSTANDING
    configParser3DTiled<BURSTLEN>(s_command, r_offset, r_burst, e_r);
    manualBurstWrite<MEM_DATA_WIDTH, LATENCY, OUTSTANDING, BURSTLEN>(r_offset, r_burst, e_r, s_data, data);
}

} //namespace details


/**
 * @brief Reads a 3D tiled region from memory and streams it to an output stream.
 *
 * This function performs a 3D tiled memory-to-stream transfer operation. It processes
 * a 3D grid of data, dividing it into smaller tiles and reading each tile sequentially
 * from burst-enabled memory into an output stream.
 *
 * @tparam MEM_DATA_WIDTH Width of memory data in bits
 * @tparam LATENCY Memory read latency in cycles
 * @tparam BURSTLEN Maximum burst length for memory reads (default: 32)
 * @tparam OUTSTANDING_READ Number of outstanding read requests allowed (default: 32)
 * @tparam IN_ITR Input iteration count for optimization (default: 2)
 *
 * @param[in] mem_in Burst-enabled MAXI interface for reading from memory
 * @param[out] strm_out Output stream to write the read data
 * @param[in] config MemConfigTile configuration object containing:
 *   - start_offset: Starting byte offset in memory
 *   - total_size_bytes: Total size of data to read in bytes
 *   - start_z, end_z: Z-dimension range
 *   - grid_xblocks: Grid size in X dimension
 *   - grid_size_y: Grid size in Y dimension
 *   - tile_count_x, tile_count_y: Number of tiles in X and Y dimensions
 *   - tile_size_x, tile_size_y: Size of each tile in X and Y
 *   - last_tile_size_x, last_tile_size_y: Size of last tile if different
 *   - effective_tile_size_x, effective_tile_size_y: Effective tile stride
 *
 * @note This function is designed for HLS synthesis and uses pragmas for stream optimization.
 * @note Debug logging can be enabled by defining DEBUG_LOG macro.
 *
 * @see commandGen3D
 * @see read3DTiled
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short LATENCY, unsigned short BURSTLEN=32, unsigned short OUTSTANDING_READ=32, unsigned short IN_ITR=2>
static void tiledMem2stream3D(::hls::burst_maxi<ap_uint<MEM_DATA_WIDTH> >& mem_in, hls::stream<ap_uint<MEM_DATA_WIDTH> >& strm_out, const ops::hls::MemConfigTile& config)
{
    #ifdef DEBUG_LOG
        printf("|HLS DEBUG_LOG|%s| reading tile. tile_start:%d, tile_size:%d\n", __func__, config.start_offset, config.total_size_bytes);
    #endif

    const unsigned short z_diff = config.end_z - config.start_z;
    const unsigned short stride_y = config.grid_xblocks;
    const unsigned short stride_z = config.grid_xblocks * config.grid_size_y;

    ::hls::stream<ap_uint<160>> command("command");
    #pragma HLS stream variable = command depth = OUTSTANDING_READ

    for (unsigned short tile_y = 0; tile_y < config.tile_count_y; tile_y++)
    {
        const unsigned short tile_size_y = tile_y == (config.tile_count_y -1) ? config.last_tile_size_y : config.tile_size_y;
        const ap_uint<64> tile_y_offset = tile_y * config.effective_tile_size_y * config.grid_xblocks;

        for (unsigned short tile_x = 0; tile_x < config.tile_count_x; tile_x++)
        {
#ifdef DEBUG_LOG
            printf("|HLS DEBUG_LOG|%s| Tile details tile_x:%d, tile_y:%d\n", __func__, tile_x, tile_y);
#endif
            const unsigned short tile_size_x = tile_x == (config.tile_count_x -1) ? config.last_tile_size_x : config.tile_size_x;
            const ap_uint<64> tile_x_offset = tile_x * config.effective_tile_size_x;
            const ap_uint<64> offset = config.start_offset + tile_x_offset + tile_y_offset;

            details::commandGen3D(offset, 1, (ap_uint<16>)tile_size_x, (ap_uint<16>)stride_y, (ap_uint<16>)tile_size_y, (ap_uint<16>)stride_z, (ap_uint<16>)z_diff, command);
            details::read3DTiled<MEM_DATA_WIDTH, LATENCY, OUTSTANDING_READ, BURSTLEN>(mem_in, command, strm_out);
        }
    }
}

/**
 * @brief Writes a 3D tiled data stream to memory with burst access.
 * 
 * This function transfers data from an input stream to external memory, organizing the write
 * operations according to a 3D tiling configuration. It generates memory access commands for
 * each tile and performs the actual data transfer with support for burst writes and pipelining.
 * 
 * @tparam MEM_DATA_WIDTH   Width of the memory data bus in bits
 * @tparam LATENCY          Latency parameter for memory operations
 * @tparam BURSTLEN         Maximum burst length for memory transactions (default: 32)
 * @tparam OUTSTANDING_READ Maximum number of outstanding read/write commands (default: 32)
 * @tparam IN_ITR           Input iteration parameter for pipelining (default: 2)
 * 
 * @param[out] mem_out      Burst MAXI interface to external memory
 * @param[in]  strm_in      Input stream containing the data to be written
 * @param[in] config MemConfigTile configuration object containing:
 *   - start_offset: Starting byte offset in memory
 *   - total_size_bytes: Total size of data to read in bytes
 *   - start_z, end_z: Z-dimension range
 *   - grid_xblocks: Grid size in X dimension
 *   - grid_size_y: Grid size in Y dimension
 *   - tile_count_x, tile_count_y: Number of tiles in X and Y dimensions
 *   - tile_size_x, tile_size_y: Size of each tile in X and Y
 *   - last_tile_size_x, last_tile_size_y: Size of last tile if different
 *   - effective_tile_size_x, effective_tile_size_y: Effective tile stride
 * 
 * @note The function performs 3D tiled memory writes by iterating through Y and X tile
 *       dimensions while handling Z-dimension strides internally through the command generator.
 * @note DEBUG_LOG can be enabled at compile time to print tile write information.
 * 
 * @see commandGen3D
 * @see write3DTiled
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short LATENCY, unsigned short BURSTLEN=32, unsigned short OUTSTANDING_READ=32, unsigned short IN_ITR=2>
static void tiledStream2mem3D(::hls::burst_maxi<ap_uint<MEM_DATA_WIDTH> >& mem_out, hls::stream<ap_uint<MEM_DATA_WIDTH> >& strm_in, const ops::hls::MemConfigTile& config)
{
// #ifdef DEBUG_LOG
    printf("|HLS DEBUG_LOG|%s| writing tile. tile_start:%d, tile_size:%d\n", __func__, config.start_offset, config.total_size_bytes);
// #endif

    const unsigned short z_diff = config.end_z - config.start_z;
    const unsigned short stride_y = config.grid_xblocks;
    const unsigned short stride_z = config.grid_xblocks * config.grid_size_y;

    ::hls::stream<ap_uint<160>> command("command");
    #pragma HLS stream variable = command depth = OUTSTANDING_READ

    for (unsigned short tile_y = 0; tile_y < config.tile_count_y; tile_y++)
    {
        const unsigned short tile_size_y = tile_y == (config.tile_count_y -1) ? config.last_tile_size_y : config.tile_size_y;
        const ap_uint<64> tile_y_offset = tile_y * config.effective_tile_size_y * config.grid_xblocks;

        for (unsigned short tile_x = 0; tile_x < config.tile_count_x; tile_x++)
        {
            const unsigned short tile_size_x = tile_x == (config.tile_count_x -1) ? config.last_tile_size_x : config.tile_size_x;
            const ap_uint<64> tile_x_offset = tile_x * config.effective_tile_size_x;
            const ap_uint<64> offset = config.start_offset + tile_x_offset + tile_y_offset;

            details::commandGen3D(offset, 1, (ap_uint<16>)tile_size_x, (ap_uint<16>)stride_y, (ap_uint<16>)tile_size_y, (ap_uint<16>)stride_z, (ap_uint<16>)z_diff, command);
            details::write3DTiled<MEM_DATA_WIDTH, LATENCY, OUTSTANDING_READ, BURSTLEN>(strm_in, command, mem_out);
        }
    }
}

} //namespace data_mover
} //namespace xf

#endif /* DOXYGEN_SHOULD_SKIP_THIS */