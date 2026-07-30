#pragma once

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/** @file
  * @brief Vitis HLS specific L1 data mover functions.
  * @author Beniel Thileepan
  * @details Implements of the templatised data mover functions with 
  * protcol conversion and data width conversions.
  */

#include <ap_int.h>
#include <ap_axi_sdata.h>
#include <hls_stream.h>
#include "../../common/include/ops_hls_defs.hpp"
#include "../../common/include/ops_hls_utils.hpp"
#include <math.h>
#include <stdio.h>
#include <tuple>
// #define DEBUG_LOG


#ifdef DEBUG_LOG
	// #ifndef __SYNTHESIS__
		#ifndef DEBUG_LOG_SIZE_OF
			#define DEBUG_LOG_SIZE_OF 4
		#endif
		#define DEBUG_LOG_PRINT
	// #endif
#endif


namespace ops {
namespace hls {

#ifdef __SYNTHESIS__
#if defined(__has_include) && __has_include(<hls_print.h>)
    #define VITIS_HAS_SYNTH_PRINT 1
#else
    #define VITIS_HAS_SYNTH_PRINT 0
#endif
#endif 

#if VITIS_HAS_SYNTH_PRINT && defined(__SYNTHESIS__)

// Forward declare Xilinx intrinsics to satisfy TAPA's Clang parser (tapacc)
extern "C" {
    void _ssdm_op_PrintNone(const char*);
    void _ssdm_op_PrintInt(const char*, int);
    void _ssdm_op_PrintDouble(const char*, unsigned long long);
}

// Required because blackbox does not support double
typedef union {
    double d;
    unsigned long long l;
} convert;

void print(const char* fmt) { 
    // FIXME: replace with intrinsic call and first argument index into format table
    _ssdm_op_PrintNone(fmt); 
}
template <typename _TYPE>
void print(const char* fmt, _TYPE v) { 
    // FIXME: replace with intrinsic call and first argument index into format table
    _ssdm_op_PrintInt(fmt, (int) v); 
}
void print(const char* fmt, double v) { 
    convert tmp;
    tmp.d = v;
    // FIXME: replace with intrinsic call and first argument index into format table
    _ssdm_op_PrintDouble(fmt, tmp.l);
}
void print(const char* fmt, float v) { 
    convert tmp;
    tmp.d = v;
    // FIXME: replace with intrinsic call and first argument index into format table
    _ssdm_op_PrintDouble(fmt, tmp.l);
}
#else
inline
void print(const char* fmt) { 
    printf("HLS_PRINT: ");
    printf(fmt); 
}
template <typename _TYPE>
inline
void print(const char* fmt, _TYPE v) { 
    printf("HLS_PRINT: ");
    printf(fmt, v); 
}
#endif

/**
 * @brief 	convMemBeat2axisPkt reads a memory location with index and generate into AXI4-stream. This works
 * with MEM_DATA_WIDTH >= AXIS_DATA_WIDTH.
 * 
 */
template <unsigned int MEM_DATA_WIDTH, unsigned int AXIS_DATA_WIDTH>
static void convMemBeat2axisPkt(ap_uint<MEM_DATA_WIDTH>* mem_in,
					::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_out,
					const unsigned int& size,
					const unsigned int& index)
{	
#ifndef __SYNTHESIS__
	static_assert(MEM_DATA_WIDTH >= AXIS_DATA_WIDTH,
			"MEM_DATA_WIDTH has to be grater that AXIS_DATA_WIDTH");
#endif
	constexpr unsigned int bytes_per_beat = MEM_DATA_WIDTH / 8;
	constexpr unsigned int bytes_per_axis_pkt = AXIS_DATA_WIDTH / 8;
	constexpr unsigned int num_strm_pkts_per_beat = MEM_DATA_WIDTH / AXIS_DATA_WIDTH;

#ifdef DEBUG_LOG_PRINT
	printf("\n|HLS DEBUG_LOG| %s | bytes_per_beat: %d, bytes_per_axis_pkt: %d, num_strm_pkts_per_beat: %d\n"
			, __func__, bytes_per_beat, bytes_per_axis_pkt, num_strm_pkts_per_beat);
	printf("======================================================================================================\n");
#endif
	
	ap_uint<MEM_DATA_WIDTH> tmp;
	tmp = mem_in[index];
	
#ifdef DEBUG_LOG_PRINT
	printf("   |HLS DEBUG_LOG|%s| reading beat index: %d\n", __func__, index);
#endif

	for (unsigned int pkt = 0; pkt < num_strm_pkts_per_beat; pkt++)
	{
	#pragma HLS PIPELINE II=num_strm_pkts_per_beat
	#pragma HLS LOOP_TRIPCOUNT min=min_strm_pkts_per_beat avg=avg_strm_pkts_per_beat max=max_strm_pkts_per_beat
		unsigned int byte_idx = index * bytes_per_beat + pkt * bytes_per_axis_pkt;

		if (byte_idx < size)
		{
			ap_axiu<AXIS_DATA_WIDTH,0,0,0> tmp_pkt;
			tmp_pkt.data = tmp.range((pkt + 1) * AXIS_DATA_WIDTH - 1, pkt * AXIS_DATA_WIDTH);
			strm_out.write(tmp_pkt);

#ifdef DEBUG_LOG_PRINT
			printf("   |HLS DEBUG_LOG|%s| sending axis pkt: %d, val=(",__func__, pkt);

			for (unsigned n = 0; n < bytes_per_axis_pkt/DEBUG_LOG_SIZE_OF; n++)
			{
				DataConv tmp;
				tmp.i = tmp_pkt.data.range((n+1) * DEBUG_LOG_SIZE_OF * 8 - 1, n * DEBUG_LOG_SIZE_OF * 8);
				printf("%f,", tmp.f);
			}
			printf(")\n");
#endif
		}
	}
}

/**
 * @brief 	convAxisPkt2memBeat writes a memory location with index and from AXI4-stream.
 * This works MEM_DATA_WIDTH >= AXIS_DATA_WIDTH
 * 
 */
template <unsigned int MEM_DATA_WIDTH, unsigned int AXIS_DATA_WIDTH>
static void convAxisPkt2memBeat(ap_uint<MEM_DATA_WIDTH>* mem_out,
					::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_in,
					unsigned int& size,
					unsigned int& index)
{	
#ifndef __SYNTHESIS__
	static_assert(MEM_DATA_WIDTH >= AXIS_DATA_WIDTH,
			"MEM_DATA_WIDTH has to be grater that AXIS_DATA_WIDTH");
#endif
	constexpr unsigned int bytes_per_beat = MEM_DATA_WIDTH / 8;
	constexpr unsigned int bytes_per_axis_pkt = AXIS_DATA_WIDTH / 8;
	constexpr unsigned int num_strm_pkts_per_beat = MEM_DATA_WIDTH / AXIS_DATA_WIDTH;

#ifdef DEBUG_LOG_PRINT
	printf("\n|HLS DEBUG_LOG| %s | bytes_per_beat: %d, bytes_per_axis_pkt: %d, num_strm_pkts_per_beat: %d, beat index: %d\n"
			, __func__, bytes_per_beat, bytes_per_axis_pkt, num_strm_pkts_per_beat, index);
	printf("======================================================================================================\n");
#endif

	ap_uint<MEM_DATA_WIDTH> tmp;
	
	for (unsigned int pkt = 0; pkt < num_strm_pkts_per_beat; pkt++)
	{
	#pragma HLS PIPELINE II=1
	#pragma HLS LOOP_TRIPCOUNT min=min_strm_pkts_per_beat avg=avg_strm_pkts_per_beat max=max_strm_pkts_per_beat
		unsigned int byte_idx = index * bytes_per_beat + pkt * bytes_per_axis_pkt;

		if (byte_idx < size)
		{
			ap_axiu<AXIS_DATA_WIDTH,0,0,0> tmp_pkt;
			tmp_pkt = strm_in.read();
			tmp.range((pkt + 1) * AXIS_DATA_WIDTH - 1, pkt * AXIS_DATA_WIDTH) = tmp_pkt.data;

#ifdef DEBUG_LOG_PRINT
			printf("   |HLS DEBUG_LOG||%s| receiving axis pkt: %d, val=(", __func__, pkt);

			for (unsigned n = 0; n < bytes_per_axis_pkt/DEBUG_LOG_SIZE_OF; n++)
			{
				DataConv tmp;
				tmp.i = tmp_pkt.data.range((n+1) * DEBUG_LOG_SIZE_OF * 8 - 1, n * DEBUG_LOG_SIZE_OF * 8);
				printf("%f,", tmp.f);
			}
			printf(")\n");
#endif
		}
	}
	mem_out[index] = tmp;
}

/**
 * @brief 	convAxisPkt2memBeatMaksed writes a memory location with index and 
 * from AXI4-stream with strobe channels.
 *
 * @details This works with DATA_WITDH < AXIS_DATA_WIDTH. This will read from
 * AXI4-stream packet with verifying strobe channel. If the TSTRB of of the first byte of a data
 * point with DATA_WIDH is LOW, it will omit writing it to memory.
 *
 */
template <unsigned int DATA_WIDTH, unsigned int AXIS_DATA_WIDTH>
static void convAxisPkt2memBeatMasked(ap_uint<DATA_WIDTH>* mem_out,
					::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_in,
					unsigned int& size,
					unsigned int& index)
{
#ifndef __SYNTHESIS__
	static_assert(DATA_WIDTH <= AXIS_DATA_WIDTH,
			"MEM_DATA_WIDTH has to be less than equal AXIS_DATA_WIDTH");
#endif

	constexpr unsigned int bytes_per_data = DATA_WIDTH / 8;
	constexpr unsigned int bytes_per_axis_pkt = AXIS_DATA_WIDTH / 8;
	constexpr unsigned int num_data_per_axis_pkt = AXIS_DATA_WIDTH / DATA_WIDTH;
//	constexpr unsigned int shift_val = 1 << bytes_per_data;
	ap_uint<1> cond_write = 0;

#ifdef DEBUG_LOG_PRINT
	printf("\n|HLS DEBUG_LOG|%s| bytes_per_axis_pkt: %d, bytes_per_data: %d, num_data_per_axis_pkt: %d, beat index: %d\n"
			, __func__, bytes_per_axis_pkt, bytes_per_data, num_data_per_axis_pkt, index);
	printf("======================================================================================================\n");
#endif
	ap_axiu<AXIS_DATA_WIDTH,0,0,0> tmp_pkt;
	
	tmp_pkt = strm_in.read();

#ifdef DEBUG_LOG_PRINT
		printf("   |HLS DEBUG_LOG||%s| receiving axis , val=(", __func__);

		for (unsigned n = 0; n < bytes_per_axis_pkt/DEBUG_LOG_SIZE_OF; n++)
		{
			DataConv tmp;
			tmp.i = tmp_pkt.data.range((n+1) * DEBUG_LOG_SIZE_OF * 8 - 1, n * DEBUG_LOG_SIZE_OF * 8);
			printf("%f,", tmp.f);
		}

		printf(") strb=(%x)\n", tmp_pkt.strb);

#endif
//	ap_uint<bytes_per_axis_pkt> strb_pos = 1;

	for (unsigned int idx = 0; idx < num_data_per_axis_pkt; idx++)
	{
#pragma HLS PIPELINE  II=1
#pragma HLS LOOP_TRIPCOUNT avg=avg_num_data_per_axis_pkt

//		ap_uint<bytes_per_axis_pkt> cond_val = tmp_pkt.strb & strb_pos;
		cond_write = tmp_pkt.strb.range(idx * bytes_per_data + 1, idx * bytes_per_data);
		ap_uint<DATA_WIDTH> write_val = tmp_pkt.data.range((idx+1) * DATA_WIDTH - 1, idx * DATA_WIDTH);

#ifdef DEBUG_LOG_PRINT
		DataConv tmp;
		tmp.i = write_val;
		printf("   |HLS DEBUG_LOG||%s| writing to mem index: %d, strb_pos:%d, val: %f, cond: %d\n"
				, __func__, index+idx, idx * bytes_per_data, tmp.f, cond_write);
#endif
		if (cond_write)
		{
			mem_out[index + idx] = write_val;
		}
//		strb_pos *= shift_val;
	}
}

/**
 * @brief 	convAxisPkt2memBeat writes a memory location with index and from AXI4-stream.
 * This works MEM_DATA_WIDTH >= AXIS_DATA_WIDTH
 *
 */
template <unsigned int MEM_DATA_WIDTH, unsigned int STREAM_DATA_WIDTH>
static void convStreamPkt2memBeat(ap_uint<MEM_DATA_WIDTH>* mem_out,
					::hls::stream<ap_uint<STREAM_DATA_WIDTH>>& strm_in,
					unsigned int& size,
					unsigned int& index)
{
#ifndef __SYNTHESIS__
	static_assert(MEM_DATA_WIDTH >= STREAM_DATA_WIDTH,
			"MEM_DATA_WIDTH has to be grater that AXIS_DATA_WIDTH");
#endif
	constexpr unsigned int bytes_per_beat = MEM_DATA_WIDTH / 8;
	constexpr unsigned int bytes_per_stream_pkt = STREAM_DATA_WIDTH / 8;
	constexpr unsigned int num_strm_pkts_per_beat = MEM_DATA_WIDTH / STREAM_DATA_WIDTH;

#ifdef DEBUG_LOG_PRINT
	printf("\n|HLS DEBUG_LOG| %s | bytes_per_beat: %d, bytes_per_stream_pkt: %d, num_strm_pkts_per_beat: %d, beat index: %d\n"
			, __func__, bytes_per_beat, bytes_per_stream_pkt, num_strm_pkts_per_beat, index);
	printf("======================================================================================================\n");
#endif
	ap_uint<MEM_DATA_WIDTH> tmp;

	for (unsigned int pkt = 0; pkt < num_strm_pkts_per_beat; pkt++)
	{
	#pragma HLS PIPELINE II=1
	#pragma HLS LOOP_TRIPCOUNT min=min_strm_pkts_per_beat avg=avg_strm_pkts_per_beat max=max_strm_pkts_per_beat
		unsigned int byte_idx = index * bytes_per_beat + pkt * bytes_per_stream_pkt;

		if (byte_idx < size)
		{
			ap_uint<STREAM_DATA_WIDTH> tmp_pkt;
			tmp_pkt = strm_in.read();
			tmp.range((pkt + 1) * STREAM_DATA_WIDTH - 1, pkt * STREAM_DATA_WIDTH) = tmp_pkt;

#ifdef DEBUG_LOG_PRINT
			printf("   |HLS DEBUG_LOG||%s| receiving axis pkt: %d, val=(", __func__, pkt);

			for (unsigned n = 0; n < bytes_per_stream_pkt/DEBUG_LOG_SIZE_OF; n++)
			{
				DataConv tmp;
				tmp.i = tmp_pkt.range((n+1) * DEBUG_LOG_SIZE_OF * 8 - 1, n * DEBUG_LOG_SIZE_OF * 8);
				printf("%f,", tmp.f);
			}
			printf(")\n");
#endif
		}
	}
	mem_out[index] = tmp;
}

//convStreamPkt2memBeatMasked<DATA_WIDTH,STREAM_DATA_WIDTH>(mem_out, data_in, mask_in, size, index);
/**
 * @brief 	convStreamPkt2memBeatMasked writes a memory location with index and
 * from HLS stream with mask channel.
 *
 * @details This works with DATA_WITDH < STREAM_DATA_WIDTH. This will read from
 * HLS-stream packet with verifying mask channel which is similar to AXIS STRB channel. If the TSTRB
 * of the first byte of a data point with DATA_WIDH is LOW, it will omit writing it to memory.
 *
 */
template <unsigned int DATA_WIDTH, unsigned int STREAM_DATA_WIDTH>
static void convStreamPkt2memBeatMasked(ap_uint<DATA_WIDTH>* mem_out,
					::hls::stream<ap_uint<STREAM_DATA_WIDTH>>& data_in,
					::hls::stream<ap_uint<STREAM_DATA_WIDTH/8>>& mask_in,
					unsigned int& size,
					unsigned int& index)
{
#ifndef __SYNTHESIS__
	static_assert(DATA_WIDTH <= STREAM_DATA_WIDTH,
			"MEM_DATA_WIDTH has to be less that equal AXIS_DATA_WIDTH");
#endif

	constexpr unsigned int bytes_per_data = DATA_WIDTH / 8;
	constexpr unsigned int bytes_per_strm_pkt = STREAM_DATA_WIDTH / 8;
	constexpr unsigned int num_data_per_strm_pkt = STREAM_DATA_WIDTH / DATA_WIDTH;
//	constexpr unsigned int shift_val = 1 << bytes_per_data;
	ap_uint<1> cond_write = 0;

#ifdef DEBUG_LOG_PRINT
	printf("\n|HLS DEBUG_LOG|%s| bytes_per_strm_pkt: %d, bytes_per_data: %d, num_data_per_strm_pkt: %d, beat index: %d\n"
			, __func__, bytes_per_strm_pkt, bytes_per_data, num_data_per_strm_pkt, index);
	printf("======================================================================================================\n");
#endif

	ap_uint<STREAM_DATA_WIDTH> data_pkt;
	ap_uint<STREAM_DATA_WIDTH/8> mask_pkt;

	data_pkt = data_in.read();
	mask_pkt = mask_in.read();

#ifdef DEBUG_LOG_PRINT
		printf("   |HLS DEBUG_LOG||%s| receiving axis , val=(", __func__);

		for (unsigned n = 0; n < bytes_per_strm_pkt/DEBUG_LOG_SIZE_OF; n++)
		{
			DataConv tmp;
			tmp.i = data_in.range((n+1) * DEBUG_LOG_SIZE_OF * 8 - 1, n * DEBUG_LOG_SIZE_OF * 8);
			printf("%f,", tmp.f);
		}

		printf(") strb=(%x)\n", mask_pkt);

#endif

	for (unsigned int idx = 0; idx < num_data_per_strm_pkt; idx++)
	{
#pragma HLS PIPELINE  II=1
#pragma HLS LOOP_TRIPCOUNT avg=avg_num_data_per_axis_pkt

		cond_write = mask_pkt.range(idx * bytes_per_data + 1, idx * bytes_per_data);
		ap_uint<DATA_WIDTH> write_val = data_pkt.range((idx+1) * DATA_WIDTH - 1, idx * DATA_WIDTH);

#ifdef DEBUG_LOG_PRINT
		DataConv tmp;
		tmp.i = write_val;
		printf("   |HLS DEBUG_LOG||%s| writing to mem index: %d, strb_pos:%d, val: %f, cond: %d\n"
				, __func__, index+idx, idx * bytes_per_data, tmp.f, cond_write);
#endif
		if (cond_write)
		{
			mem_out[index + idx] = write_val;
		}
	}
}

/**
 * @brief 	mem2stream reads from a memory location with to an hls stream.
 *  		This is optimized to read from AXI4 with burst and to utilize width maximum througput
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port and the hls stream port
 * @tparam BURST_SIZE : Burst length of the AXI4 (max beats < 256)
 * @tparam IN_ITR: II of the mem read
 *
 * @param mem_in : input memory port
 * @param stream_out : output hls-stream
 * @param size : Number of bytes of the data
 */
template <unsigned int MEM_DATA_WIDTH, unsigned int BURST_SIZE=32, unsigned int IN_ITR=2>
void mem2stream(ap_uint<MEM_DATA_WIDTH>* mem_in,
				::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_out,
				const unsigned int num_beats)
{
#ifndef __SYNTHESIS__
	static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
			"MEM_DATA_WIDTH failed limit check");
	static_assert(BURST_SIZE >= min_burst_len && BURST_SIZE <= max_burst_len,
			" BURST_SIZE has failed limit check");
	static_assert((BURST_SIZE > 0) && ((BURST_SIZE & (BURST_SIZE - 1)) == 0), 
            "BURST_SIZE must be a power of 2 for bitwise boundary optimization");
#endif

	constexpr unsigned int bytes_per_beat = MEM_DATA_WIDTH >> 3;
    constexpr unsigned int ii = IN_ITR;
	constexpr unsigned int BURST_MASK = BURST_SIZE - 1;

	const unsigned int non_burst_beats = num_beats & BURST_MASK; // num_beats % BURST_SIZE
    const unsigned int burst_beats = num_beats & ~BURST_MASK; // num_beats / BURST_SIZE

#ifdef DEBUG_LOG_PRINT
    const unsigned int num_bursts = num_beats / BURST_SIZE; 
	print("====================================================================================\n");
	print("|HLS DEBUG_LOG| mem2stream | num_beats: %d\n", num_beats);
	print("|HLS DEBUG_LOG| mem2stream | num_burst: %d\n", num_bursts);
	print("|HLS DEBUG_LOG| mem2stream | burst_beats: %d\n", burst_beats);
	print("|HLS DEBUG_LOG| mem2stream | non_burst_beats: %d\n\n", non_burst_beats);
	print("====================================================================================\n");
#endif


	unsigned int index = 0;

	for (unsigned int beat = 0; beat < burst_beats; beat++)
	{
    #pragma HLS PIPELINE II=ii

        ap_uint<MEM_DATA_WIDTH> tmp = mem_in[index];
        strm_out << tmp;
#ifdef DEBUG_LOG_PRINT
		print("====================================================================================\n");
        print("|HLS DEBUG_LOG| mem2stream | reading burst index: %d, val=(\n", index);

        for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
        {
            DataConv conv;
            conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
            print("		%f,\n", conv.f);
        }
        print(")\n\n");
		print("====================================================================================\n");
#endif
        index++;
	}

	for (unsigned int beat = 0; beat < non_burst_beats; beat++)
	{
	#pragma HLS PIPELINE II=ii
		ap_uint<MEM_DATA_WIDTH> tmp = mem_in[index];
		strm_out << tmp;
#ifdef DEBUG_LOG_PRINT
	print("====================================================================================\n");
        print("|HLS DEBUG_LOG| mem2stream | reading non-burst index: %d, val=(\n", index);

        for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
        {
            DataConv conv;
            conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
            print("		%f\n", conv.f);
        }
        print(")\n\n");
		print("====================================================================================\n");
#endif
		index++;
	}
}

/**
 * @brief 	mem2streamV2 reads from a memory location with to an hls stream. Focusing on
 * 			strict single loop, allowing the vitis to infer burst.
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port and the hls stream port
 * @tparam IN_ITR: II of the mem read
 *
 * @param mem_in : input memory port
 * @param stream_out : output hls-stream
 * @param size : Number of bytes of the data
 */
template <unsigned int MEM_DATA_WIDTH, unsigned int IN_ITR=2>
void mem2streamV2(ap_uint<MEM_DATA_WIDTH>* mem_in,
				::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_out,
				const unsigned int num_beats)
{
#ifndef __SYNTHESIS__
	static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
			"MEM_DATA_WIDTH failed limit check");
#endif

	constexpr unsigned int bytes_per_beat = MEM_DATA_WIDTH >> 3;

#ifdef DEBUG_LOG_PRINT
	print("====================================================================================\n");
	print("|HLS DEBUG_LOG| mem2stream | num_beats: %d\n", num_beats);
	print("====================================================================================\n");
#endif

	for (unsigned int beat = 0; beat < num_beats; beat++)
	{
    #pragma HLS PIPELINE II=IN_ITR

        ap_uint<MEM_DATA_WIDTH> tmp = mem_in[beat];
        strm_out << tmp;
#ifdef DEBUG_LOG_PRINT
		print("====================================================================================\n");
        print("|HLS DEBUG_LOG| mem2stream | reading burst index: %d, val=(\n", index);

        for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
        {
            DataConv conv;
            conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
            print("		%f,\n", conv.f);
        }
        print(")\n\n");
		print("====================================================================================\n");
#endif
	}
}

/**
 * @brief 	mem2stream variant 2: reads from a memory location with to an hls stream with different size.
 *  		This is optimized to read from AXI4 with burst and to utilize width maximum througput
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port and the hls stream port
 * @tparam BURST_SIZE : Burst length of the AXI4 (max beats < 256)
 * @tparam IN_ITR: II of the mem read from memory
 *
 * @param mem_in : input memory port
 * @param stream_out : output hls-stream
 * @param size : Number of bytes of the data
 */
template <unsigned int MEM_DATA_WIDTH, unsigned int STREAM_DATA_WIDTH, unsigned int BURST_SIZE=32, unsigned int IN_ITR=2>
void mem2stream(ap_uint<MEM_DATA_WIDTH>* mem_in,
                ::hls::stream<ap_uint<STREAM_DATA_WIDTH>>& strm_out,
                const unsigned int num_beats)
{
#ifndef __SYNTHESIS__
    // Existing width assertions
	static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
			"MEM_DATA_WIDTH failed limit check");
	static_assert(BURST_SIZE >= min_burst_len && BURST_SIZE <= max_burst_len,
			"BURST_SIZE has failed limit check");
	static_assert(STREAM_DATA_WIDTH >= min_axis_data_width && STREAM_DATA_WIDTH <= max_axis_data_width,
			"STREAM_DATA_WIDTH failed limit check");
    static_assert(STREAM_DATA_WIDTH <= MEM_DATA_WIDTH, 
            "STREAM_DATA_WIDTH must be less than or equal to MEM_DATA_WIDTH");
    static_assert(MEM_DATA_WIDTH % STREAM_DATA_WIDTH == 0, 
            "MEM_DATA_WIDTH must be an exact multiple of STREAM_DATA_WIDTH");
    static_assert((BURST_SIZE > 0) && ((BURST_SIZE & (BURST_SIZE - 1)) == 0), 
            "BURST_SIZE must be a power of 2 for bitwise boundary optimization");
#endif

    constexpr unsigned int pkts_per_beat = MEM_DATA_WIDTH / STREAM_DATA_WIDTH;
    constexpr unsigned int ii_adj = (IN_ITR > pkts_per_beat) ? IN_ITR : pkts_per_beat;
    constexpr unsigned int BURST_MASK = BURST_SIZE - 1;
    
    const unsigned int non_burst_beats = num_beats & BURST_MASK; //num_beats % BURST_SIZE
    const unsigned int burst_beats = num_beats & ~BURST_MASK; //num_beats / BURST_SIZE
#ifdef DEBUG_LOG_PRINT
    const unsigned int num_bursts = num_beats / BURST_SIZE; 
    print("====================================================================================\n");
    print("|HLS DEBUG_LOG| mem2stream_v2 | num_beats: %d\n", num_beats);
    print("|HLS DEBUG_LOG| mem2stream_v2 | num_burst: %d\n", num_bursts);
    print("|HLS DEBUG_LOG| mem2stream_v2 | non_burst_beats: %d\n", non_burst_beats);
    print("|HLS DEBUG_LOG| mem2stream_v2 | pkts_per_beat: %d \n", pkts_per_beat);
	print("|HLS DEBUG_LOG| mem2stream_v2 | adjusted_ii: %d \n", ii_adj);
    print("====================================================================================\n");
#endif
    unsigned int index = 0;

    for (unsigned int beat = 0; beat < burst_beats; beat++)
    {
    #pragma HLS PIPELINE II=ii_adj

        ap_uint<MEM_DATA_WIDTH> tmp = mem_in[index];
        
        for (unsigned int i = 0; i < pkts_per_beat; i++) 
        {
        #pragma HLS UNROLL
            strm_out << tmp.range((i + 1) * STREAM_DATA_WIDTH - 1, i * STREAM_DATA_WIDTH);
        }
#ifdef DEBUG_LOG_PRINT
        print("====================================================================================\n");
        print("|HLS DEBUG_LOG| mem2stream_v2 | reading burst index: %d, val=(\n", index);

        for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
        {
            DataConv conv;
            conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
            print("     %f,\n", conv.f);
        }
        print(")\n\n");
        print("====================================================================================\n");
#endif

        index++;
    }

    for (unsigned int beat = 0; beat < non_burst_beats; beat++)
    {
    #pragma HLS PIPELINE II=ii_adj
    
        ap_uint<MEM_DATA_WIDTH> tmp = mem_in[index];
        
        for (unsigned int i = 0; i < pkts_per_beat; i++) 
        {
        #pragma HLS UNROLL
            strm_out << tmp.range((i + 1) * STREAM_DATA_WIDTH - 1, i * STREAM_DATA_WIDTH);
        }
#ifdef DEBUG_LOG_PRINT
        print("====================================================================================\n");
        print("|HLS DEBUG_LOG| mem2stream_v2 | reading non-burst index: %d, val=(\n", index);

        for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
        {
            DataConv conv;
            conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
            print("     %f\n", conv.f);
        }
        print(")\n\n");
        print("====================================================================================\n");
#endif
        index++;
    }
}

// /**
//  * @brief 	mem2stream reads from a memory location with to an hls stream.
//  *  		This is optimized to read from AXI4 with burst and to utilize width maximum througput
//  *
//  * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port and the hls stream port
//  * @tparam BURST_SIZE : Burst length of the AXI4 (max beats < 256)
//  * @tparam IN_ITR: II of the mem read
//  *
//  * @param mem_in : input memory port
//  * @param stream_u_out : output msb_hls-stream
//  * @param stream_u_out : output lsb_hls-stream
//  * @param size : Number of bytes of the data
//  * @param enbl : enabling
//  */
// template <unsigned int MEM_DATA_WIDTH, unsigned int BURST_SIZE=32, unsigned int IN_ITR=2>
// void mem2stream(ap_uint<MEM_DATA_WIDTH>* mem_in,
// 				::hls::stream<ap_uint<MEM_DATA_WIDTH/2>>& strm_u_out,
// 				::hls::stream<ap_uint<MEM_DATA_WIDTH/2>>& strm_l_out,
// 				const unsigned int num_beats)
// {
// #ifndef __SYNTHESIS__
// 	static_assert(MEM_DATA_WIDTH % 2 == 0,
// 			"MEM_DATA_WIDTH sould be divisible by 2");
// 	static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
// 			"MEM_DATA_WIDTH failed limit check");
// 	static_assert(BURST_SIZE >= min_burst_len && BURST_SIZE <= max_burst_len,
// 			" BURST_SIZE has failed limit check");
// #endif

// 	constexpr unsigned short bytes_per_beat = MEM_DATA_WIDTH / 8;
//     constexpr unsigned short ii = IN_ITR;
// 	constexpr unsigned short burst_size = BURST_SIZE;
// 	constexpr unsigned short mem_data_width_by_2 = MEM_DATA_WIDTH / 2;
// 	constexpr unsigned short mem_data_min_1 = MEM_DATA_WIDTH - 1;
// 	constexpr unsigned short mem_data_width_by_2_min_1 = mem_data_width_by_2 - 1;
// 	const unsigned int non_burst_beats = num_beats % BURST_SIZE;
//     const unsigned int burst_beats = num_beats - non_burst_beats;


// #ifdef DEBUG_LOG_PRINT
//     const unsigned int num_bursts = num_beats / BURST_SIZE; 
// 	print("|HLS DEBUG_LOG| mem2stream | num_beats: %d\n", num_beats);
// 	print("|HLS DEBUG_LOG| mem2stream | num_burst: %d\n", num_bursts);
// 	print("|HLS DEBUG_LOG| mem2stream | non_burst_beats: %d\n", non_burst_beats);
// 	print("====================================================================================\n");
// #endif


// 	unsigned int index = 0;

// 	for (unsigned int beat = 0; beat < burst_beats; beat++)
// 	{
//     #pragma HLS PIPELINE II=ii

//         ap_uint<MEM_DATA_WIDTH> tmp = mem_in[index];
//         strm_u_out << tmp.range(MEM_DATA_WIDTH - 1, mem_data_width_by_2);
// 		strm_l_out << tmp.range(mem_data_width_by_2_min_1, 0);

//  #ifdef DEBUG_LOG_PRINT
//         print("|HLS DEBUG_LOG| mem2stream | reading burst index: %d, val=(\n", index);

//         for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
//         {
//             DataConv conv;
//             conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
//             print("		%f,\n", conv.f);
//         }
//         print(")\n");
//  #endif

//         index++;
// 	}

// 	for (unsigned int beat = 0; beat < non_burst_beats; beat++)
// 	{
// 	#pragma HLS PIPELINE II=ii
// 		ap_uint<MEM_DATA_WIDTH> tmp  = mem_in[index];
// 		strm_u_out << tmp.range(MEM_DATA_WIDTH - 1, mem_data_width_by_2);
// 		strm_l_out << tmp.range(mem_data_width_by_2_min_1, 0);
// #ifdef DEBUG_LOG_PRINT
//         print("|HLS DEBUG_LOG| mem2stream | reading non-burst index: %d, val=(\n", index);

//         for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
//         {
//             DataConv conv;
//             conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
//             print("		%f\n", conv.f);
//         }
//         print(")\n");
// #endif
// 		index++;
// 	}
// }

/**
 * @brief 	mem2stream reads from a memory location with to an hls stream.
 *  		This is optimized to read from AXI4 with burst and to utilize width maximum througput
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port and the hls stream port
 * @tparam BURST_SIZE : Burst length of the AXI4 (max beats < 256)
 *
 * @param mem_in : input memory port
 * @param stream_out : output hls-stream
 * @param config : Memconfig to guide reading
 */
template <unsigned int MEM_DATA_WIDTH, unsigned int BURST_SIZE=32>
void mem2stream(ap_uint<MEM_DATA_WIDTH>* mem_in,
				::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_out,
				const MemConfig& config)
{
#ifdef DEBUG_LOG_PRINT
		printf("|HLS DEBUG_LOG|%s| starting\n", __func__);
#endif
	if (config.isContinous)
	{
#ifdef DEBUG_LOG_PRINT
		printf("|HLS DEBUG_LOG|%s| continuous read\n", __func__);
#endif

#ifdef DEBUG_LOG_PRINT
		printf("|HLS DEBUG_LOG|%s| init offset:%d, size_bytes:%d\n", __func__, config.start_offset, config.total_size_bytes);
#endif
		mem2stream<MEM_DATA_WIDTH>((ap_uint<MEM_DATA_WIDTH>* )(mem_in + config.start_offset), strm_out, config.total_xblocks);
	}
	else
	{
		for (unsigned short k = config.start_z; k < config.end_z; k++)
		{
			for (unsigned short j = config.start_y; j < config.end_y; j++)
			{
			#pragma HLS LOOP_FLATTEN
				unsigned int offset = config.start_x + j * config.grid_xblocks + k * config.grid_size_y * config.grid_xblocks;
    #ifdef DEBUG_LOG_PRINT
				printf("|HLS DEBUG_LOG|%s| reading. offset:%d, j:%d, k:%d\n"
						, __func__,offset, j, k);
    #endif
				mem2stream<MEM_DATA_WIDTH>((ap_uint<MEM_DATA_WIDTH>* )(mem_in + offset), strm_out, config.num_xblocks);
			}
		}
	}
#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| exiting.\n"
			, __func__);
#endif
}

template <unsigned short MEM_DATA_WIDTH, unsigned short IN_ITR=2>
static void stream2streambuffered(::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in, ::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_out, const unsigned int num_beats)
{
	for (unsigned int i = 0; i < num_beats; i++)
	{
#pragma HLS PIPELINE II=IN_ITR
		auto val = strm_in.read();
		strm_out << val;
	}
}

/**
 * @brief 	stream2mem reads from a memory from hls stram and write to memory
 *  		This is optimized to write to AXI4 with burst and to utilize maximum throughput.
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port the hls stream port
 * @tparam BURST_SIZE : Burst length of the AXI4 (max beats < 256)
 * @tparam IN_ITR: II configuration of mem write
 *
 * @param mem_out : out memory port
 * @param stream_in : input hls-stream
 * @param size : Number of bytes of the data
 */
template <unsigned int MEM_DATA_WIDTH, unsigned int BURST_SIZE=32, unsigned int IN_ITR=2>
void stream2mem(ap_uint<MEM_DATA_WIDTH>* mem_out,
				::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in,
				const unsigned int num_beats)
{
#ifndef __SYNTHESIS__
	static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
			"MEM_DATA_WIDTH failed limit check");
	static_assert(BURST_SIZE >= min_burst_len && BURST_SIZE <= max_burst_len,
			" BURST_SIZE has failed limit check");
#endif

	constexpr unsigned int bytes_per_beat = MEM_DATA_WIDTH / 8;
    constexpr unsigned int ii = IN_ITR;
	const unsigned int burst_size = BURST_SIZE;
	const unsigned int non_burst_beats = num_beats % BURST_SIZE;
    const unsigned int burst_beats = num_beats - non_burst_beats;

#ifdef DEBUG_LOG_PRINT
    const unsigned int num_bursts = num_beats / BURST_SIZE;
	printf("|HLS DEBUG_LOG| %s | num_beats: %d, num_burst: %d, non_burst_beats: %d\n"
			, __func__, num_beats, num_bursts, non_burst_beats);
	printf("====================================================================================\n");
#endif

	unsigned int index = 0;

	for (unsigned int beat = 0; beat < burst_beats; beat++)
	{
    #pragma HLS PIPELINE II=ii

        ap_uint<MEM_DATA_WIDTH> tmp = strm_in.read();
        mem_out[index] = tmp;	
#ifdef DEBUG_LOG_PRINT
        printf("|HLS DEBUG_LOG| %s | writing burst index: %d, val=(\n", __func__, index);

        for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
        {
            DataConv conv;
            conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
            printf("%f,", conv.f);
        }
        printf(")\n");
#endif
        index++;
		
	}

	for (unsigned int beat = 0; beat < non_burst_beats; beat++)
	{
	#pragma HLS PIPELINE II=ii
		ap_uint<MEM_DATA_WIDTH> tmp = strm_in.read();
		mem_out[index] = tmp;
#ifdef DEBUG_LOG_PRINT
        printf("|HLS DEBUG_LOG| %s | writing non-burst index: %d, val=(\n", __func__, index);

        for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
        {
            DataConv conv;
            conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
            printf("%f,", conv.f);
        }
        printf(")\n");
#endif
        index++;
	}
#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| exiting.\n"
			, __func__);
#endif
}

/**
 * @brief hlsTerminate read from axis stream and discard the packets
 *
 * @tparam AXIS_DATA_WIDTH : Data width of the AXI4-stream port
 * @tparam II : Initial Interval of the read
 * 
 * @param strm_in : AXI4-stream input
 * @param num_pkts: number of axis pkts
 */
template <unsigned int HLS_DATA_WIDTH, unsigned int II=1>
void hlsTerminate(::hls::stream<ap_uint<HLS_DATA_WIDTH>>& strm_in,
		unsigned int num_pkts)
{
#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| starting.\n"
			, __func__);
#endif
	for (unsigned int i = 0; i < num_pkts; i++)
	{
#pragma HLS PIPELINE II=II
#ifdef DEBUG_LOG_PRINT
		printf("|HLS DEBUG_LOG|%s| terminating pkt:%d.\n"
				, __func__, i);
#endif
		auto pkt = strm_in.read();
	}
#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| exiting.\n"
			, __func__);
#endif
}

/**
 * @brief 	stream2memWithAvoid reads from an hls stream and writes to memory while skipping initial beats.
 *  		This is optimized to write to AXI4 with burst and to utilize maximum throughput.
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port and the hls stream port
 * @tparam BURST_SIZE : Burst length of the AXI4 (max beats < 256)
 * @tparam IN_ITR: II configuration of mem write
 *
 * @param mem_out : output memory port
 * @param stream_in : input hls-stream
 * @param num_beats : Total number of beats to process from the stream
 * @param avoid_beats : Number of initial beats to skip/discard before writing to memory
 */
template <unsigned int MEM_DATA_WIDTH, unsigned int BURST_SIZE=32, unsigned int IN_ITR=2>
void stream2memWithAvoid(ap_uint<MEM_DATA_WIDTH>* mem_out,
				::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in,
				const unsigned int num_beats, const unsigned int avoid_beats)
{
#ifndef __SYNTHESIS__
	static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
			"MEM_DATA_WIDTH failed limit check");
	static_assert(BURST_SIZE >= min_burst_len && BURST_SIZE <= max_burst_len,
			" BURST_SIZE has failed limit check");
	static_assert((BURST_SIZE > 0) && ((BURST_SIZE & (BURST_SIZE - 1)) == 0), 
                  "BURST_SIZE must be a power of 2 for bitwise optimization");
#endif

	constexpr unsigned int bytes_per_beat = MEM_DATA_WIDTH / 8;
    constexpr unsigned int ii = IN_ITR;
	const unsigned int burst_size = BURST_SIZE;
	const unsigned int burst_shift = LOG2(BURST_SIZE);
	const unsigned int writing_beats = num_beats - avoid_beats;
	const unsigned int non_burst_beats = writing_beats % BURST_SIZE;
    const unsigned int burst_beats = writing_beats - non_burst_beats;
	const unsigned int num_bursts = burst_beats >> burst_shift;

#ifdef DEBUG_LOG_PRINT
	print("====================================================================================\n");
	print("|HLS DEBUG_LOG| stream2memWithAvoid | num_beats: %d\n", num_beats);
	print("|HLS DEBUG_LOG| stream2memWithAvoid | burst_beats: %d\n", burst_beats);
	print("|HLS DEBUG_LOG| stream2memWithAvoid | non_burst_beats: %d\n", non_burst_beats);
	print("|HLS DEBUG_LOG| stream2memWithAvoid | num_burst: %d\n", num_bursts);
	print("|HLS DEBUG_LOG| stream2memWithAvoid | avoid_beats: %d\n", avoid_beats);
	print("====================================================================================\n");
#endif

	unsigned int index = 0;

	for (unsigned int beat = 0; beat < avoid_beats; beat++)
	{
	#pragma HLS PIPELINE II=ii
		auto tmp = strm_in.read(); //Discarding
#ifdef DEBUG_LOG_PRINT
		print("====================================================================================\n");
		print("|HLS DEBUG_LOG| stream2memWithAvoid | discarding index: %d, val=(\n", index);
		for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
		{
			DataConv conv;
			conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
			print("		%f,\n", conv.f);
		}
		print(")%d \n\n",0);
		print("====================================================================================\n");
#endif
		index++;
	}

	for (unsigned int burst = 0; burst < num_bursts; burst++)
	{
		for(unsigned int beat = 0; beat < BURST_SIZE; beat++)
		{
		#pragma HLS PIPELINE II=ii

			ap_uint<MEM_DATA_WIDTH> tmp = strm_in.read();
			mem_out[index] = tmp;	
#ifdef DEBUG_LOG_PRINT
			print("====================================================================================\n");
        	print("|HLS DEBUG_LOG| stream2memWithAvoid | writing burst index: %d, val=(\n", index);
			for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
			{
				DataConv conv;
				conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
				print("		%f,\n", conv.f);
			}
			print(")%d \n\n",0);
			print("====================================================================================\n");
#endif
			index++;
		}
	}

	for (unsigned int beat = 0; beat < non_burst_beats; beat++)
	{
	#pragma HLS PIPELINE II=ii
		ap_uint<MEM_DATA_WIDTH> tmp = strm_in.read();
		mem_out[index] = tmp;
#ifdef DEBUG_LOG_PRINT
        print("====================================================================================\n");
		print("|HLS DEBUG_LOG| stream2memWithAvoid | writing non-burst index: %d, val=(\n", index);

        for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
        {
            DataConv conv;
            conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
            print("		%f,\n", conv.f);
        }
        print(")%d\n\n",0);
		print("====================================================================================\n");
#endif
        index++;
	}
}


/**
 * @brief 	stream2memWithAvoidV2 reads from an hls stream and writes to memory while skipping initial beats.
 *  		enforcing single loop to make vitis infer burst size
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port and the hls stream port
 * @tparam BURST_SIZE : Burst length of the AXI4 (max beats < 256)
 * @tparam IN_ITR: II configuration of mem write
 *
 * @param mem_out : output memory port
 * @param stream_in : input hls-stream
 * @param num_beats : Total number of beats to process from the stream
 * @param avoid_beats : Number of initial beats to skip/discard before writing to memory
 */
template <unsigned int MEM_DATA_WIDTH, unsigned int IN_ITR=2>
void stream2memWithAvoidV2(ap_uint<MEM_DATA_WIDTH>* mem_out,
				::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in,
				const unsigned int num_beats, const unsigned int avoid_beats)
{
#ifndef __SYNTHESIS__
	static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
			"MEM_DATA_WIDTH failed limit check");
#endif

	constexpr unsigned int bytes_per_beat = MEM_DATA_WIDTH / 8;

	const unsigned int writing_beats = num_beats - avoid_beats;

#ifdef DEBUG_LOG_PRINT
	print("====================================================================================\n");
	print("|HLS DEBUG_LOG| stream2memWithAvoid | num_beats: %d\n", num_beats);
	print("|HLS DEBUG_LOG| stream2memWithAvoid | writing_beats: %d\n", writing_beats);
	print("|HLS DEBUG_LOG| stream2memWithAvoid | avoid_beats: %d\n", avoid_beats);
	print("====================================================================================\n");
#endif

	for (unsigned int beat = 0; beat < num_beats; beat++)
	{
		#pragma HLS PIPELINE II=IN_ITR

			ap_uint<MEM_DATA_WIDTH> tmp = strm_in.read();
			if (beat >= avoid_beats)
				mem_out[beat] = tmp;	
#ifdef DEBUG_LOG_PRINT
			print("====================================================================================\n");
        	print("|HLS DEBUG_LOG| stream2memWithAvoid | writing burst index: %d, val=(\n", beat);
			for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
			{
				DataConv conv;
				conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
				print("		%f,\n", conv.f);
			}
			print(")%d \n\n",0);
			print("====================================================================================\n");
#endif
	}
}

/**
 * @brief   stream2memWithAvoid variant 2: reads from a smaller hls stream and writes to wider memory while skipping initial beats.
 * This is optimized to write to AXI4 with burst and to utilize maximum throughput.
 *
 * @tparam MEM_DATA_WIDTH    : Data width of the AXI4 port
 * @tparam STREAM_DATA_WIDTH : Data width of the hls stream port
 * @tparam BURST_SIZE        : Burst length of the AXI4 (MUST be power of 2, max beats < 256)
 * @tparam IN_ITR            : II configuration of mem write
 *
 * @param mem_out     : output memory port
 * @param strm_in     : input hls-stream
 * @param num_beats   : Total number of MEMORY beats to process from the stream
 * @param avoid_beats : Number of initial MEMORY beats to skip/discard before writing
 */
template <unsigned int MEM_DATA_WIDTH, unsigned int STREAM_DATA_WIDTH, unsigned int BURST_SIZE=32, unsigned int IN_ITR=2>
void stream2memWithAvoid(ap_uint<MEM_DATA_WIDTH>* mem_out,
                         ::hls::stream<ap_uint<STREAM_DATA_WIDTH>>& strm_in,
                         const unsigned int num_beats, const unsigned int avoid_beats)
{
#ifndef __SYNTHESIS__ 

    static_assert(STREAM_DATA_WIDTH <= MEM_DATA_WIDTH, 
        	"STREAM_DATA_WIDTH must be less than or equal to MEM_DATA_WIDTH");
    static_assert(MEM_DATA_WIDTH % STREAM_DATA_WIDTH == 0, 
            "MEM_DATA_WIDTH must be an exact multiple of STREAM_DATA_WIDTH");
    static_assert(STREAM_DATA_WIDTH >= min_axis_data_width && STREAM_DATA_WIDTH <= max_axis_data_width,
			"STREAM_DATA_WIDTH failed limit check");
	static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
			"MEM_DATA_WIDTH failed limit check");
	static_assert(BURST_SIZE >= min_burst_len && BURST_SIZE <= max_burst_len,
			" BURST_SIZE has failed limit check");
    static_assert((BURST_SIZE > 0) && ((BURST_SIZE & (BURST_SIZE - 1)) == 0), 
            "BURST_SIZE must be a power of 2 for bitwise optimization");
#endif

    constexpr unsigned int pkts_per_beats = MEM_DATA_WIDTH / STREAM_DATA_WIDTH;
    
    // Adjust II internally: It takes at least pkts_per_beats cycles to read the data from a single stream port
    constexpr unsigned int ii_adj = (IN_ITR > pkts_per_beats) ? IN_ITR : pkts_per_beats;

    // Bitwise boundary math
    constexpr unsigned int BURST_MASK = BURST_SIZE - 1;
    constexpr unsigned int burst_shift = LOG2(BURST_SIZE);
    
    const unsigned int writing_beats = num_beats - avoid_beats;
    const unsigned int non_burst_beats = writing_beats & BURST_MASK;
    const unsigned int burst_beats = writing_beats & ~BURST_MASK;
    const unsigned int num_bursts = burst_beats >> burst_shift;

#ifdef DEBUG_LOG_PRINT
    print("====================================================================================\n");
    print("|HLS DEBUG_LOG| stream2memWithAvoid_v2 | num_beats: %d\n", num_beats);
    print("|HLS DEBUG_LOG| stream2memWithAvoid_v2 | burst_beats: %d\n", burst_beats);
    print("|HLS DEBUG_LOG| stream2memWithAvoid_v2 | non_burst_beats: %d\n", non_burst_beats);
    print("|HLS DEBUG_LOG| stream2memWithAvoid_v2 | num_burst: %d\n", num_bursts);
    print("|HLS DEBUG_LOG| stream2memWithAvoid_v2 | avoid_beats: %d\n", avoid_beats);
    print("|HLS DEBUG_LOG| stream2memWithAvoid_v2 | pkts_per_beats: %d \n", pkts_per_beats);
	print("|HLS DEBUG_LOG| stream2memWithAvoid_v2 | adjusted_ii: %d \n", ii_adj);
    print("====================================================================================\n");
#endif

    unsigned int index = 0;

    // 1. DISCARD LOOP
    for (unsigned int beat = 0; beat < avoid_beats; beat++)
    {
    #pragma HLS PIPELINE II=ii_adj
        ap_uint<MEM_DATA_WIDTH> tmp; 
        
        // Read pkts_per_beats times to clear the equivalent of one MEM_DATA_WIDTH beat
        for (unsigned int i = 0; i < pkts_per_beats; i++) 
        {
        #pragma HLS UNROLL
            tmp.range((i + 1) * STREAM_DATA_WIDTH - 1, i * STREAM_DATA_WIDTH) = strm_in.read();
        }

#ifdef DEBUG_LOG_PRINT
        print("====================================================================================\n");
        print("|HLS DEBUG_LOG| stream2memWithAvoid_v2 | discarding index: %d, val=(\n", index);
        for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
        {
            DataConv conv;
            conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
            print("     %f,\n", conv.f);
        }
        print(")%d \n\n",0);
        print("====================================================================================\n");
#endif
        index++;
    }

    // 2. BURST WRITE LOOP
    for (unsigned int burst = 0; burst < num_bursts; burst++)
    {
        for(unsigned int beat = 0; beat < BURST_SIZE; beat++)
        {
        #pragma HLS PIPELINE II=ii_adj

            ap_uint<MEM_DATA_WIDTH> tmp;
            
            // Pack stream data into the memory beat
            for (unsigned int i = 0; i < pkts_per_beats; i++) 
            {
            #pragma HLS UNROLL
                tmp.range((i + 1) * STREAM_DATA_WIDTH - 1, i * STREAM_DATA_WIDTH) = strm_in.read();
            }
            
            mem_out[index] = tmp;       

#ifdef DEBUG_LOG_PRINT
            print("====================================================================================\n");
            print("|HLS DEBUG_LOG| stream2memWithAvoid_v2 | writing burst index: %d, val=(\n", index);
            for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
            {
                DataConv conv;
                conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
                print("     %f,\n", conv.f);
            }
            print(")%d \n\n",0);
            print("====================================================================================\n");
#endif
            index++;
        }
    }

    // 3. REMAINDER WRITE LOOP
    for (unsigned int beat = 0; beat < non_burst_beats; beat++)
    {
    #pragma HLS PIPELINE II=ii_adj
    
        ap_uint<MEM_DATA_WIDTH> tmp;
        
        // Pack remainder data
        for (unsigned int i = 0; i < pkts_per_beats; i++) 
        {
        #pragma HLS UNROLL
            tmp.range((i + 1) * STREAM_DATA_WIDTH - 1, i * STREAM_DATA_WIDTH) = strm_in.read();
        }
        
        mem_out[index] = tmp;
        
#ifdef DEBUG_LOG_PRINT
        print("====================================================================================\n");
        print("|HLS DEBUG_LOG| stream2memWithAvoid_v2 | writing non-burst index: %d, val=(\n", index);

        for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
        {
            DataConv conv;
            conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
            print("     %f,\n", conv.f);
        }
        print(")%d\n\n",0);
        print("====================================================================================\n");
#endif
        index++;
    }
    
#ifdef DEBUG_LOG_PRINT
    printf("|HLS DEBUG_LOG|%s| exiting.\n"
            , __func__);
#endif
}

/**
 * @brief 	stream2mem reads from a memory from hls stram and write to memory
 *  		This is optimized to write to AXI4 with burst and to utilize maximum throughput.
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port the hls stream port
 * @tparam BURST_SIZE : Burst length of the AXI4 (max beats < 256)
 *
 * @param mem_out : out memory port
 * @param stream_in : input hls-stream
 * @param config : Memconfig to guide reading
 */
template <unsigned int MEM_DATA_WIDTH, unsigned int BURST_SIZE=32>
void stream2mem(ap_uint<MEM_DATA_WIDTH>* mem_out,
				::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in,
				 const MemConfig& config)
{
#ifdef DEBUG_LOG_PRINT
		printf("|HLS DEBUG_LOG|%s| starting\n", __func__);
#endif
	if (config.isContinous)
	{
#ifdef DEBUG_LOG_PRINT
		printf("|HLS DEBUG_LOG|%s| continuous read\n", __func__);
#endif

#ifdef DEBUG_LOG_PRINT
		printf("|HLS DEBUG_LOG|%s| init offset:%d, size_bytes:%d\n", __func__, config.start_offset, config.total_size_bytes);
#endif
		stream2mem<MEM_DATA_WIDTH>((ap_uint<MEM_DATA_WIDTH>* )(mem_out + config.start_offset), strm_in, config.total_xblocks);
	}
	else
	{
		for (unsigned short k = config.start_z; k < config.end_z; k++)
		{
			for (unsigned short j = config.start_y; j < config.end_y; j++)
			{
			#pragma HLS LOOP_FLATTEN
				unsigned int offset = config.start_x + j * config.grid_xblocks + k * config.grid_size_y * config.grid_xblocks;
    #ifdef DEBUG_LOG_PRINT
				printf("|HLS DEBUG_LOG|%s| reading. offset:%d, j:%d, k:%d\n"
						, __func__,offset, j, k);
    #endif
				stream2mem<MEM_DATA_WIDTH>((ap_uint<MEM_DATA_WIDTH>* )(mem_out + offset), strm_in, config.num_xblocks);
			}
		}
	}
#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| exiting.\n"
			, __func__);
#endif
}

/**************************** TILED ops  ****************************/

/**
 * @brief 	stream2interleave reads from number of HLS stream and create in-terleved stream for tiled processing
 *
 * @details This function reads data from NUM_STREAMS number of input streams of size MEM_DATAWIDTH and create
 * 			NUM_STREAMS of output streams of size, MEM_DATAWIDTH+DATA_WIDTH*OVERLAP_SIZE to support NUM_STREAMS/2 
 * 			process elements. Each process element consumes two adjacent output streams where the borders will be
 * 			overlapped over the process element boundary. 
 *
 * @tparam MEM_DATA_WIDTH The bit-width of each memory access (e.g., 64, 128, 256)
 * @tparam DATA_WIDTH Element datawidth
 * @tparam OVERLAP_SIZE Overlap size of the low level tiles
 * @tparam NUM_STREAMS Number of input, output streams
 *
 * @param[in] in_streams - Input streams array of size MEM_DATA_WIDTH
 * @param[out] out_sreams - Output streams array of size MEM_DATA_WIDTH + DATA_WIDTH * OVERLAP_SIZE
 * @param[in] num_pkts - Number of stream packets
 */

template <unsigned short MEM_DATA_WIDTH, unsigned short DATA_WIDTH, unsigned short NUM_STREAMS, unsigned short OVERLAP_SIZE = 1>
void stream2interleave(::hls::stream<ap_uint<MEM_DATA_WIDTH>> in_stream[NUM_STREAMS], ::hls::stream<ap_uint<MEM_DATA_WIDTH+DATA_WIDTH*OVERLAP_SIZE>> out_stream[NUM_STREAMS], const unsigned int num_pkts) 
{
#ifndef __SYNTHESIS__
	static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
			"MEM_DATA_WIDTH failed limit check");
	static_assert(NUM_STREAMS % 2 == 0,
			" NUM_STREAMS has to be divisible by 2");
#endif

	constexpr unsigned short REALISED_OVERLAP_SIZE = DATA_WIDTH * OVERLAP_SIZE;
	constexpr unsigned short REALISED_OVERLAP_SIZE_MIN_1 = REALISED_OVERLAP_SIZE - 1;
	constexpr unsigned short MSB_IN = MEM_DATA_WIDTH - 1;
	constexpr unsigned short LSB_LAST_OVERLAP_IN = MEM_DATA_WIDTH - REALISED_OVERLAP_SIZE;
	constexpr unsigned short MEM_DATA_WIDTH_OUT = MEM_DATA_WIDTH + REALISED_OVERLAP_SIZE;
	constexpr unsigned short MSB_OUT = MEM_DATA_WIDTH_OUT - 1;
	constexpr unsigned short LSB_LAST_OVERLAP_OUT = MEM_DATA_WIDTH_OUT - REALISED_OVERLAP_SIZE;
	constexpr unsigned short NUM_STREAMS_BY_2 = NUM_STREAMS >> 1;
	constexpr unsigned short NUM_STREAMS_MIN_1 = NUM_STREAMS - 1;

	const unsigned int num_pkts_plus_1 = register_it(num_pkts + 1);

#ifdef DEBUG_LOG_PRINT
	printf("==== DATAMOVER STREAM2INTERLEAVE INITIAL PARAMETERS ====\n");
	printf("MEM_DATA_WIDTH: %u\n", (unsigned int)MEM_DATA_WIDTH);
	printf("DATA_WIDTH: %u\n", (unsigned int)DATA_WIDTH);
	printf("OVERLAP_SIZE: %u\n", (unsigned int)OVERLAP_SIZE);
	printf("NUM_STREAMS: %u\n", (unsigned int)NUM_STREAMS);
	printf("REALISED_OVERLAP_SIZE: %u\n", (unsigned int)REALISED_OVERLAP_SIZE);
	printf("MEM_DATA_WIDTH_OUT: %u\n", (unsigned int)MEM_DATA_WIDTH_OUT);
	printf("num_pkts: %u\n", num_pkts);
	printf("========================================================\n");
#endif

	// 3 step shift registers
	ap_uint<MEM_DATA_WIDTH> data_front[NUM_STREAMS], data[NUM_STREAMS], data_back[NUM_STREAMS];
	#pragma HLS ARRAY_PARTITION variable=data_front dim=0 complete
	#pragma HLS ARRAY_PARTITION variable=data dim=0 complete
	#pragma HLS ARRAY_PARTITION variable=data_back dim=0 complete

	for (unsigned int itr = 0; itr < num_pkts_plus_1; itr++) 
	{
		#pragma HLS PIPELINE II=1
		bool read_cond = register_it(itr < num_pkts);
		ap_uint<MEM_DATA_WIDTH> tmp[NUM_STREAMS];
		#pragma HLS ARRAY_PARTITION variable=tmp dim=0 complete

		if (read_cond) {
			for (unsigned int n = 0; n < NUM_STREAMS; n++)
			{
				#pragma HLS UNROLL
				tmp[n] = in_stream[n].read();	

#ifdef DEBUG_LOG_PRINT
				printf("==== STREAM2INTERLEAVE READ ====\n");
				printf("Stream[%u], Iteration[%u]\n", n, itr);
				printf("Values (float): (");
				for (unsigned int j = 0; j < MEM_DATA_WIDTH / (DEBUG_LOG_SIZE_OF * 8); j++) {
					DataConv conv;
					conv.i = tmp[n].range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
					if (j > 0) printf(", ");
					printf("%f", conv.f);
				}
				printf(")\n");
				printf("===============================\n");
#endif
			}
		}

		for (unsigned int n = 0; n < NUM_STREAMS; n++)
		{
			#pragma HLS UNROLL
			data_back[n] = data[n];
			data[n] = data_front[n];
			data_front[n] =  register_it(tmp[n]);	
		}

		ap_uint<MEM_DATA_WIDTH_OUT> tmp_out[NUM_STREAMS];
		#pragma HLS ARRAY_PARTITION variable=tmp_out dim=1 complete

		if (itr > 0)
		{
			for (unsigned int n = 0; n < NUM_STREAMS_BY_2; n++)
			{
				#pragma HLS UNROLL
				tmp_out[2*n].range(MSB_OUT,REALISED_OVERLAP_SIZE) = data[2*n]; 
				
				if (n == 0) {
					tmp_out[2*n].range(REALISED_OVERLAP_SIZE_MIN_1,0) = data_back[NUM_STREAMS_MIN_1].range(MSB_IN, LSB_LAST_OVERLAP_IN);
				} else {
					tmp_out[2*n].range(REALISED_OVERLAP_SIZE_MIN_1,0) = data[2*n-1].range(MSB_IN, LSB_LAST_OVERLAP_IN);
				}
				tmp_out[2*n+1].range(MSB_IN, 0) = data[2*n+1];
				
				if (n == NUM_STREAMS_BY_2 - 1) {
					tmp_out[2*n+1].range(MSB_OUT, LSB_LAST_OVERLAP_OUT) = data_front[0].range(REALISED_OVERLAP_SIZE_MIN_1,0);
				} else {
					tmp_out[2*n+1].range(MSB_OUT, LSB_LAST_OVERLAP_OUT) = data[2*n+2].range(REALISED_OVERLAP_SIZE_MIN_1,0);
				}
			}

			for (unsigned int n = 0; n < NUM_STREAMS; n++)
			{
				#pragma HLS UNROLL
				out_stream[n].write(tmp_out[n]);

#ifdef DEBUG_LOG_PRINT
				printf("==== STREAM2INTERLEAVE WRITE ====\n");
				printf("Stream[%u], Iteration[%u]\n", n, itr);
				printf("Values (float): (");
				for (unsigned int j = 0; j < MEM_DATA_WIDTH_OUT / (DEBUG_LOG_SIZE_OF * 8); j++) {
					DataConv conv;
					conv.i = tmp_out[n].range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
					if (j > 0) printf(", ");
					printf("%f", conv.f);
				}
				printf(")\n");
				printf("================================\n");
#endif
			}
		}
	}
}


/**
 * @brief 	interleav2stream reads from number of interleaved HLS stream and create standard stream from tiled processing
 *
 * @details This function reads data from NUM_STREAMS number of input streams of size MEM_DATAWIDTH+DATA_WIDTH*OVERLAP_SIZE 
 * 			and create NUM_STREAMS of output streams of size, MEM_DATAWIDTH to from NUM_STREAMS/2 process
 * 			elements.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of each memory access (e.g., 64, 128, 256)
 * @tparam DATA_WIDTH Element datawidth
 * @tparam OVERLAP_SIZE Overlap size of the low level tiles
 * @tparam NUM_STREAMS Number of input, output streams
 *
 * @param[in] in_streams - Input streams array of size MEM_DATA_WIDTH + DATA_WIDTH * OVERLAP_SIZE
 * @param[out] out_sreams - Output streams array of size MEM_DATA_WIDTH
 * @param[in] num_pkts - Number of stream packets
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short DATA_WIDTH, unsigned short NUM_STREAMS, unsigned short OVERLAP_SIZE = 1>
void interleave2stream(::hls::stream<ap_uint<MEM_DATA_WIDTH+DATA_WIDTH*OVERLAP_SIZE>> in_stream[NUM_STREAMS], ::hls::stream<ap_uint<MEM_DATA_WIDTH>> out_stream[NUM_STREAMS], const unsigned int num_pkts) 
{
	constexpr unsigned short REALISED_OVERLAP_SIZE = DATA_WIDTH * OVERLAP_SIZE;
	constexpr unsigned short MEM_DATA_WIDTH_IN = MEM_DATA_WIDTH + REALISED_OVERLAP_SIZE;
	constexpr unsigned short MSB_IN = MEM_DATA_WIDTH_IN - 1;
	constexpr unsigned short MSB_OUT = MEM_DATA_WIDTH - 1;
	constexpr unsigned short NUM_STREAMS_BY_2 = NUM_STREAMS >> 1;

#ifdef DEBUG_LOG_PRINT
    printf("==== DATAMOVER INTERLEAVE2STREAM INITIAL PARAMETERS ====\n");
    printf("MEM_DATA_WIDTH: %u\n", (unsigned int)MEM_DATA_WIDTH);
    printf("DATA_WIDTH: %u\n", (unsigned int)DATA_WIDTH);
    printf("OVERLAP_SIZE: %u\n", (unsigned int)OVERLAP_SIZE);
    printf("NUM_STREAMS: %u\n", (unsigned int)NUM_STREAMS);
    printf("REALISED_OVERLAP_SIZE: %u\n", (unsigned int)REALISED_OVERLAP_SIZE);
    printf("MEM_DATA_WIDTH_IN: %u\n", (unsigned int)MEM_DATA_WIDTH_IN);
    printf("num_pkts: %u\n", num_pkts);
    printf("========================================================\n");
#endif

	ap_uint<MEM_DATA_WIDTH_IN> tmp_in[NUM_STREAMS];
	#pragma HLS ARRAY_PARTITION variable = tmp_in dim=0 complete
	ap_uint<MEM_DATA_WIDTH> tmp_out[NUM_STREAMS];
	#pragma HLS ARRAY_PARTITION variable = tmp_out dim=0 complete

	for (unsigned int itr = 0; itr < num_pkts; itr++) 
	{
		#pragma HLS PIPELINE II=1
		
		for (unsigned short n = 0; n < NUM_STREAMS; n++)
		{
			#pragma HLS UNROLL
			tmp_in[n] = register_it(in_stream[n].read());

#ifdef DEBUG_LOG_PRINT
            printf("==== INTERLEAVE2STREAM READ ====\n");
            printf("Stream[%u], Iteration[%u]\n", n, itr);
            printf("Values (float): (");
            for (unsigned int j = 0; j < MEM_DATA_WIDTH_IN / (DEBUG_LOG_SIZE_OF * 8); j++) {
                DataConv conv;
                conv.i = tmp_in[n].range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
                if (j > 0) printf(", ");
                printf("%f", conv.f);
            }
            printf(")\n");
            printf("================================\n");
#endif
		}

		for (unsigned short n = 0; n < NUM_STREAMS_BY_2; n++)
		{
			#pragma HLS UNROLL
			tmp_out[2*n] = tmp_in[2*n].range(MSB_IN, REALISED_OVERLAP_SIZE);
			tmp_out[2*n+1] = tmp_in[2*n+1].range(MSB_OUT,0);
		}

		for (unsigned short n = 0; n < NUM_STREAMS; n++)
		{
			#pragma HLS UNROLL
			out_stream[n].write(tmp_out[n]);

#ifdef DEBUG_LOG_PRINT
            printf("==== INTERLEAVE2STREAM WRITE ====\n");
            printf("Stream[%u], Iteration[%u]\n", n, itr);
            printf("Values (float): (");
            for (unsigned int j = 0; j < MEM_DATA_WIDTH / (DEBUG_LOG_SIZE_OF * 8); j++) {
                DataConv conv;
                conv.i = tmp_out[n].range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
                if (j > 0) printf(", ");
                printf("%f", conv.f);
            }
            printf(")\n");
            printf("=================================\n");
#endif
		}
	}
}

/**
 * @brief 	updateInterleaveBoundaries directly updates the boundary overlap values of an interleaved stream.
 *
 * @details This function replaces the sequence of interleave2stream -> stream2interleave. 
 * It reads data from NUM_STREAMS of interleaved input streams and produces NUM_STREAMS 
 * of interleaved output streams. It passes the core standard data straight through 
 * while extracting the new boundary values from adjacent streams (data_back, data, data_front)
 * to reconstruct the valid overlaps for the next temporal PE.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of the standard memory access (e.g., 64, 128, 256)
 * @tparam DATA_WIDTH Element datawidth
 * @tparam OVERLAP_SIZE Overlap size of the low level tiles
 * @tparam NUM_STREAMS Number of input, output streams
 *
 * @param[in] in_stream - Input streams array of size MEM_DATA_WIDTH + DATA_WIDTH * OVERLAP_SIZE
 * @param[out] out_stream - Output streams array of size MEM_DATA_WIDTH + DATA_WIDTH * OVERLAP_SIZE
 * @param[in] num_pkts - Number of stream packets
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short DATA_WIDTH, unsigned short NUM_STREAMS, unsigned short OVERLAP_SIZE = 1>
void updateInterleaveBoundaries(::hls::stream<ap_uint<MEM_DATA_WIDTH+DATA_WIDTH*OVERLAP_SIZE>> in_stream[NUM_STREAMS], ::hls::stream<ap_uint<MEM_DATA_WIDTH+DATA_WIDTH*OVERLAP_SIZE>> out_stream[NUM_STREAMS], const unsigned int num_pkts) 
{
#ifndef __SYNTHESIS__
	static_assert(NUM_STREAMS % 2 == 0,
			" NUM_STREAMS has to be divisible by 2");
#endif

	constexpr unsigned short REALISED_OVERLAP_SIZE = DATA_WIDTH * OVERLAP_SIZE;
	constexpr unsigned short REALISED_OVERLAP_SIZE_MIN_1 = REALISED_OVERLAP_SIZE - 1;
	
	// Original non-interleaved dimensions
	constexpr unsigned short MSB_IN = MEM_DATA_WIDTH - 1;
	constexpr unsigned short LSB_LAST_OVERLAP_IN = MEM_DATA_WIDTH - REALISED_OVERLAP_SIZE;
	
	// Interleaved dimensions
	constexpr unsigned short MEM_DATA_WIDTH_OUT = MEM_DATA_WIDTH + REALISED_OVERLAP_SIZE;
	constexpr unsigned short MSB_OUT = MEM_DATA_WIDTH_OUT - 1;
	constexpr unsigned short LSB_LAST_OVERLAP_OUT = MEM_DATA_WIDTH_OUT - REALISED_OVERLAP_SIZE;
	
	constexpr unsigned short NUM_STREAMS_BY_2 = NUM_STREAMS >> 1;
	constexpr unsigned short NUM_STREAMS_MIN_1 = NUM_STREAMS - 1;

	const unsigned int num_pkts_plus_1 = register_it(num_pkts + 1);

	// 3-step shift registers mapped to interleaved width
	ap_uint<MEM_DATA_WIDTH_OUT> data_front[NUM_STREAMS], data[NUM_STREAMS], data_back[NUM_STREAMS];
	#pragma HLS ARRAY_PARTITION variable=data_front dim=0 complete
	#pragma HLS ARRAY_PARTITION variable=data dim=0 complete
	#pragma HLS ARRAY_PARTITION variable=data_back dim=0 complete

	for (unsigned int itr = 0; itr < num_pkts_plus_1; itr++) 
	{
		#pragma HLS PIPELINE II=1
		bool read_cond = register_it(itr < num_pkts);
		ap_uint<MEM_DATA_WIDTH_OUT> tmp[NUM_STREAMS];
		#pragma HLS ARRAY_PARTITION variable=tmp dim=0 complete

		if (read_cond) {
			for (unsigned int n = 0; n < NUM_STREAMS; n++)
			{
				#pragma HLS UNROLL
				tmp[n] = in_stream[n].read();	
			}
		}

		for (unsigned int n = 0; n < NUM_STREAMS; n++)
		{
			#pragma HLS UNROLL
			data_back[n] = data[n];
			data[n] = data_front[n];
			data_front[n] =  register_it(tmp[n]);	
		}

		ap_uint<MEM_DATA_WIDTH_OUT> tmp_out[NUM_STREAMS];
		#pragma HLS ARRAY_PARTITION variable=tmp_out dim=0 complete

		if (itr > 0)
		{
			for (unsigned int n = 0; n < NUM_STREAMS_BY_2; n++)
			{
				#pragma HLS UNROLL
				
				// 1. Pass the standard part of the EVEN stream directly through
				tmp_out[2*n].range(MSB_OUT, REALISED_OVERLAP_SIZE) = data[2*n].range(MSB_OUT, REALISED_OVERLAP_SIZE);
				
				// 2. Update the bottom overlap for the EVEN stream
				if (n == 0) {
					// Borrow from the top of the previous iteration's last ODD stream
					tmp_out[2*n].range(REALISED_OVERLAP_SIZE_MIN_1, 0) = data_back[NUM_STREAMS_MIN_1].range(MSB_IN, LSB_LAST_OVERLAP_IN);
				} else {
					// Borrow from the top of the adjacent ODD stream
					tmp_out[2*n].range(REALISED_OVERLAP_SIZE_MIN_1, 0) = data[2*n-1].range(MSB_IN, LSB_LAST_OVERLAP_IN);
				}
				
				// 3. Pass the standard part of the ODD stream directly through
				tmp_out[2*n+1].range(MSB_IN, 0) = data[2*n+1].range(MSB_IN, 0);
				
				// 4. Update the top overlap for the ODD stream
				if (n == NUM_STREAMS_BY_2 - 1) {
					// Borrow from the bottom of the next iteration's first EVEN stream
					tmp_out[2*n+1].range(MSB_OUT, LSB_LAST_OVERLAP_OUT) = data_front[0].range(2 * REALISED_OVERLAP_SIZE - 1, REALISED_OVERLAP_SIZE);
				} else {
					// Borrow from the bottom of the adjacent EVEN stream
					tmp_out[2*n+1].range(MSB_OUT, LSB_LAST_OVERLAP_OUT) = data[2*n+2].range(2 * REALISED_OVERLAP_SIZE - 1, REALISED_OVERLAP_SIZE);
				}
			}

			for (unsigned int n = 0; n < NUM_STREAMS; n++)
			{
				#pragma HLS UNROLL
				out_stream[n].write(tmp_out[n]);
			}
		}
	}
}

static ap_uint<144> commandGen2D(const ap_uint<64>& offset, const ap_uint<16>& stride_x, const ap_uint<16>& size_x,
                const ap_uint<16>& stride_y, const ap_uint<16>& size_y, const ap_uint<16>& avoid_x)
{
	ap_uint<144> command;
    command.range(63,0) = offset;
    command.range(79,64) = stride_x;
    command.range(95,80) = size_x;
    command.range(111,96) = stride_y;
    command.range(127,112) = size_y;
    command.range(143,128) = avoid_x;

#ifdef DEBUG_LOG_PRINT
    printf("|HLS DEBUG_LOG|%s| offset:%llu, stride_x:%u, size_x:%u, stride_y:%u, size_y:%u, avoid_x:%u\n", __func__, 
		(unsigned long long)offset, (unsigned int)stride_x, (unsigned int)size_x, (unsigned int)stride_y, (unsigned int)size_y, (unsigned int)avoid_x);
#endif
    return command;
}

static ap_uint<128> commandGen2D(const ap_uint<64>& offset, const ap_uint<16>& stride_x, const ap_uint<16>& size_x,
                const ap_uint<16>& stride_y, const ap_uint<16>& size_y)
{
	ap_uint<128> command;
    command.range(63,0) = offset;
    command.range(79,64) = stride_x;
    command.range(95,80) = size_x;
    command.range(111,96) = stride_y;
    command.range(127,112) = size_y;

#ifdef DEBUG_LOG_PRINT
    printf("|HLS DEBUG_LOG|%s| offset:%llu, stride_x:%u, size_x:%u, stride_y:%u, size_y:%u\n", __func__, 
		(unsigned long long)offset, (unsigned int)stride_x, (unsigned int)size_x, (unsigned int)stride_y, (unsigned int)size_y);
#endif
    return command;
}

static ap_uint<160> commandGen3D(const ap_uint<64>& offset, const ap_uint<16>& stride_x, const ap_uint<16>& size_x,
                const ap_uint<16>& stride_y, const ap_uint<16>& size_y,
                const ap_uint<16>& stride_z, const ap_uint<16>& size_z)
{
	ap_uint<160> command;
    command.range(63,0) = offset;
    command.range(79,64) = stride_x;
    command.range(95,80) = size_x;
    command.range(111,96) = stride_y;
    command.range(127,112) = size_y;
    command.range(143,128) = stride_z;
    command.range(159,144) = size_z;

#ifdef DEBUG_LOG_PRINT
    printf("|HLS DEBUG_LOG|%s| offset:%llu, stride_x:%u, size_x:%u, stride_y:%u, size_y:%u, stride_z:%u, size_z:%u\n", __func__, (unsigned long long)offset, (unsigned int)stride_x, (unsigned int)size_x, (unsigned int)stride_y, (unsigned int)size_y, (unsigned int)stride_z, (unsigned int)size_z);
#endif
    return command;
}

static ap_uint<192> commandGen3D(const ap_uint<64>& offset, const ap_uint<16>& stride_x, const ap_uint<16>& size_x,
                const ap_uint<16>& stride_y, const ap_uint<16>& size_y,
                const ap_uint<16>& stride_z, const ap_uint<16>& size_z,
				const ap_uint<16>& avoid_x, const ap_uint<16>& avoid_y)
{
	ap_uint<192> command;
    command.range(63,0) = offset;
    command.range(79,64) = stride_x;
    command.range(95,80) = size_x;
    command.range(111,96) = stride_y;
    command.range(127,112) = size_y;
    command.range(143,128) = stride_z;
    command.range(159,144) = size_z;
	command.range(175,160) = avoid_x;
	command.range(191,176) = avoid_y;

#ifdef DEBUG_LOG_PRINT
    printf("|HLS DEBUG_LOG|%s| offset:%llu, stride_x:%u, size_x:%u, stride_y:%u, size_y:%u, stride_z:%u, size_z:%u, avoid_x:%u, avoid_y:%u\n", __func__, 
		(unsigned long long)offset, (unsigned int)stride_x, (unsigned int)size_x, (unsigned int)stride_y, (unsigned int)size_y, 
		(unsigned int)stride_z, (unsigned int)size_z, (unsigned int)avoid_x, (unsigned int)avoid_y);
#endif
    return command;
}

template <unsigned short MEM_DATA_WIDTH, unsigned short BURST_SIZE=32, unsigned short IN_ITR=2>
static void tileMem2stream(ap_uint<MEM_DATA_WIDTH>* mem_in, ::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_out, ap_uint<160> command)
{
    ap_uint<64> offset = command.range(63,0);
    ap_uint<16> size_x = command.range(95,80);
    ap_uint<16> stride_y = command.range(111,96);
    ap_uint<16> size_y = command.range(127,112);
    ap_uint<16> stride_z = command.range(143,128);
    ap_uint<16> size_z = command.range(159,144);

    for (ap_uint<16> z = 0; z < size_z; z++)
    {
        ap_uint<64> s3 = offset + z * stride_z;

        for (ap_uint<16> y = 0; y < size_y; y++)
        {
            #pragma HLS PIPELINE
            ap_uint<64> s2 = s3 + y * stride_y;

            #ifdef DEBUG_LOG_PRINT
                printf("|HLS DEBUG_LOG|%s| offset:%llu, size_x:%u, stride_y:%u, size_y:%u, stride_z:%u, size_z:%u z:%u y:%u\n", 
                       __func__, (unsigned long long)s2, (unsigned int)size_x, (unsigned int)stride_y, (unsigned int)size_y, 
                       (unsigned int)stride_z, (unsigned int)size_z, (unsigned int)z, (unsigned int)y);
            #endif
            mem2stream<MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>(mem_in + s2, strm_out, size_x);
        }
    }
}

template <unsigned short MEM_DATA_WIDTH, unsigned short BURST_SIZE=32, unsigned short IN_ITR=2>
static void tileStream2mem(ap_uint<MEM_DATA_WIDTH>* mem_out, ::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in, ap_uint<160> command)
{
    ap_uint<64> offset = command.range(63,0);
    ap_uint<16> size_x = command.range(95,80);
    ap_uint<16> stride_y = command.range(111,96);
    ap_uint<16> size_y = command.range(127,112);
    ap_uint<16> stride_z = command.range(143,128);
    ap_uint<16> size_z = command.range(159,144);

    for (ap_uint<16> z = 0; z < size_z; z++)
    {
        ap_uint<64> s3 = offset + z * stride_z;

        for (ap_uint<16> y = 0; y < size_y; y++)
        {
            #pragma HLS PIPELINE
            ap_uint<64> s2 = s3 + y * stride_y;
            // #ifdef DEBUG_LOG_PRINT
                printf("|HLS DEBUG_LOG|%s| offset:%llu, size_x:%u, stride_y:%u, size_y:%u, stride_z:%u, size_z:%u z:%u y:%u\n", 
                       __func__, (unsigned long long)s2, (unsigned int)size_x, (unsigned int)stride_y, (unsigned int)size_y, 
                       (unsigned int)stride_z, (unsigned int)size_z, (unsigned int)z, (unsigned int)y);
            // #endif
            stream2mem<MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>(mem_out + s2, strm_in, size_x);
        }
    }
}

template <unsigned short MEM_DATA_WIDTH, unsigned short BURST_SIZE=32, unsigned short IN_ITR=2>
static void tileStream2memWithAvoid(ap_uint<MEM_DATA_WIDTH>* mem_out, ::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in, ap_uint<192> command)
{
    ap_uint<64> offset = command.range(63,0);
    ap_uint<16> size_x = command.range(95,80);
    ap_uint<16> stride_y = command.range(111,96);
    ap_uint<16> size_y = command.range(127,112);
    ap_uint<16> stride_z = command.range(143,128);
    ap_uint<16> size_z = command.range(159,144);
	ap_uint<16> avoid_x = command.range(175,160);
	ap_uint<16> avoid_y = command.range(191,176);

    for (ap_uint<16> z = 0; z < size_z; z++)
    {
        ap_uint<64> s3 = offset + z * stride_z;

        for (ap_uint<16> y = 0; y < size_y; y++)
        {
            #pragma HLS PIPELINE
            ap_uint<64> s2 = s3 + y * stride_y;
            #ifdef DEBUG_LOG_PRINT
                printf("|HLS DEBUG_LOG|%s| offset:%llu, size_x:%u, stride_y:%u, size_y:%u, stride_z:%u, size_z:%u avoid_x:%u avoid_y:%u z:%u y:%u\n", 
                       __func__, (unsigned long long)s2, (unsigned int)size_x, (unsigned int)stride_y, (unsigned int)size_y, 
                       (unsigned int)stride_z, (unsigned int)size_z, (unsigned int)avoid_x, (unsigned int)avoid_y, (unsigned int)z, (unsigned int)y);
            #endif
			if (y < avoid_y)
				hlsTerminate<MEM_DATA_WIDTH, IN_ITR>(strm_in, size_x);
			else
            	stream2memWithAvoid<MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>(mem_out + s2, strm_in, size_x, avoid_x);
        }
    }
}
/**
 * @brief 	tileMem2stream reads strided tile data from memory to a stream with tiling and batching support.
 *
 * @details This function reads data from memory according to a tiled memory configuration and writes it to an output stream.
 * It handles multi-dimensional tiling in X, Y, and Z dimensions with support for non-uniform tile sizes at boundaries.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of each memory access (e.g., 64, 128, 256)
 * @tparam BURST_SIZE Burst size for memory transfers (default: 32)
 * @tparam IN_ITR Initiation interval for the pipeline (default: 2)
 *
 * @param[in] mem_in Pointer to the input memory from which tile data will be read
 * @param[out] strm_out Reference to the output HLS stream where tile data will be written
 * @param[in] config Reference to MemConfigTile configuration containing:
 *        - start_offset: Base offset in memory
 *        - end_z, start_z: Z-dimension range
 *        - tile_size_y, last_tile_size_y: Y-tile dimensions
 *        - tile_count_x, tile_count_y: Number of tiles in each dimension
 *        - effective_tile_size_x, effective_tile_size_y: Effective tile dimensions
 *        - grid_xblocks, grid_size_y: Grid dimensions for stride calculation
 *
 * @note Supports partial tiles at boundaries through last_tile_size parameters
 * @note Debug logging available when DEBUG_LOG is defined
 * 
 * @see ops::hls::MemConfigTile
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short BURST_SIZE=32, unsigned short IN_ITR=2>
static void tileMem2stream(ap_uint<MEM_DATA_WIDTH>* mem_in, ::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_out, const ops::hls::MemConfigTile& config)
{
    // #pragma HLS INLINE off
    #ifdef DEBUG_LOG_PRINT
        printf("|HLS DEBUG_LOG|%s| reading tile. tile_start:%d, tile_size:%d\n", __func__, config.start_offset, config.total_size_bytes);
    #endif

    const unsigned short z_diff = config.end_z - config.start_z;
    const unsigned short tile_size_y_mul_z_diff = config.tile_size_y * z_diff;
    const unsigned short last_tile_size_y_mul_z_diff = config.last_tile_size_y * z_diff;

    for (unsigned short tile_y = 0; tile_y < config.tile_count_y; tile_y++)
    {
        const unsigned short tile_size_y = tile_y == (config.tile_count_y -1) ? config.last_tile_size_y : config.tile_size_y;
        const unsigned short realized_tile_size_y_mul_z_diff = tile_y == (config.tile_count_y -1) ? last_tile_size_y_mul_z_diff : tile_size_y_mul_z_diff;
        const unsigned int tile_y_offset = tile_y * config.effective_tile_size_y * config.grid_xblocks;
        // const unsigned int abs_row_id_y_offset = config.tile_count_x * tile_y * tile_size_y_mul_z_diff;

        for (unsigned short tile_x = 0; tile_x < config.tile_count_x; tile_x++)
        {
            const unsigned int abs_row_id_x_offset = tile_x * realized_tile_size_y_mul_z_diff;
            const unsigned int tile_x_offset = tile_x * config.effective_tile_size_x; 

            for (unsigned short k = 0; k < z_diff; k++)
            {
                // const unsigned int abs_row_id_y_offset_k = k * realized_tile_size_y_mul_z_diff;
                const unsigned int k_offset = k * config.grid_xblocks * config.grid_size_y;

                for (unsigned short j = 0; j < tile_size_y; j++)
                {
                    #pragma HLS PIPELINE // TODO: Check whether this is effective or not. Most probaly not required as mem2stream already has PIPELINE pragma inside.
                    const unsigned short tile_size_x = tile_x == (config.tile_count_x -1) ? config.last_tile_size_x : config.tile_size_x;
                    unsigned int offset_1 = config.start_offset + tile_x_offset;
                    unsigned int offset_2 = k_offset + tile_y_offset;
                    unsigned int offset_3 = offset_1 + offset_2;
                    unsigned int j_offset  = j * config.grid_xblocks;
                    unsigned int offset = offset_3 + j_offset; 
                    
                    #ifdef DEBUG_LOG_PRINT
                        printf("|HLS DEBUG_LOG|%s| offset_1:%u offset_2:%u offset_3:%u j_offset:%u offset:%u tile_y:%u tile_x:%u k:%u j:%u tile_size_x:%u\n",
                               __func__,
                               (unsigned int)offset_1,
                               (unsigned int)offset_2,
                               (unsigned int)offset_3,
                               (unsigned int)j_offset,
                               (unsigned int)offset,
                               (unsigned int)tile_y,
                               (unsigned int)tile_x,
                               (unsigned int)k,
                               (unsigned int)j,
                               (unsigned int)tile_size_x);
                    #endif
                    mem2stream<MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>(mem_in + offset, strm_out, tile_size_x);
                }
            }
        }
    }
}

/**
 * @brief 	writes strided tile data from a stream to memory with tiling and batching support.
 *
 * @details This function reads data from an input stream and writes it to memory according to a
 * tiled memory configuration. It handles multi-dimensional tiling in X, Y, and Z dimensions
 * with support for non-uniform tile sizes at boundaries.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of each memory access (e.g., 64, 128, 256)
 * @tparam BURST_SIZE Burst size for memory transfers (default: 32)
 * @tparam IN_ITR Initiation interval for the pipeline (default: 2)
 *
 * @param[in] strm_in Reference to the input HLS stream containing the tile data to be written
 * @param[out] mem_out Pointer to the output memory where tile data will be written
 * @param[in] config Reference to MemConfigTile configuration containing:
 *        - start_offset: Base offset in memory
 *        - end_z, start_z: Z-dimension range
 *        - tile_size_y, last_tile_size_y: Y-tile dimensions
 *        - tile_count_x, tile_count_y: Number of tiles in each dimension
 *        - effective_tile_size_x, effective_tile_size_y: Effective tile dimensions
 *        - grid_xblocks, grid_size_y: Grid dimensions for stride calculation
 *
 * @note Supports partial tiles at boundaries through last_tile_size parameters
 * @note Debug logging available when DEBUG_LOG is defined
 * 
 * @see ops::hls::MemConfigTile
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short BURST_SIZE=32, unsigned short IN_ITR=2>
static void tileStream2mem(ap_uint<MEM_DATA_WIDTH>* mem_out, ::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in, const ops::hls::MemConfigTile& config)
{
    // #pragma HLS INLINE off
    #ifdef DEBUG_LOG_PRINT
        printf("|HLS DEBUG_LOG|%s| writing tile. tile_start:%d, tile_size:%d\n", __func__, config.start_offset, config.total_size_bytes);
    #endif

    const unsigned short z_diff = config.end_z - config.start_z;
    const unsigned short tile_size_y_mul_z_diff = config.tile_size_y * z_diff;
    const unsigned short last_tile_size_y_mul_z_diff = config.last_tile_size_y * z_diff;

    for (unsigned short tile_y = 0; tile_y < config.tile_count_y; tile_y++)
    {
        const unsigned short tile_size_y = tile_y == (config.tile_count_y -1) ? config.last_tile_size_y : config.tile_size_y;
        const unsigned short realized_tile_size_y_mul_z_diff = tile_y == (config.tile_count_y -1) ? last_tile_size_y_mul_z_diff : tile_size_y_mul_z_diff;
        const unsigned int tile_y_offset = tile_y * config.effective_tile_size_y * config.grid_xblocks;
        // const unsigned int abs_row_id_y_offset = config.tile_count_x * tile_y * tile_size_y_mul_z_diff;

        for (unsigned short tile_x = 0; tile_x < config.tile_count_x; tile_x++)
        {
            const unsigned int abs_row_id_x_offset = tile_x * realized_tile_size_y_mul_z_diff;
            const unsigned int tile_x_offset = tile_x * config.effective_tile_size_x; 

            for (unsigned short k = 0; k < z_diff; k++)
            {
                // const unsigned int abs_row_id_y_offset_k = k * realized_tile_size_y_mul_z_diff;
                const unsigned int k_offset = k * config.grid_xblocks * config.grid_size_y;

                for (unsigned short j = 0; j < tile_size_y; j++)
                {
                    #pragma HLS PIPELINE // TODO: Check whether this is effective or not. Most probaly not required as mem2stream already has PIPELINE pragma inside.
                    const unsigned short tile_size_x = tile_x == (config.tile_count_x -1) ? config.last_tile_size_x : config.tile_size_x;
                    unsigned int offset_1 = config.start_offset + tile_x_offset;
                    unsigned int offset_2 = k_offset + tile_y_offset;
                    unsigned int offset_3 = offset_1 + offset_2;
                    unsigned int j_offset  = j * config.grid_xblocks;
                    unsigned int offset = offset_3 + j_offset;
                    #ifdef DEBUG_LOG_PRINT
                        printf("|HLS DEBUG_LOG|%s| offset_1:%u offset_2:%u offset_3:%u j_offset:%u offset:%u tile_y:%u tile_x:%u k:%u j:%u tile_size_x:%u\n",
                               __func__,
                               (unsigned int)offset_1,
                               (unsigned int)offset_2,
                               (unsigned int)offset_3,
                               (unsigned int)j_offset,
                               (unsigned int)offset,
                               (unsigned int)tile_y,
                               (unsigned int)tile_x,
                               (unsigned int)k,
                               (unsigned int)j,
                               (unsigned int)tile_size_x);
                    #endif
                    stream2mem<MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>(mem_out + offset, strm_in, tile_size_x);
                }
            }
        }
    }
}

/**
 * @brief Reads strided tile data from a specific memory bank to a stream with 2D grid support (X-axis tiling only).
 *
 * @details This function acts as an optimized Address Generation Unit (AGU) for a specific physical memory bank.
 * In a 2D grid layout, the memory region is tiled strictly along the X-axis, while the Y-axis is fully 
 * traversed row-by-row. By utilizing the templated `BANK_ID`, the function statically computes `total_rows`—the 
 * exact subset of global Y-rows that reside physically within this specific interleaved bank. This design 
 * guarantees that the inner loop traverses only valid local rows, entirely eliminating loop overhead, pipeline 
 * bubbles, and runtime modulo operations.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of each memory access (e.g., 64, 128, 256).
 * @tparam NUM_BANKS The total number of interleaved memory banks in the system. Must be a power of two.
 * @tparam BANK_ID The logical ID of the reading bank. Used at compile-time to determine local row bounds.
 * @tparam BURST_SIZE Maximum AXI burst size for memory transfers (default: 32).
 * @tparam IN_ITR Initiation interval for the pipeline (default: 2).
 *
 * @param[in] mem_in Pointer to the specific physical AXI memory bank where tile data is read from.
 * @param[out] strm_out Reference to the output HLS stream passing data to the compute pipeline.
 * @param[in] config Reference to MemConfigTile configuration containing:
 * - start_offset: Base offset in memory
 * - tile_size_y: Total size of the Y dimension (treated as full grid_size_y in 2D)
 * - tile_count_x: Number of tiles in the X dimension
 * - effective_tile_size_x: Effective X-tile dimension for stride offset calculation
 * - grid_xblocks: Grid dimension for contiguous memory stride calculation
 *
 * @note Supports non-uniform tile sizes at the grid boundaries via `last_tile_size_x`.
 * @note Debug logging is actively supported when `DEBUG_LOG_PRINT` is defined.
 *
 * @see ops::hls::MemConfigTile
 * @see ops::hls::mem2streamV2
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short NUM_BANKS=2, unsigned short BANK_ID=0, unsigned short BURST_SIZE=32, unsigned short IN_ITR=2>
static void stridedTileMem2stream2D(ap_uint<MEM_DATA_WIDTH>* mem_in, ::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_out, const ops::hls::MemConfigTile& config)
{

	constexpr unsigned short BANK_SHIFT = LOG2(NUM_BANKS);
	constexpr unsigned short BANK_MASK = NUM_BANKS - 1;
	const bool is_additional_row = (config.grid_size_y & BANK_MASK) > BANK_ID;
	const unsigned short total_rows = (config.grid_size_y >> BANK_SHIFT) + (is_additional_row ? 1 : 0);

    #ifdef DEBUG_LOG_PRINT
    printf("|HLS DEBUG_LOG|%s|bank-%u| Initializing 2D read (X-tiling only). start_offset:%u, total_size_bytes:%u, tile_count_x:%u, total_y_rows:%u, bank_y_rows:%u, is_additional_row: %u\n", 
           __func__, BANK_ID, config.start_offset, config.total_size_bytes, config.tile_count_x, config.grid_size_y, total_rows, is_additional_row);
    #endif

    for (unsigned short tile_x = 0; tile_x < config.tile_count_x; tile_x++)
    {
        const unsigned int tile_x_offset = tile_x * config.effective_tile_size_x; 
        const unsigned short tile_size_x = tile_x == (config.tile_count_x -1) ? config.last_tile_size_x : config.tile_size_x;

        // Traverse the entire Y dimension, dynamically striding by NUM_BANKS
        for (unsigned short j = 0; j < total_rows; j++)
        {
            unsigned int offset_1 = config.start_offset + tile_x_offset;
            unsigned int j_offset  = j * config.grid_xblocks;
            unsigned int offset = offset_1 + j_offset;

        #ifdef DEBUG_LOG_PRINT
            printf("|HLS DEBUG_LOG|%s|bank-%u| offset_1:%u j_offset:%u final_offset:%u | tile_x_id:%u j_row:%u tile_size_x:%u\n",
                        __func__, BANK_ID, offset_1, j_offset, offset, (unsigned int)tile_x, (unsigned int)j, (unsigned int)tile_size_x);
        #endif
            mem2streamV2<MEM_DATA_WIDTH, IN_ITR>((ap_uint<MEM_DATA_WIDTH>* )(mem_in + offset), strm_out, tile_size_x);
        }
    }
}

/**
 * @brief Writes strided tile data from memory to stream with 3D tiling support.
 *
 * @details This function acts as an optimized Address Generation Unit (AGU) for a specific physical memory bank 
 * within a 3D tiled layout. Instead of striding across a global Y dimension, it utilizes the templated `BANK_ID` 
 * to statically resolve the physical Y-bounds per bank on a tile-by-tile basis. This ensures the inner loops 
 * traverse only the rows resident in this specific memory bank for the current `tile_y`, avoiding runtime 
 * modulo operations and eliminating loop overhead.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of each memory access (e.g., 64, 128, 256).
 * @tparam NUM_BANKS The total number of interleaved memory banks. Must be a power of two.
 * @tparam BANK_ID The logical ID of the reading bank. Used at compile-time to determine local row bounds.
 * @tparam BURST_SIZE Burst size for memory transfers (default: 32).
 * @tparam IN_ITR Initiation interval for the pipeline (default: 2).
 *
 * @param[in] mem_in Pointer to the specific physical AXI memory bank where tile data is read from.
 * @param[out] strm_out Reference to the output HLS stream passing data to the compute pipeline.
 * @param[in] config Reference to MemConfigTile configuration containing:
 * - start_offset: Base offset in memory
 * - start_z, end_z: Z-dimension iteration range
 * - tile_size_y, last_tile_size_y: Y-tile dimensions
 * - tile_count_x, tile_count_y: Number of tiles in each dimension
 * - effective_tile_size_x, effective_tile_size_y: Effective dimensions for offset calculation
 * - grid_xblocks, grid_size_y: Grid dimensions for stride calculation
 *
 * @note Supports non-uniform tile sizes at the grid boundaries via `last_tile_size_x` and `last_tile_size_y`.
 * @note Debug logging is actively supported when `DEBUG_LOG_PRINT` is defined.
 *
 * @see ops::hls::MemConfigTile
 * @see ops::hls::stream2mem
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short NUM_BANKS=2, unsigned short BANK_ID=0, unsigned short BURST_SIZE=32, unsigned short IN_ITR=2>
static void stridedTileMem2stream3D(ap_uint<MEM_DATA_WIDTH>* mem_in, ::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_out, const ops::hls::MemConfigTile& config)
{
    constexpr unsigned short BANK_SHIFT = LOG2(NUM_BANKS);
    constexpr unsigned short BANK_MASK = NUM_BANKS - 1;
    
    // Overall bank Y size is used to calculate the Z stride within the local memory
    const bool is_additional_row = (config.grid_size_y & BANK_MASK) > BANK_ID;
    const unsigned short bank_grid_size_y = (config.grid_size_y >> BANK_SHIFT) + (is_additional_row ? 1 : 0);
    const unsigned int bank_z_stride = bank_grid_size_y * config.grid_xblocks;
    const unsigned short z_diff = config.end_z - config.start_z;

    #ifdef DEBUG_LOG_PRINT
    printf("|HLS DEBUG_LOG|%s|bank-%u| Initializing 3D read. start_offset:%u, tile_count_y:%u, tile_count_x:%u, z_diff:%u, bank_z_stride:%u\n", 
           __func__, BANK_ID, config.start_offset, config.tile_count_y, config.tile_count_x, z_diff, bank_z_stride);
    #endif

    for (unsigned short tile_y = 0; tile_y < config.tile_count_y; tile_y++)
    {
        const unsigned short tile_size_y = (tile_y == config.tile_count_y - 1) ? config.last_tile_size_y : config.tile_size_y;
        
        // Use ap_uint wrapping to find the specific offset for this BANK_ID relative to the tile's starting bank
        unsigned int global_y_start = tile_y * config.effective_tile_size_y;
        ap_uint<BANK_SHIFT> start_bank = global_y_start;
        ap_uint<BANK_SHIFT> bank_offset_in_tile = BANK_ID - start_bank; 
        
        const unsigned short remainder = tile_size_y & BANK_MASK;
        const bool tile_is_additional_row = bank_offset_in_tile < remainder;
        const unsigned short local_tile_size_y = (tile_size_y >> BANK_SHIFT) + (tile_is_additional_row ? 1 : 0);
        
        // Physical offset for this tile in the specific memory bank's Y dimension
        const unsigned int local_tile_y_offset = ((global_y_start + bank_offset_in_tile) >> BANK_SHIFT) * config.grid_xblocks;
        
        for (unsigned short tile_x = 0; tile_x < config.tile_count_x; tile_x++)
        {
            const unsigned int tile_x_offset = tile_x * config.effective_tile_size_x; 
            const unsigned short tile_size_x = (tile_x == config.tile_count_x -1) ? config.last_tile_size_x : config.tile_size_x;

            for (unsigned short k = 0; k < z_diff; k++)
            {
                const unsigned int k_offset = k * bank_z_stride;

                for (unsigned short j = 0; j < local_tile_size_y; j++)
                {
                    unsigned int offset_1 = config.start_offset + tile_x_offset;
                    unsigned int offset_2 = k_offset + local_tile_y_offset;
                    unsigned int j_offset = j * config.grid_xblocks;
                    unsigned int offset = offset_1 + offset_2 + j_offset;

                #ifdef DEBUG_LOG_PRINT
                    printf("|HLS DEBUG_LOG|%s|bank-%u| offset_1:%u offset_2:%u j_offset:%u final_offset:%u | tile_y:%u tile_x:%u k:%u j:%u tile_size_x:%u\n",
                                __func__, BANK_ID, offset_1, offset_2, j_offset, offset, (unsigned int)tile_y, (unsigned int)tile_x, (unsigned int)k, (unsigned int)j, (unsigned int)tile_size_x);
                #endif
                    mem2streamV2<MEM_DATA_WIDTH, IN_ITR>((ap_uint<MEM_DATA_WIDTH>* )(mem_in + offset), strm_out, tile_size_x);
                }
            }
        }
    }
}

/**
 * @brief readConfigStreamGenerator: Generates and distributes bank-specific configuration commands 
 * across multiple output streams.
 *
 * @details This function decodes a single 160-bit configuration command containing 3D memory 
 * access parameters (strides, sizes, and offsets). It iterates through the Z and Y dimensions 
 * to compute absolute memory offsets. Utilizing power-of-two bitwise optimizations, it 
 * calculates the base bank offset and starting bank, handles modulo wrap-around logic, and 
 * pushes appropriate 48-bit sub-commands to an array of hardware streams. The 
 * distribution logic is fully unrolled for optimal HLS synthesis.
 *
 * @tparam NUM_BANKS The number of target memory banks. Must be a power of two to enable 
 * bitwise masking optimizations.
 * @tparam IN_ITR Initiation interval for the inner HLS pipeline (default: 2).
 *
 * @param[in] command A 160-bit packed configuration word containing:
 * - [63:0]   offset_x: Base offset in the X dimension
 * - [95:80]  size_x:   Size in the X dimension
 * - [111:96] stride_y: Stride in the Y dimension
 * - [127:112] size_y:   Size in the Y dimension
 * - [143:128] stride_z: Stride in the Z dimension
 * - [159:144] size_z:   Size in the Z dimension
 * @param[out] strms Array of output HLS streams (hls::stream<ap_uint<48>>). 
 * Each 48-bit command contains:
 * - [31:0]   bank_offset: The starting offset for that specific bank
 * - [47:32]  b_size_x:    The calculated X dimension size for that specific bank
 *
 * @note Enforces a compile-time static assertion that `NUM_BANKS` is a power of two, 
 * replacing expensive hardware modulo operations with bitwise AND masking.
 */
template <unsigned short NUM_BANKS, unsigned short IN_ITR=2>
static void readConfigStreamGenerator(const size_t offset_x, const unsigned short size_x, const unsigned short stride_y, const unsigned short size_y, const unsigned short stride_z, const unsigned short size_z, ::hls::stream<ap_uint<48>> strms[NUM_BANKS])
{
#ifndef __SYNTHESIS__
	static_assert((NUM_BANKS != 0) && ((NUM_BANKS & (NUM_BANKS - 1)) == 0), "NUM_BANKS must be a power of two");
#endif 

	constexpr unsigned short NUM_BANKS_SHIFT = LOG2(NUM_BANKS);
	constexpr unsigned short BANK_MASK = NUM_BANKS - 1;

	// size_t offset_x = command.range(63,0);
	// ap_uint<16> size_x = command.range(95,80);
    // ap_uint<16> stride_y = command.range(111,96);
    // ap_uint<16> size_y = command.range(127,112);
    // ap_uint<16> stride_z = command.range(143,128);
    // ap_uint<16> size_z = command.range(159,144);

#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| command parsed. offset_x:%llu, size_x:%u, stride_y:%u, size_y:%u, stride_z:%u, size_z:%u\n", 
			__func__, (unsigned long long)offset_x, (unsigned int)size_x, (unsigned int)stride_y, 
			(unsigned int)size_y, (unsigned int)stride_z, (unsigned int)size_z);
#endif

	const unsigned short size_x_div_by_banks_floor = size_x >> NUM_BANKS_SHIFT;
	const unsigned int size_x_mod_num_banks = size_x & BANK_MASK;

	for (unsigned short z = 0; z < size_z; z++)
    {
        for (unsigned short y = 0; y < size_y; y++)
        {
//          #pragma HLS PIPELINE II = IN_ITR

            size_t z_offset = offset_x + z * stride_z;
            size_t abs_offset = z_offset + y * stride_y;
            
            const unsigned int bank_offset = abs_offset >> NUM_BANKS_SHIFT;
            const unsigned int starting_bank = abs_offset & BANK_MASK; //Equivalent to abs_offset % NUM_BANKS
            
            // Optimized wrap-around calculation
            const unsigned int banks_upper_limit = (starting_bank + size_x_mod_num_banks) & BANK_MASK;  //Equivalent to (...) % NUM_BANKS
            const bool is_t1_or_t2 = banks_upper_limit < starting_bank;

#ifdef DEBUG_LOG_PRINT
			printf("|HLS DEBUG_LOG|%s| z:%u, y:%u, abs_offset:%llu, bank_offset:%u, starting_bank:%u\n", 
					__func__, (unsigned int)z, (unsigned int)y, (unsigned long long)abs_offset, 
					(unsigned int)bank_offset, (unsigned int)starting_bank);
#endif

            for (unsigned int i = 0; i < NUM_BANKS; i++)
            {
                #pragma HLS UNROLL

                const unsigned int current_bank_offset = (i < starting_bank) ? bank_offset + 1 : bank_offset;

                const bool is_gt_sb = (i >= starting_bank);
                const bool is_lt_uplim = (i < banks_upper_limit);

                bool cond;
                if (is_t1_or_t2) {
                    cond = is_gt_sb || is_lt_uplim;
                } else {
                    cond = is_gt_sb && is_lt_uplim;
                }

                const unsigned char add_arg = cond ? 1 : 0;
                unsigned short b_size_x = size_x_div_by_banks_floor + add_arg;

                ap_uint<48> b_command;
                b_command.range(31,0) = current_bank_offset;
                b_command.range(47,32) = b_size_x;

#ifdef DEBUG_LOG_PRINT
				printf("|HLS DEBUG_LOG|%s| bank_idx:%u, current_bank_offset:%u, b_size_x:%u\n", 
						__func__, (unsigned int)i, (unsigned int)current_bank_offset, (unsigned int)b_size_x);
#endif

                strms[i] << b_command;
            }
        }
    }
}


// /**
//  * @brief readConfigStreamGeneratorV2: Generates and distributes bank-specific configuration commands 
//  * across multiple output streams.
//  *
//  * @details This function decodes a single 160-bit configuration command containing 3D memory 
//  * access parameters (strides, sizes, and offsets). It iterates through the Z and Y dimensions 
//  * to compute absolute memory offsets. Utilizing power-of-two bitwise optimizations, it 
//  * calculates the base bank offset and starting bank, handles modulo wrap-around logic, and 
//  * pushes appropriate 48-bit sub-commands to an array of hardware streams. The 
//  * distribution logic is fully unrolled for optimal HLS synthesis.
//  *
//  * @tparam NUM_BANKS The number of target memory banks. Must be a power of two to enable 
//  * bitwise masking optimizations.
//  * @tparam IN_ITR Initiation interval for the inner HLS pipeline (default: 2).
//  *
//  * @param[in] command A 160-bit packed configuration word containing:
//  * - [63:0]   offset_x: Base offset in the X dimension
//  * - [95:80]  size_x:   Size in the X dimension
//  * - [111:96] stride_y: Stride in the Y dimension
//  * - [127:112] size_y:   Size in the Y dimension
//  * - [143:128] stride_z: Stride in the Z dimension
//  * - [159:144] size_z:   Size in the Z dimension
//  * @param[out] strms Array of output HLS streams (hls::stream<ap_uint<48>>). 
//  * Each 48-bit command contains:
//  * - [31:0]   bank_offset: The starting offset for that specific bank
//  * - [47:32]  b_size_x:    The calculated X dimension size for that specific bank
//  *
//  * @note Enforces a compile-time static assertion that `NUM_BANKS` is a power of two, 
//  * replacing expensive hardware modulo operations with bitwise AND masking.
//  */
// template <unsigned short NUM_BANKS, unsigned short IN_ITR = 2, typename... Flags>
// static void readConfigStreamGeneratorV2(
//     const unsigned short bank_offset_x_floor, 
//     const unsigned short bank_tile_size_x_floor, 
// 	const unsigned short bank_grid_size_x_floor,
//     ap_uint<LOG2(NUM_BANKS)> bank_offset_x_upper_bank, //This is the starting bank
//     const unsigned short size_y, 
// 	const unsigned short bank_offest_y_floor,
// 	const unsigned short abs_row_offset,
//     const unsigned short size_z, 
// 	const unsigned short bank_stride_z_floor,
//     ::hls::stream<ap_uint<48>> strms[NUM_BANKS], 
//     Flags... flags)
// {
// #ifndef __SYNTHESIS__
// 	static_assert((NUM_BANKS != 0) && ((NUM_BANKS & (NUM_BANKS - 1)) == 0), "NUM_BANKS must be a power of two");
// 	static_assert(sizeof...(Flags) == NUM_BANKS, "Error: The number of flag arguments must match NUM_BANKS.");
// #endif 

// 	constexpr unsigned short NUM_BANKS_SHIFT = LOG2(NUM_BANKS);
// 	constexpr unsigned short BANK_MASK = NUM_BANKS - 1;

// 	ap_uint<2> flag_array[NUM_BANKS] = { static_cast<ap_uint<2>>(flags)... };
// 	#pragma HLS ARRAY_PARTITION variable=flag_array complete

// 	unsigned int initial_bank_offsets[NUM_BANKS];
//     #pragma HLS ARRAY_PARTITION variable=initial_bank_offsets complete dim=1
    
//     unsigned short banks_grid_size_x[NUM_BANKS];
//     #pragma HLS ARRAY_PARTITION variable=banks_grid_size_x complete dim=1

//     unsigned short bank_stride_z[NUM_BANKS];
//     #pragma HLS ARRAY_PARTITION variable=bank_stride_z complete dim=1

// 	for (ap_uint<NUM_BANKS_SHIFT+1> b_id = 0; b_id < NUM_BANKS; b_id++) {
//         #pragma HLS UNROLL
// 		const unsigned short bank_offset_y = bank_offest_y_floor + (flag_array[b_id].range(0,0) ? abs_row_offset : 0);
// 		initial_bank_offsets[b_id] = bank_offset_x_floor + (b_id < bank_offset_x_upper_bank ? 1 : 0) + bank_offset_y;
// 		banks_grid_size_x[b_id] = bank_grid_size_x_floor + flag_array[b_id].range(0,0);
// 		bank_stride_z[b_id] = bank_stride_z_floor + (uint16_t)(is_big_grid_size_x ? grid_size_y : 0);

// #ifdef DEBUG_LOG
//         printf("|HLS DEBUG_LOG|%s| bank_id: %u, init_offset: %u, banks_grid_size_x: %u, bank_stride_z: %u\n", 
//                __func__, (unsigned int)b_id, initial_bank_offsets[b_id], banks_grid_size_x[b_id], bank_stride_z[b_id]);
// #endif
//     }
// 	// size_t offset_x = command.range(63,0);
// 	// ap_uint<16> size_x = command.range(95,80);
//     // ap_uint<16> stride_y = command.range(111,96);
//     // ap_uint<16> size_y = command.range(127,112);
//     // ap_uint<16> stride_z = command.range(143,128);
//     // ap_uint<16> size_z = command.range(159,144);

// #ifdef DEBUG_LOG_PRINT
// 	printf("|HLS DEBUG_LOG|%s| command parsed. offset_x:%llu, size_x:%u, stride_y:%u, size_y:%u, stride_z:%u, size_z:%u\n", 
// 			__func__, (unsigned long long)offset_x, (unsigned int)size_x, (unsigned int)stride_y, 
// 			(unsigned int)size_y, (unsigned int)stride_z, (unsigned int)size_z);
// #endif

// 	// const unsigned short size_x_div_by_banks_floor = size_x >> NUM_BANKS_SHIFT;
// 	// const unsigned int size_x_mod_num_banks = size_x & BANK_MASK;

// 	for (unsigned short z = 0; z < size_z; z++)
//     {
//         for (unsigned short y = 0; y < size_y; y++)
//         {
//          #pragma HLS LOOP_FLATTEN

//             size_t z_offset = offset_x + z * stride_z;
//             size_t abs_offset = z_offset + y * stride_y;
            
//             const unsigned int bank_offset = abs_offset >> NUM_BANKS_SHIFT;
//             const unsigned int starting_bank = abs_offset & BANK_MASK; //Equivalent to abs_offset % NUM_BANKS
            
//             // Optimized wrap-around calculation
//             const unsigned int banks_upper_limit = (starting_bank + size_x_mod_num_banks) & BANK_MASK;  //Equivalent to (...) % NUM_BANKS
//             const bool is_t1_or_t2 = banks_upper_limit < starting_bank;

// #ifdef DEBUG_LOG_PRINT
// 			printf("|HLS DEBUG_LOG|%s| z:%u, y:%u, abs_offset:%llu, bank_offset:%u, starting_bank:%u\n", 
// 					__func__, (unsigned int)z, (unsigned int)y, (unsigned long long)abs_offset, 
// 					(unsigned int)bank_offset, (unsigned int)starting_bank);
// #endif

//             for (unsigned int i = 0; i < NUM_BANKS; i++)
//             {
//                 #pragma HLS UNROLL

//                 const unsigned int current_bank_offset = (i < starting_bank) ? bank_offset + 1 : bank_offset;

//                 const bool is_gt_sb = (i >= starting_bank);
//                 const bool is_lt_uplim = (i < banks_upper_limit);

//                 bool cond;
//                 if (is_t1_or_t2) {
//                     cond = is_gt_sb || is_lt_uplim;
//                 } else {
//                     cond = is_gt_sb && is_lt_uplim;
//                 }

//                 const unsigned char add_arg = cond ? 1 : 0;
//                 unsigned short b_size_x = size_x_div_by_banks_floor + add_arg;

//                 ap_uint<48> b_command;
//                 b_command.range(31,0) = current_bank_offset;
//                 b_command.range(47,32) = b_size_x;

// #ifdef DEBUG_LOG_PRINT
// 				printf("|HLS DEBUG_LOG|%s| bank_idx:%u, current_bank_offset:%u, b_size_x:%u\n", 
// 						__func__, (unsigned int)i, (unsigned int)current_bank_offset, (unsigned int)b_size_x);
// #endif

//                 strms[i] << b_command;
//             }
//         }
//     }
// }

/**
 * @brief offsetGenerator: Generates and distributes bank-specific 32-bit memory offsets 
 * across multiple output streams for a 3D memory access pattern.
 *
 * @details This function calculates absolute memory offsets based on pre-computed 3D spatial 
 * parameters (tile sizes, grid sizes, and base offsets) provided as individual arguments. 
 * It iterates through the Z and Y dimensions to compute the absolute memory offsets. Utilizing 
 * power-of-two bitwise optimizations, it handles base bank offsets and wrap-around logic, 
 * pushing the appropriate 32-bit offsets to an array of hardware streams. The distribution 
 * logic is fully unrolled for optimal HLS synthesis.
 *
 * @tparam NUM_BANKS The number of target memory banks. Must be a power of two to enable 
 * bitwise masking optimizations.
 * @tparam IN_ITR Initiation interval for the inner HLS pipeline (default: 2).
 *
 * @param[in] tile_size_y Size of the tile in the Y dimension.
 * @param[in] tile_size_z Size of the tile in the Z dimension.
 * @param[in] bank_grid_size_x_floor Base grid size in the X dimension per bank.
 * @param[in] big_grid_size_x_banks_upper Number of lower-indexed banks that receive an extra X grid element.
 * @param[in] grid_size_y Total grid size in the Y dimension.
 * @param[in] tile_offset_x_floor Base offset in the X dimension per bank.
 * @param[in] big_offset_x_banks_upper Number of lower-indexed banks that receive an extra X offset element.
 * @param[in] tile_offset_y Base offset in the Y dimension.
 * @param[in] bank_tile_size_x_floor Base tile size in the X dimension per bank.
 * @param[in] big_bank_tile_size_x_mask Bitmask indicating which banks receive an extra X tile element.
 * @param[out] offset_strm Array of output HLS streams (hls::stream<ap_uint<32>>). 
 * Each push contains the 32-bit absolute memory offset for that specific bank.
 *
 * @note Enforces a compile-time static assertion that `NUM_BANKS` is a power of two, 
 * replacing expensive hardware modulo operations with bitwise AND masking.
 */
template <unsigned short NUM_BANKS, unsigned short IN_ITR=2>
static void offsetGenerator(const unsigned short tile_size_y, const unsigned short tile_size_z, 
		const unsigned short bank_grid_size_x_floor, const ap_uint<LOG2(NUM_BANKS)> big_grid_size_x_banks_upper, const unsigned short grid_size_y, 
		const unsigned short tile_offset_x_floor, const ap_uint<LOG2(NUM_BANKS)> big_offset_x_banks_upper,  const unsigned short tile_offset_y, const unsigned short bank_tile_size_x_floor, const ap_uint<NUM_BANKS> big_bank_tile_size_x_mask,
		::hls::stream<ap_uint<32>> offset_strm[NUM_BANKS])
{
#ifndef __SYNTHESIS__
	static_assert((NUM_BANKS != 0) && ((NUM_BANKS & (NUM_BANKS - 1)) == 0), "NUM_BANKS must be a power of two");
#endif 

	constexpr unsigned short NUM_BANKS_SHIFT = LOG2(NUM_BANKS);
	constexpr unsigned short BANK_MASK = NUM_BANKS - 1;

	// size_t offset_x = command.range(63,0);
	// ap_uint<16> size_x = command.range(95,80);
    // ap_uint<16> stride_y = command.range(111,96);
    // ap_uint<16> size_y = command.range(127,112);
    // ap_uint<16> stride_z = command.range(143,128);
    // ap_uint<16> size_z = command.range(159,144);

#ifdef DEBUG_LOG_PRINT
    printf("|HLS DEBUG_LOG|%s| Initial Parameters. tile_size_y:%u, tile_size_z:%u, bank_grid_size_x_floor:%u, big_grid_size_x_banks_upper:%u,"\
		 	"grid_size_y:%u, tile_offset_x_floor:%u, big_offset_x_banks_upper:%u, tile_offset_y:%u\n", 
            __func__, (unsigned int)tile_size_y, (unsigned int)tile_size_z, (unsigned int)bank_grid_size_x_floor, 
            (unsigned int)big_grid_size_x_banks_upper, (unsigned int)grid_size_y, (unsigned int)tile_offset_x_floor, 
            (unsigned int)big_offset_x_banks_upper, (unsigned int)tile_offset_y);
#endif

	// const unsigned short size_x_div_by_banks_floor = size_x >> NUM_BANKS_SHIFT;
	// const unsigned int size_x_mod_num_banks = size_x & BANK_MASK;
	unsigned int initial_bank_offsets[NUM_BANKS];
	#pragma HLS ARRAY_PARTITION variable=initial_bank_offsets complete

	unsigned short grid_size_x_banks[NUM_BANKS]; //equivalent to stride_y
	#pragma HLS ARRAY_PARTITION variable=grid_size_x_banks complete

	unsigned short bank_stride_z[NUM_BANKS]; 
	#pragma HLS ARRAY_PARTITION variable=bank_stride_z complete

	unsigned short bank_tile_size_x[NUM_BANKS];
	#pragma HLS ARRAY_PARTITION variable=bank_tile_size_x complete

	for (ap_uint<NUM_BANKS_SHIFT+1> b_id = 0; b_id < NUM_BANKS; b_id++)
	{
		#pragma HLS UNROLL
		grid_size_x_banks[b_id] = bank_grid_size_x_floor + (b_id < big_grid_size_x_banks_upper ? 1 : 0);
		initial_bank_offsets[b_id] = tile_offset_x_floor + (b_id < big_offset_x_banks_upper ? 1 : 0);
		initial_bank_offsets[b_id] += tile_offset_y * grid_size_x_banks[b_id];
		bank_stride_z[b_id] = grid_size_x_banks[b_id] * grid_size_y;
		bank_tile_size_x[b_id] = bank_tile_size_x_floor + big_bank_tile_size_x_mask[b_id];

#ifdef DEBUG_LOG_PRINT
                printf("|HLS DEBUG_LOG|%s| bank_id: %u, grid_size_x_banks: %u, initial_bank_offsets: %u: bank_stride_z: %u\n", 
                        __func__, b_id, grid_size_x_banks[b_id], initial_bank_offsets[b_id], bank_stride_z[b_id]);
#endif
	}


	for (unsigned short z = 0; z < tile_size_z; z++)
    {
        for (unsigned short y = 0; y < tile_size_y; y++)
        {
			#pragma HLS LOOP_FLATTEN 
//          #pragma HLS PIPELINE II = IN_ITR
			for (ap_uint<NUM_BANKS_SHIFT+1> b_id = 0; b_id < NUM_BANKS; b_id++)
			{
				#pragma HLS UNROLL
				const unsigned int abs_bank_offset = initial_bank_offsets[b_id] + y * grid_size_x_banks[b_id] + z * bank_stride_z[b_id];

#ifdef DEBUG_LOG_PRINT
                printf("|HLS DEBUG_LOG|%s| Inner Loop. z:%u, y:%u, b_id:%u, abs_bank_offset:%u\n", 
                        __func__, (unsigned int)z, (unsigned int)y, (unsigned int)b_id, abs_bank_offset);
#endif
				if (bank_tile_size_x[b_id] != 0)
					offset_strm[b_id].write(abs_bank_offset);
			}
        }
    }
}

/**
 * @brief offsetGenerator: Generates and distributes bank-specific 32-bit memory offsets 
 * across multiple output streams for a 2D memory access pattern.
 *
 * @details This function calculates absolute memory offsets based on pre-computed 2D spatial 
 * parameters (tile sizes, grid sizes, and base offsets) provided as individual arguments. 
 * It iterates through the Y dimension to compute the absolute memory offsets. Utilizing 
 * power-of-two bitwise optimizations, it handles base bank offsets and wrap-around logic, 
 * pushing the appropriate 32-bit offsets to an array of hardware streams. The distribution 
 * logic is fully unrolled for optimal HLS synthesis.
 *
 * @tparam NUM_BANKS The number of target memory banks. Must be a power of two to enable 
 * bitwise masking optimizations.
 * @tparam IN_ITR Initiation interval for the inner HLS pipeline (default: 2).
 *
 * @param[in] tile_size_y Size of the tile in the Y dimension.
 * @param[in] bank_grid_size_x_floor Base grid size in the X dimension per bank.
 * @param[in] big_grid_size_x_banks_upper Number of lower-indexed banks that receive an extra X grid element.
 * @param[in] tile_offset_x_floor Base offset in the X dimension per bank.
 * @param[in] big_offset_x_banks_upper Number of lower-indexed banks that receive an extra X offset element.
 * @param[in] bank_tile_size_x_floor Base tile size in the X dimension per bank.
 * @param[in] big_bank_tile_size_x_mask Bitmask indicating which banks receive an extra X tile element.
 * @param[out] offset_strm Array of output HLS streams (hls::stream<ap_uint<32>>). 
 * Each push contains the 32-bit absolute memory offset for that specific bank.
 *
 * @note Enforces a compile-time static assertion that `NUM_BANKS` is a power of two, 
 * replacing expensive hardware modulo operations with bitwise AND masking.
 */
template <unsigned short NUM_BANKS, unsigned short IN_ITR=2>
static void offsetGenerator(const unsigned short tile_size_y, 
        const unsigned short bank_grid_size_x_floor, const ap_uint<LOG2(NUM_BANKS)> big_grid_size_x_banks_upper, 
        const unsigned short tile_offset_x_floor, const ap_uint<LOG2(NUM_BANKS)> big_offset_x_banks_upper,  
        const unsigned short bank_tile_size_x_floor, 
        const ap_uint<NUM_BANKS> big_bank_tile_size_x_mask,
        ::hls::stream<ap_uint<32>> offset_strm[NUM_BANKS])
{
#ifndef __SYNTHESIS__
    static_assert((NUM_BANKS != 0) && ((NUM_BANKS & (NUM_BANKS - 1)) == 0), "NUM_BANKS must be a power of two");
#endif 

    constexpr unsigned short NUM_BANKS_SHIFT = LOG2(NUM_BANKS);
    constexpr unsigned short BANK_MASK = NUM_BANKS - 1;

    // size_t offset_x = command.range(63,0);
    // ap_uint<16> size_x = command.range(95,80);
    // ap_uint<16> stride_y = command.range(111,96);
    // ap_uint<16> size_y = command.range(127,112);

#ifdef DEBUG_LOG_PRINT
    printf("|HLS DEBUG_LOG|%s| Initial Parameters. tile_size_y:%u, bank_grid_size_x_floor:%u, big_grid_size_x_banks_upper:%u,"
            "tile_offset_x_floor:%u, big_offset_x_banks_upper:%u\n", 
            __func__, (unsigned int)tile_size_y, (unsigned int)bank_grid_size_x_floor, 
            (unsigned int)big_grid_size_x_banks_upper, (unsigned int)tile_offset_x_floor, 
            (unsigned int)big_offset_x_banks_upper);
#endif

    unsigned int initial_bank_offsets[NUM_BANKS];
    #pragma HLS ARRAY_PARTITION variable=initial_bank_offsets complete

    unsigned short grid_size_x_banks[NUM_BANKS]; // equivalent to stride_y
    #pragma HLS ARRAY_PARTITION variable=grid_size_x_banks complete

    unsigned short bank_tile_size_x[NUM_BANKS];
    #pragma HLS ARRAY_PARTITION variable=bank_tile_size_x complete

    for (ap_uint<NUM_BANKS_SHIFT+1> b_id = 0; b_id < NUM_BANKS; b_id++)
    {
        #pragma HLS UNROLL
        grid_size_x_banks[b_id] = bank_grid_size_x_floor + (b_id < big_grid_size_x_banks_upper ? 1 : 0);
        initial_bank_offsets[b_id] = tile_offset_x_floor + (b_id < big_offset_x_banks_upper ? 1 : 0);
        bank_tile_size_x[b_id] = bank_tile_size_x_floor + big_bank_tile_size_x_mask[b_id];

#ifdef DEBUG_LOG_PRINT
        printf("|HLS DEBUG_LOG|%s| bank_id: %u, grid_size_x_banks: %u, initial_bank_offsets: %u\n", 
                __func__, (unsigned int)b_id, grid_size_x_banks[b_id], initial_bank_offsets[b_id]);
#endif
    }

    // Single loop for 2D, looping over Y dimension only
    for (unsigned short y = 0; y < tile_size_y; y++)
    {
        #pragma HLS PIPELINE II = IN_ITR
        for (ap_uint<NUM_BANKS_SHIFT+1> b_id = 0; b_id < NUM_BANKS; b_id++)
        {
            #pragma HLS UNROLL
            const unsigned int abs_bank_offset = initial_bank_offsets[b_id] + y * grid_size_x_banks[b_id];

#ifdef DEBUG_LOG_PRINT
            printf("|HLS DEBUG_LOG|%s| Inner Loop. y:%u, b_id:%u, abs_bank_offset:%u\n", 
                    __func__, (unsigned int)y, (unsigned int)b_id, abs_bank_offset);
#endif
            if (bank_tile_size_x[b_id] != 0)
                offset_strm[b_id].write(abs_bank_offset);
        }
    }
}

/**
 * @brief writeConfigStreamGenerator: Generates and distributes bank-specific configuration commands 
 * across multiple output streams, accounting for avoidance zones.
 *
 * @details This function decodes a single 192-bit configuration command containing 3D memory 
 * access parameters alongside boundary avoidance data (strides, sizes, offsets, avoid_x, and avoid_y). 
 * It iterates through the Z and Y dimensions to compute absolute memory offsets. Utilizing power-of-two 
 * bitwise optimizations, it calculates the base bank offset and starting bank, handles modulo 
 * wrap-around logic, and pushes appropriate 64-bit sub-commands to an array of hardware streams. 
 * The distribution logic is fully unrolled for optimal HLS synthesis.
 *
 * @tparam NUM_BANKS The number of target memory banks. Must be a power of two to enable 
 * bitwise masking optimizations.
 * @tparam IN_ITR Initiation interval for the inner HLS pipeline (default: 2).
 *
 * @param[in] command A 192-bit packed configuration word containing:
 * - [63:0]   offset_x: Base offset in the X dimension
 * - [95:80]  size_x:   Size in the X dimension
 * - [111:96] stride_y: Stride in the Y dimension
 * - [127:112] size_y:   Size in the Y dimension
 * - [143:128] stride_z: Stride in the Z dimension
 * - [159:144] size_z:   Size in the Z dimension
 * - [175:160] avoid_x:  Avoid size in X dimension
 * - [191:176] avoid_y:  Avoid size in Y dimension
 * @param[out] strms Array of output HLS streams (hls::stream<ap_uint<64>>). 
 * Each 64-bit command contains:
 * - [31:0]   bank_offset: The starting offset for that specific bank
 * - [47:32]  b_size_x:    The calculated X dimension size for that specific bank
 * - [63:48]  b_avoid_x:   The calculated avoid_x size for that specific bank
 *
 * @note Enforces a compile-time static assertion that `NUM_BANKS` is a power of two, 
 * replacing expensive hardware modulo operations with bitwise AND masking.
 */
template <unsigned short NUM_BANKS, unsigned short IN_ITR=2>
static void writeConfigStreamGenerator(const size_t offset_x, const unsigned short size_x, const unsigned short stride_y, const unsigned short size_y, const unsigned short stride_z, const unsigned short size_z, const unsigned short avoid_x, const unsigned short avoid_y, ::hls::stream<ap_uint<64>> strms[NUM_BANKS])
{
#ifndef __SYNTHESIS__
    static_assert((NUM_BANKS != 0) && ((NUM_BANKS & (NUM_BANKS - 1)) == 0), "NUM_BANKS must be a power of two");
#endif 

    constexpr unsigned short NUM_BANKS_SHIFT = LOG2(NUM_BANKS);
    constexpr unsigned short BANK_MASK = NUM_BANKS - 1;

    // ap_uint<64> offset_x = command.range(63,0);
    // ap_uint<16> size_x = command.range(95,80);
    // ap_uint<16> stride_y = command.range(111,96);
    // ap_uint<16> size_y = command.range(127,112);
    // ap_uint<16> stride_z = command.range(143,128);
    // ap_uint<16> size_z = command.range(159,144);
    // ap_uint<16> avoid_x = command.range(175,160);
    // ap_uint<16> avoid_y = command.range(191,176);

#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| command parsed. offset_x:%llu, size_x:%u, stride_y:%u, size_y:%u, stride_z:%u, size_z:%u, avoid_x:%u, avoid_y:%u\n", 
			__func__, (unsigned long long)offset_x, (unsigned int)size_x, (unsigned int)stride_y, 
			(unsigned int)size_y, (unsigned int)stride_z, (unsigned int)size_z, (unsigned int)avoid_x, (unsigned int)avoid_y);
#endif

    const unsigned short avoid_x_per_bank = avoid_x >> NUM_BANKS_SHIFT;
    const ap_uint<3> avoid_x_mod_banks = avoid_x & BANK_MASK;

    const unsigned short size_x_div_by_banks_floor = size_x >> NUM_BANKS_SHIFT;
    const unsigned int size_x_mod_num_banks = size_x & BANK_MASK;

    for (unsigned short z = 0; z < size_z; z++)
    {
        for (unsigned short y = 0; y < size_y; y++)
        {
//          #pragma HLS PIPELINE II = IN_ITR

            size_t z_offset = offset_x + z * stride_z;
            size_t abs_offset = z_offset + y * stride_y;
            
            const unsigned int bank_offset = abs_offset >> NUM_BANKS_SHIFT;
            const unsigned int starting_bank = abs_offset & BANK_MASK; //Equivalent to abs_offset % NUM_BANKS
            
#ifdef DEBUG_LOG_PRINT
			printf("|HLS DEBUG_LOG|%s| z:%u, y:%u, abs_offset:%llu, bank_offset:%u, starting_bank:%u\n", 
					__func__, (unsigned int)z, (unsigned int)y, (unsigned long long)abs_offset, 
					(unsigned int)bank_offset, (unsigned int)starting_bank);
#endif
            // Optimized wrap-around calculation
            const unsigned int banks_upper_limit = (starting_bank + size_x_mod_num_banks) & BANK_MASK;  //Equivalent to (...) % NUM_BANKS
            const bool is_t1_or_t2 = banks_upper_limit < starting_bank;

            const ap_uint<3> avoid_x_bank_upper_limit = starting_bank + avoid_x_mod_banks;
            const bool is_t1_or_t2_avoid_x = avoid_x_bank_upper_limit < starting_bank;

            for (unsigned int i = 0; i < NUM_BANKS; i++)
            {
                #pragma HLS UNROLL

                const unsigned int current_bank_offset = (i < starting_bank) ? bank_offset + 1 : bank_offset;

                const bool is_gt_sb = (i >= starting_bank);
                const bool is_lt_uplim = (i < banks_upper_limit);
                const bool is_avoid_lt_uplim = (i < avoid_x_bank_upper_limit);

                bool cond;
                if (is_t1_or_t2) {
                    cond = is_gt_sb || is_lt_uplim;
                } else {
                    cond = is_gt_sb && is_lt_uplim;
                }
                
                bool avoid_cond;
                if (is_t1_or_t2_avoid_x) {
                    avoid_cond = is_gt_sb || is_avoid_lt_uplim;
                } else {
                    avoid_cond = is_gt_sb && is_avoid_lt_uplim;
                }

                const unsigned char add_arg = cond ? 1 : 0;
                const unsigned char add_avoid_arg = avoid_cond ? 1 : 0;
                unsigned short b_size_x = size_x_div_by_banks_floor + add_arg;
                unsigned short adj_avoid_x = avoid_x_per_bank + add_avoid_arg;
                unsigned short b_avoid_x = y < avoid_y ? b_size_x : adj_avoid_x;
                b_avoid_x = b_avoid_x > b_size_x ? b_size_x : b_avoid_x;

                ap_uint<64> b_command;
                b_command.range(31,0) = current_bank_offset;
                b_command.range(47,32) = b_size_x;
                
                // Fixed out-of-bounds range mapping (64 is invalid for ap_uint<64>)
                b_command.range(63,48) = b_avoid_x; 

#ifdef DEBUG_LOG_PRINT
				printf("|HLS DEBUG_LOG|%s| bank_idx:%u, current_bank_offset:%u, b_size_x:%u, b_avoid_x:%u\n", 
						__func__, (unsigned int)i, (unsigned int)current_bank_offset, (unsigned int)b_size_x, (unsigned int)b_avoid_x);
#endif
                strms[i] << b_command;
            }
        }
    }
}

/**
 * @brief stridedTileMem2streamV2 Writes strided tile data from memory to stream with tiling from the externel config
 * 		generator
 *
 * @details This function reads data from memory writes it to an output stream to according to a
 * command feeding from a dataflow command generator. For each rows new command will be feed from the generator
 *
 * @tparam MEM_DATA_WIDTH The bit-width of each memory access (e.g., 64, 128, 256)
 * @tparam BURST_SIZE Burst size for memory transfers (default: 32)
 * @tparam IN_ITR Initiation interval for the pipeline (default: 2)
 *
 * @param[in] mem_in Pointer to the input memory where tile data is read from
 * @param[in] strm_command Simple command stream with offset and size information to be
 * 			read from the memory.
 * @param[out] strm_out Reference to the output HLS stream that the tile data will be converted to
 *
 * @note Supports partial tiles at boundaries through last_tile_size parameters
 * @note Debug logging available when DEBUG_LOG is defined
 * 
 * @see ops::hls::ReadConfigStreamGenerator
 * @see ops::hls::stream2mem
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short BURST_SIZE=32, unsigned short IN_ITR=2>
static void stridedTileMem2streamV2(ap_uint<MEM_DATA_WIDTH>* mem_in, ::hls::stream<ap_uint<48>>& strm_command,
		::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_out, unsigned short size_y, unsigned short size_z=1
#ifdef DEBUG_LOG_PRINT
		, const char* print_prompt = ""
#endif	
	)
{
	const unsigned int total_iters = size_z * size_y;

	for (unsigned int i = 0; i < total_iters; i++) {
		auto command = strm_command.read();
		unsigned int bank_offset = command.range(31,0);
		unsigned short size_x = command.range(47,32);
		#ifdef DEBUG_LOG_PRINT
			printf("|HLS DEBUG_LOG|%s|%s| reading tile from mem to stream, bank_offset:%d, size_x:%d \n", __func__, print_prompt, bank_offset, size_x);
		#endif
		ops::hls::mem2stream<MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>(mem_in + bank_offset, strm_out, size_x);

	}
}

template <unsigned short MEM_DATA_WIDTH, unsigned short AXIS_DATA_WIDTH, unsigned short BURST_SIZE=32, unsigned short IN_ITR=2>
static void stridedTileMem2streamV2(ap_uint<MEM_DATA_WIDTH>* mem_in, ::hls::stream<ap_uint<48>>& strm_command,
		::hls::stream<ap_uint<AXIS_DATA_WIDTH>>& strm_out, unsigned short size_y, unsigned short size_z=1)
{
	const unsigned int total_iters = size_z * size_y;

	for (unsigned int i = 0; i < total_iters; i++) {
//			#pragma HLS PIPELINE II = IN_ITR
		auto command = strm_command.read();
		unsigned int bank_offset = command.range(31,0);
		unsigned short size_x = command.range(47,32);
		#ifdef DEBUG_LOG_PRINT
			printf("|HLS DEBUG_LOG|%s| reading tile from mem to stream, bank_offset:%d, size_x:%d \n", __func__, bank_offset, size_x);
		#endif
		ops::hls::mem2stream<MEM_DATA_WIDTH, AXIS_DATA_WIDTH, BURST_SIZE, IN_ITR>(mem_in + bank_offset, strm_out, size_x);

	}
}

/**
 * @brief stridedTileMem2streamV3 Writes strided tile data from memory to stream seamlessly.
 *
 * @details This is a standalone, optimized version that natively handles AXI4 burst 
 * inference using a single inner loop, avoiding Vitis HLS loop flattening warnings.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of each memory access (e.g., 64, 128, 256)
 * @tparam BURST_SIZE Burst size for memory transfers (kept for API compatibility)
 * @tparam IN_ITR Initiation interval for the pipeline (default: 2)
 *
 * @param[in] mem_in Pointer to the input memory where tile data is read from
 * @param[in] strm_command Simple command stream with offset and size information
 * @param[out] strm_out Reference to the output HLS stream
 * @param[in] size_y Number of rows
 * @param[in] size_z Depth (default: 1)
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short BURST_SIZE=32, unsigned short IN_ITR=2>
static void stridedTileMem2streamV3(ap_uint<MEM_DATA_WIDTH>* mem_in, 
                                    ::hls::stream<ap_uint<48>>& strm_command,
                                    ::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_out, 
                                    unsigned short size_y, 
                                    unsigned short size_z=1
#ifdef DEBUG_LOG_PRINT
                                    , const char* print_prompt = ""
#endif  
    )
{
    const unsigned int total_iters = size_z * size_y;

    tile_loop: for (unsigned int i = 0; i < total_iters; i++) {

		auto command = strm_command.read();
        unsigned int bank_offset = command.range(31,0);
        unsigned short 	size_x = command.range(47,32);

#ifdef DEBUG_LOG_PRINT
		printf("|HLS DEBUG_LOG|%s|%s| reading tile from mem to stream, bank_offset:%d, size_x:%d \n", 
				__func__, print_prompt, bank_offset, size_x);
#endif
        read_burst_loop: for (unsigned short j = 0; j < size_x; j++) {
			#pragma HLS LOOP_FLATTEN off
            #pragma HLS PIPELINE II=IN_ITR
            

            ap_uint<MEM_DATA_WIDTH> tmp = mem_in[bank_offset + j];
            strm_out << tmp;

#ifdef DEBUG_LOG_PRINT
		printf("====================================================================================\n");
        printf("|HLS DEBUG_LOG| stridedTileMem2streamV3 | reading i: %d, j: %d, val=(\n", i, j);

        for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
        {
            DataConv conv;
            conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
            printf("		%f,\n", conv.f);
        }
        printf(")\n\n");
		printf("====================================================================================\n");
#endif
        }
    }
}

/**
 * @brief stridedTileMem2streamV4 Writes strided tile data from memory to stream seamlessly.
 *
 * @details This is a standalone, optimized version that natively handles AXI4 burst 
 * inference using a single inner loop, avoiding Vitis HLS loop flattening warnings.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of each memory access (e.g., 64, 128, 256)
 * @tparam BURST_SIZE Burst size for memory transfers (kept for API compatibility)
 * @tparam IN_ITR Initiation interval for the pipeline (default: 2)
 *
 * @param[in] mem_in Pointer to the input memory where tile data is read from
 * @param[in] strm_command Simple command stream with offset and size information
 * @param[out] strm_out Reference to the output HLS stream
 * @param[in] size_y Number of rows
 * @param[in] size_z Depth (default: 1)
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short NUM_BANKS, unsigned short BANK_ID, unsigned short BURST_SIZE=32, unsigned short IN_ITR=2>
static void stridedTileMem2streamV4(ap_uint<MEM_DATA_WIDTH>* mem_in, 
                                    ::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_out, 
									::hls::stream<ap_uint<32>>& offset_command,
                                    const unsigned short bank_tile_size_x_floor,
									const ap_uint<NUM_BANKS> big_bank_tile_size_x_mask,
									const unsigned int total_rows
									// const unsigned short tile_size_y,
                                    // const unsigned short tile_size_z=1
#ifdef DEBUG_LOG_PRINT
                                    , const char* print_prompt = ""
#endif  
    )
{
	constexpr unsigned short BURST_SIZE_SHIFT = LOG2(BURST_SIZE);
	constexpr unsigned short BURST_SIZE_MASK = BURST_SIZE - 1;
    // const unsigned int total_iters = size_z * size_y;
	const unsigned short bank_size_x =  bank_tile_size_x_floor + big_bank_tile_size_x_mask.range(BANK_ID, BANK_ID);
	// const unsigned short burst_beats = bank_size_x >> BURST_SIZE_SHIFT;
	// const unsigned short non_burst_beats = bank_size_x & BURST_SIZE_MASK;
	// const bool is_burst_beats_zero = burst_beats == 0;

#ifdef DEBUG_LOG_PRINT
				printf("|HLS DEBUG_LOG|%s|%s| reading tile from mem to stream, total_rows:%d, bank_size_x:%d \n", 
						__func__, print_prompt, total_rows, bank_size_x);
#endif

	unsigned int bank_offset = 0;

    tile_loop: for (unsigned int j = 0; j < total_rows; j++) {
		read_burst_loop: for (unsigned short i = 0; i < bank_size_x; i++) {
			#pragma HLS LOOP_FLATTEN
			#pragma HLS PIPELINE II=IN_ITR

			if (i == 0) {
				bank_offset = offset_command.read();
			}
			ap_uint<MEM_DATA_WIDTH> tmp = mem_in[bank_offset + i];
			strm_out << tmp;

	#ifdef DEBUG_LOG_PRINT
		printf("====================================================================================\n");
		printf("|HLS DEBUG_LOG| stridedTileMem2streamV4 | reading  i: %d, j: %d, bank_offset: %d, val=(\n", i, j, bank_offset);

		for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
		{
			DataConv conv;
			conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
			printf("		%f,\n", conv.f);
		}
		printf(")\n\n");
		printf("====================================================================================\n");
	#endif
		}
		// read_non_burst_loop: for (unsigned int i = 0; i < non_burst_beats; i++) {
		// 	if (i == 0 and is_burst_beats_zero) {
		// 		bank_offset = offset_command.read();
		// 	}
		// 	ap_uint<MEM_DATA_WIDTH> tmp = mem_in[bank_offset + i];
		// 	strm_out << tmp;
		// }
    }
}


/**
 * @brief stridedTileMem2streamV5 Writes strided tile data from memory to stream seamlessly.
 *
 * @details This is a standalone, optimized version that natively handles AXI4 burst 
 * inference using a single inner loop, avoiding Vitis HLS loop flattening warnings.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of each memory access (e.g., 64, 128, 256)
 * @tparam BURST_SIZE Burst size for memory transfers (kept for API compatibility)
 * @tparam IN_ITR Initiation interval for the pipeline (default: 2)
 *
 * @param[in] mem_in Pointer to the input memory where tile data is read from
 * @param[in] strm_command Simple command stream with offset and size information
 * @param[out] strm_out Reference to the output HLS stream
 * @param[in] size_y Number of rows
 * @param[in] size_z Depth (default: 1)
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short NUM_BANKS, unsigned short BANK_ID, unsigned short BURST_SIZE=32, unsigned short IN_ITR=2>
static void stridedTileMem2streamV5(ap_uint<MEM_DATA_WIDTH>* mem_in, 
		::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_out, 
		const unsigned short bank_size_x_floor,
		const ap_uint<1> is_big_size_x,
		const unsigned short bank_grid_size_x_floor, 
		const ap_uint<LOG2(NUM_BANKS)> big_grid_size_x_banks_upper, 
		const unsigned short tile_offset_x_floor, 
		const ap_uint<LOG2(NUM_BANKS)> big_offset_x_banks_upper,  
		const unsigned short tile_size_y, 
		const unsigned short grid_size_y, 
		const unsigned short tile_offset_y=0,
		const unsigned short tile_size_z=1
#ifdef DEBUG_LOG_PRINT
                                    , const char* print_prompt = ""
#endif  
    )
{
	const unsigned short bank_size_x =  bank_size_x_floor + is_big_size_x;
	const unsigned short bank_grid_size_x = bank_grid_size_x_floor + (BANK_ID < big_grid_size_x_banks_upper ? 1 : 0);
	const unsigned short init_bank_offset_x = tile_offset_x_floor + (BANK_ID < big_offset_x_banks_upper ? 1 : 0);
	const unsigned short init_bank_offset_y = tile_offset_y * bank_grid_size_x;
	const unsigned short init_bank_offest = init_bank_offset_x + init_bank_offset_y;
	const unsigned short bank_stride_z = bank_grid_size_x * grid_size_y;
	unsigned int bank_offset = 0;

#ifdef DEBUG_LOG_PRINT
    printf("|HLS DEBUG_LOG|%s-bank:%d|%s| Input Params:- bank_size_x_floor:%u, is_big_size_x:%u, tile_size_y:%u, tile_size_z:%u, bank_grid_size_x_floor:%u, big_grid_size_x_banks_upper:%u, grid_size_y:%u, tile_offset_x_floor:%u, big_offset_x_banks_upper:%u, tile_offset_y:%u\n",
            __func__, BANK_ID, print_prompt, bank_size_x_floor, (unsigned)is_big_size_x, tile_size_y, tile_size_z, bank_grid_size_x_floor, (unsigned)big_grid_size_x_banks_upper, grid_size_y, tile_offset_x_floor, (unsigned)big_offset_x_banks_upper, tile_offset_y);
    printf("|HLS DEBUG_LOG|%s-bank:%d|%s| Internal Params:-  bank_size_x:%u, bank_grid_size_x:%u, init_bank_offset_x:%u, init_bank_offset_y:%u, init_bank_offset:%u, bank_stride_z:%u\n",
            __func__, BANK_ID, print_prompt, bank_size_x, bank_grid_size_x, init_bank_offset_x, init_bank_offset_y, init_bank_offest, bank_stride_z);
#endif


    tile_loop1: for (unsigned short z = 0; z < tile_size_z; z++)
    {
        tile_loop2: for (unsigned short y = 0; y < tile_size_y; y++)
        {
			read_burst_loop: for (unsigned short j = 0; j < bank_size_x; j++) 
			{
				#pragma HLS LOOP_FLATTEN
				#pragma HLS PIPELINE II=IN_ITR

				if (j == 0){
					bank_offset = init_bank_offest + y * bank_grid_size_x + z * bank_stride_z;
				} else {
					bank_offset += 1;
				}
				ap_uint<MEM_DATA_WIDTH> tmp = mem_in[bank_offset];
				strm_out << tmp;

#ifdef DEBUG_LOG_PRINT
				printf("====================================================================================\n");
				printf("|HLS DEBUG_LOG| stridedTileMem2streamV4 | reading  i: %d, j: %d, k: %d, bank_offset: %d, val=(\n", j, y, z, bank_offset);

				for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
				{
					DataConv conv;
					conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
					printf("		%f,\n", conv.f);
				}
				printf(")\n\n");
				printf("====================================================================================\n");
#endif
			}
		}
    }
}

/**
 * @brief Writes a 3D strided tile from a hardware stream to memory, incorporating boundary avoidance.
 *
 * @details This function acts as a datamover sink within a task-parallel dataflow architecture. 
 * It iterates over the Z and Y dimensions of a structured mesh domain. For each row (Y iteration), 
 * it consumes a 64-bit configuration command to dynamically determine the absolute memory offset, 
 * the write length (`size_x`), and the boundary avoidance zone (`avoid_x`) for the target memory bank. 
 * The actual burst-aligned memory write operations are delegated to `ops::hls::stream2memWithAvoid`.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of the AXI memory interface and the input data stream.
 * @tparam BURST_SIZE The maximum AXI burst length for the memory transactions (default: 32).
 * @tparam IN_ITR The targeted Initiation Interval (II) for the underlying pipeline (default: 2).
 *
 * @param[in] strm_in The input HLS stream containing the computed tile data to be written.
 * @param[out] out Pointer to the mapped target memory bank (typically an AXI4 master interface).
 * @param[in] strm_command The HLS stream providing bank-specific 64-bit configuration commands.
 * Expected bit-packing per command:
 * 			- [31:0]  bank_offset: The absolute base offset in memory for the current row.
 * 			- [47:32] size_x:      The total number of elements to process in the X dimension.
 * 			- [63:48] avoid_x:     The number of boundary elements to avoid writing.
 * @param[in] size_y The total number of iterations in the Y dimension.
 * @param[in] size_z The total number of iterations in the Z dimension.
 *
 * @note The expected format of `strm_command` directly corresponds to the stream outputs 
 * @see generated by the `writeConfigStreamGenerator` utility.
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short BURST_SIZE=32, unsigned short IN_ITR=2>
static void stridedTileStream2memWithAvoidV2(::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in, ap_uint<MEM_DATA_WIDTH>* out, ::hls::stream<ap_uint<64>>& strm_command, unsigned short size_y, unsigned short size_z=1
#ifdef DEBUG_LOG_PRINT
		, const char* print_prompt = ""
#endif	
)
{
    const unsigned int total_iters = size_z * size_y;

	for (unsigned int i = 0; i < total_iters; i++) {
//          #pragma HLS PIPELINE II = IN_ITR
		auto command = strm_command.read();
		unsigned int bank_offset = command.range(31,0);
		unsigned short size_x = command.range(47,32);
		unsigned short avoid_x = command.range(63, 48);
		#ifdef DEBUG_LOG_PRINT
			printf("|HLS DEBUG_LOG|%s|%s| reading tile from mem to stream, bank_offset:%d, size_x:%d \n", __func__, print_prompt, bank_offset, size_x);
		#endif
		ops::hls::stream2memWithAvoid<MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>(out + bank_offset, strm_in, size_x, avoid_x);
        
    }
}

/**
 * @brief Writes a 3D strided tile from a hardware stream to memory, incorporating boundary avoidance.
 *
 * @details This is a standalone, optimized version (V3) that eliminates Vitis HLS loop flattening 
 * warnings by unifying the stream reads, discards, and memory writes into a single inner loop.
 * It dynamically reads a 64-bit command to determine the absolute memory offset, write length (`size_x`), 
 * and boundary avoidance zone (`avoid_x`).
 *
 * @tparam MEM_DATA_WIDTH The bit-width of the AXI memory interface and the input data stream.
 * @tparam BURST_SIZE The maximum AXI burst length (kept for API compatibility).
 * @tparam IN_ITR The targeted Initiation Interval (II) for the underlying pipeline (default: 2).
 *
 * @param[in] strm_in The input HLS stream containing the computed tile data to be written.
 * @param[out] out Pointer to the mapped target memory bank (typically an AXI4 master interface).
 * @param[in] strm_command The HLS stream providing bank-specific 64-bit configuration commands.
 * - [31:0]  bank_offset: The absolute base offset in memory for the current row.
 * - [47:32] size_x:      The total number of elements to process in the X dimension.
 * - [63:48] avoid_x:     The number of boundary elements to avoid writing.
 * @param[in] size_y The total number of iterations in the Y dimension.
 * @param[in] size_z The total number of iterations in the Z dimension.
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short BURST_SIZE=32, unsigned short IN_ITR=2>
static void stridedTileStream2memWithAvoidV3(
    ::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in, 
    ap_uint<MEM_DATA_WIDTH>* out, 
    ::hls::stream<ap_uint<64>>& strm_command, 
    unsigned short size_y, 
    unsigned short size_z=1
#ifdef DEBUG_LOG_PRINT
    , const char* print_prompt = ""
#endif  
)
{
    const unsigned int total_iters = size_z * size_y;

    tile_loop: for (unsigned int i = 0; i < total_iters; i++) {

		auto command = strm_command.read();;
		unsigned int bank_offset = command.range(31,0);
		unsigned short size_x = command.range(47,32);
		unsigned short avoid_x = command.range(63, 48);

#ifdef DEBUG_LOG_PRINT
		printf("|HLS DEBUG_LOG|%s|%s| reading stream to mem, bank_offset:%d, size_x:%d, avoid_x:%d \n", 
				__func__, print_prompt, bank_offset, size_x, avoid_x);
#endif

        // A single unified loop prevents loop flattening warnings. 
        write_burst_loop: for (unsigned short j = 0; j < size_x; j++) {
			#pragma HLS LOOP_FLATTEN off
            #pragma HLS PIPELINE II=IN_ITR

            // Always read from the stream to drain it properly
            ap_uint<MEM_DATA_WIDTH> tmp = strm_in.read();
            
            // Only write to memory if we have passed the avoid_x threshold.
            if (j >= avoid_x) {
                out[bank_offset + j] = tmp;
            }

#ifdef DEBUG_LOG_PRINT
            printf("====================================================================================\n");
            
            // Identify if we are in the avoid zone or actively writing
            if (j < avoid_x) {
                printf("|HLS DEBUG_LOG| %s | SKIPPING (Avoid Zone) stream beat: %d, val=(\n", __func__, j);
            } else {
                printf("|HLS DEBUG_LOG| %s | WRITING stream beat: %d to mem offset: %d, val=(\n", __func__, j, bank_offset + j);
            }

            // Print the payload using the original DataConv logic
            for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
            {
                DataConv conv;
                conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
                printf("     %f,\n", conv.f);
            }
            printf(")\n\n");
            printf("====================================================================================\n");
#endif
        }
    }
}

/**
 * @brief stridedTileStream2memWithAvoidV4: Writes a 3D strided tile from a hardware stream to memory, incorporating boundary avoidance.
 *
 * @details This is a standalone, optimized version (V4) that eliminates Vitis HLS loop flattening 
 * warnings by unifying the stream reads, discards, and memory writes into a single inner loop.
 * It uses statically provided bank configuration masks and sizes, reading a 32-bit base memory offset 
 * from a command stream at the start of each row. It actively drains the input stream but conditionally 
 * avoids writing the data to memory if the current indices fall within the `avoid_x` or `avoid_y` boundary zones.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of the AXI memory interface and the input data stream.
 * @tparam NUM_BANKS The total number of memory banks.
 * @tparam BANK_ID The specific ID of the target memory bank.
 * @tparam BURST_SIZE The maximum AXI burst length (kept for API compatibility).
 * @tparam IN_ITR The targeted Initiation Interval (II) for the underlying pipeline (default: 2).
 *
 * @param[in] strm_in The input HLS stream containing the computed tile data to be written.
 * @param[out] out Pointer to the mapped target memory bank (typically an AXI4 master interface).
 * @param[in] offset_command The HLS stream providing the 32-bit absolute base offset in memory for the current row.
 * @param[in] bank_tile_size_x_floor Base tile size in the X dimension per bank.
 * @param[in] big_bank_tile_size_x_mask Bitmask indicating which banks receive an extra X tile element.
 * @param[in] bank_avoid_x_floor Base number of boundary elements to avoid writing in X per bank.
 * @param[in] big_bank_avoid_x_mask Bitmask for extra boundary elements to avoid in X.
 * @param[in] avoid_y The number of initial iterations in the Y dimension to skip writing to memory.
 * @param[in] size_y The total number of iterations in the Y dimension.
 * @param[in] size_z The total number of iterations in the Z dimension (default: 1).
 */

template <unsigned short MEM_DATA_WIDTH, unsigned short NUM_BANKS, unsigned short BANK_ID, unsigned short BURST_SIZE=32, unsigned short IN_ITR=2>
static void stridedTileStream2memWithAvoidV4(
    	::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in, 
		ap_uint<MEM_DATA_WIDTH>* out, 
		::hls::stream<ap_uint<32>>& offset_command, 
		const unsigned short bank_tile_size_x_floor,
		const ap_uint<NUM_BANKS> big_bank_tile_size_x_mask,
		const unsigned short bank_avoid_x_floor,
		const ap_uint<NUM_BANKS> big_bank_avoid_x_mask,
		const unsigned short avoid_y,
		const unsigned short size_y,
		const unsigned short size_z=1
#ifdef DEBUG_LOG_PRINT
		, const char* print_prompt = ""
#endif  
)
{
	constexpr unsigned short BURST_SIZE_SHIFT = LOG2(BURST_SIZE);
	constexpr unsigned short BURST_SIZE_MASK = BURST_SIZE - 1;

    // const unsigned int total_iters = size_z * size_y;
	const unsigned short bank_size_x = bank_tile_size_x_floor + big_bank_tile_size_x_mask.range(BANK_ID, BANK_ID);
	const unsigned short avoid_x = bank_avoid_x_floor + big_bank_avoid_x_mask(BANK_ID, BANK_ID);
	// const unsigned short avoid_iterations = bank_size_x * avoid_y;
	// const unsigned short size_y_mask = size_y - 1;
	// const unsigned short burst_beats = bank_size_x >> BURST_SIZE_SHIFT;
	// const unsigned short non_burst_beats = bank_size_x & BURST_SIZE_MASK;
	// const bool is_burst_beats_zero = burst_beats == 0;
	unsigned int bank_offset = 0;

#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s|%s| reading stream to mem -  bank_size_x:%d, avoid_x:%d \n", 
			__func__, print_prompt, bank_size_x, avoid_x);
#endif

    tile_loop_1: for (unsigned short z = 0; z < size_z; z++) {
		tile_loop_2: for (unsigned short y = 0; y < size_y; y++) {
			write_burst_loop: for (unsigned short j = 0; j < bank_size_x; j++) {
				#pragma HLS LOOP_FLATTEN
				#pragma HLS PIPELINE II=IN_ITR
				
				if (j == 0) {
					bank_offset = offset_command.read();
				}
				// Always read from the stream to drain it properly
				ap_uint<MEM_DATA_WIDTH> tmp = strm_in.read();
				
				// Only write to memory if we have passed the avoid_x threshold.
				if (j >= avoid_x and y >= avoid_y) {
					out[bank_offset + j] = tmp;
				}

	#ifdef DEBUG_LOG_PRINT
				printf("====================================================================================\n");
				printf("|HLS DEBUG_LOG|%s|%s| writing burst  x: %d, y: %d, z:%d \n",__func__ , print_prompt, j, y, z);
				// Identify if we are in the avoid zone or actively writing
				if (j < avoid_x) {
					printf("|HLS DEBUG_LOG|%s|%s| SKIPPING (Avoid Zone) stream beat: %d, val=(\n", __func__, print_prompt, j);
				} else {
					printf("|HLS DEBUG_LOG|%s|%s| WRITING stream beat: %d to mem offset: %d, val=(\n", __func__, print_prompt, j, bank_offset + j);
				}

				// Print the payload using the original DataConv logic
				for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
				{
					DataConv conv;
					conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
					printf("     %f,\n", conv.f);
				}
				printf(")\n\n");
				printf("====================================================================================\n");
	#endif
			}
        }
    }
}

/**
 * @brief stridedTileStream2memWithAvoidV4: Writes a 2D strided tile from a hardware stream to memory, incorporating boundary avoidance.
 *
 * @details This is a standalone, optimized version that eliminates Vitis HLS loop flattening 
 * warnings by unifying the stream reads, discards, and memory writes into a single inner loop.
 * It uses statically provided bank configuration masks and sizes, reading a 32-bit base memory offset 
 * from a command stream per row. It avoids writing the first `avoid_x` elements to memory.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of the AXI memory interface and the input data stream.
 * @tparam NUM_BANKS The total number of memory banks.
 * @tparam BANK_ID The specific ID of the target memory bank.
 * @tparam BURST_SIZE The maximum AXI burst length (kept for API compatibility).
 * @tparam IN_ITR The targeted Initiation Interval (II) for the underlying pipeline (default: 2).
 *
 * @param[in] strm_in The input HLS stream containing the computed tile data to be written.
 * @param[out] out Pointer to the mapped target memory bank (typically an AXI4 master interface).
 * @param[in] offset_command The HLS stream providing the 32-bit absolute base offset in memory for the current row.
 * @param[in] bank_tile_size_x_floor Base tile size in the X dimension per bank.
 * @param[in] big_bank_tile_size_x_mask Bitmask indicating which banks receive an extra X tile element.
 * @param[in] bank_avoid_x_floor Base number of boundary elements to avoid writing in X per bank.
 * @param[in] big_bank_avoid_x_mask Bitmask for extra boundary elements to avoid in X.
 * @param[in] size_y The total number of iterations in the Y dimension.
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short NUM_BANKS, unsigned short BANK_ID, unsigned short BURST_SIZE=32, unsigned short IN_ITR=2>
static void stridedTileStream2memWithAvoidV4(
        ::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in, 
        ap_uint<MEM_DATA_WIDTH>* out, 
        ::hls::stream<ap_uint<32>>& offset_command, 
        const unsigned short bank_tile_size_x_floor,
        const ap_uint<NUM_BANKS> big_bank_tile_size_x_mask,
        const unsigned short bank_avoid_x_floor,
        const ap_uint<NUM_BANKS> big_bank_avoid_x_mask,
        const unsigned short size_y
#ifdef DEBUG_LOG_PRINT
        , const char* print_prompt = ""
#endif  
)
{
    // constexpr unsigned short BURST_SIZE_SHIFT = LOG2(BURST_SIZE);
    // constexpr unsigned short BURST_SIZE_MASK = BURST_SIZE - 1;

    const unsigned short bank_size_x = bank_tile_size_x_floor + big_bank_tile_size_x_mask.range(BANK_ID, BANK_ID);
    const unsigned short avoid_x = bank_avoid_x_floor + big_bank_avoid_x_mask.range(BANK_ID, BANK_ID);
    unsigned int bank_offset = 0;

#ifdef DEBUG_LOG_PRINT
    printf("|HLS DEBUG_LOG|%s|%s| reading stream to mem -  bank_size_x:%d, avoid_x:%d \n", 
            __func__, print_prompt, bank_size_x, avoid_x);
#endif

    tile_loop_y: for (unsigned short y = 0; y < size_y; y++) {
        write_burst_loop: for (unsigned short j = 0; j < bank_size_x; j++) {
            #pragma HLS LOOP_FLATTEN
            #pragma HLS PIPELINE II=IN_ITR
            
            if (j == 0) {
                bank_offset = offset_command.read();
            }
            // Always read from the stream to drain it properly
            ap_uint<MEM_DATA_WIDTH> tmp = strm_in.read();
            
            // Only write to memory if we have passed the avoid_x threshold.
            if (j >= avoid_x) {
                out[bank_offset + j] = tmp;
            }

#ifdef DEBUG_LOG_PRINT
            printf("====================================================================================\n");
            printf("|HLS DEBUG_LOG|%s|%s| writing burst  x: %d, y: %d\n",__func__ , print_prompt, j, y);
            // Identify if we are in the avoid zone or actively writing
            if (j < avoid_x) {
                printf("|HLS DEBUG_LOG|%s|%s| SKIPPING (Avoid Zone) stream beat: %d, val=(\n", __func__, print_prompt, j);
            } else {
                printf("|HLS DEBUG_LOG|%s|%s| WRITING stream beat: %d to mem offset: %d, val=(\n", __func__, print_prompt, j, bank_offset + j);
            }

            // Print the payload using the original DataConv logic
            for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
            {
                DataConv conv;
                conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
                printf("     %f,\n", conv.f);
            }
            printf(")\n\n");
            printf("====================================================================================\n");
#endif
        }
    }
}

/**
 * @brief stridedTileStream2memWithAvoidV5: Writes a 3D strided tile from a hardware stream to memory, incorporating boundary avoidance.
 *
 * @details This is a standalone, optimized version (V3) that eliminates Vitis HLS loop flattening 
 * warnings by unifying the stream reads, discards, and memory writes into a single inner loop.
 * It dynamically reads a 64-bit command to determine the absolute memory offset, write length (`size_x`), 
 * and boundary avoidance zone (`avoid_x`).
 *
 * @tparam MEM_DATA_WIDTH The bit-width of the AXI memory interface and the input data stream.
 * @tparam BURST_SIZE The maximum AXI burst length (kept for API compatibility).
 * @tparam IN_ITR The targeted Initiation Interval (II) for the underlying pipeline (default: 2).
 *
 * @param[in] strm_in The input HLS stream containing the computed tile data to be written.
 * @param[out] out Pointer to the mapped target memory bank (typically an AXI4 master interface).
 * @param[in] strm_command The HLS stream providing bank-specific 64-bit configuration commands.
 * - [31:0]  bank_offset: The absolute base offset in memory for the current row.
 * - [47:32] size_x:      The total number of elements to process in the X dimension.
 * - [63:48] avoid_x:     The number of boundary elements to avoid writing.
 * @param[in] size_y The total number of iterations in the Y dimension.
 * @param[in] size_z The total number of iterations in the Z dimension.
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short NUM_BANKS, unsigned short BANK_ID, unsigned short BURST_SIZE=32, unsigned short IN_ITR=2>
static void stridedTileStream2memWithAvoidV5(
    	::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in, 
		ap_uint<MEM_DATA_WIDTH>* out,
		const unsigned short bank_size_x_floor,
		const ap_uint<1> is_big_size_x,
		const unsigned short bank_grid_size_x_floor, 
		const ap_uint<LOG2(NUM_BANKS)> big_grid_size_x_banks_upper, 
		const unsigned short tile_offset_x_floor, 
		const ap_uint<LOG2(NUM_BANKS)> big_offset_x_banks_upper,  
		const unsigned short bank_avoid_x_floor,
		const ap_uint<1> is_big_avoid_x,
		const unsigned short tile_size_y,
		const unsigned short grid_size_y,
		const unsigned short tile_offset_y=0,
		const unsigned short avoid_y=0,
		const unsigned short tile_size_z=1
#ifdef DEBUG_LOG_PRINT
		, const char* print_prompt = ""
#endif  
)
{
    // const unsigned int total_iters = size_z * size_y;
	const unsigned short bank_size_x = bank_size_x_floor + is_big_size_x;
	const unsigned short avoid_x = bank_avoid_x_floor + is_big_avoid_x;
	const unsigned short bank_grid_size_x = bank_grid_size_x_floor + (BANK_ID < big_grid_size_x_banks_upper ? 1 : 0);
	const unsigned short init_bank_offset_x = tile_offset_x_floor + (BANK_ID < big_offset_x_banks_upper ? 1 : 0);
	const unsigned short init_bank_offset_y = tile_offset_y * bank_grid_size_x;
	const unsigned short init_bank_offest = init_bank_offset_x + init_bank_offset_y;
	const unsigned short bank_stride_z = bank_grid_size_x * grid_size_y;
	unsigned int bank_offset = 0;

#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s|%s| reading stream to mem -  bank_size_x:%d, avoid_x:%d \n", 
			__func__, print_prompt, bank_size_x, avoid_x);
#endif

    tile_loop1: for (unsigned short z = 0; z < tile_size_z; z++)
    {
        tile_loop2: for (unsigned short y = avoid_y; y < tile_size_y; y++)
        {
			read_burst_loop: for (unsigned short j = 0; j < bank_size_x; j++) 
			{
				#pragma HLS LOOP_FLATTEN
				#pragma HLS PIPELINE II=IN_ITR
				
				if (j == 0){
					bank_offset = init_bank_offest + y * bank_grid_size_x + z * bank_stride_z;
				} else {
					bank_offset += 1;
				}
				// Always read from the stream to drain it properly
				ap_uint<MEM_DATA_WIDTH> tmp = strm_in.read();
				
				// Only write to memory if we have passed the avoid_x threshold.
				if (j >= avoid_x) {
					out[bank_offset] = tmp;
				}

#ifdef DEBUG_LOG_PRINT
				printf("====================================================================================\n");
				printf("|HLS DEBUG_LOG|%s|%s| writing  j: %d, k: %d \n",__func__ , print_prompt, y, z);
				// Identify if we are in the avoid zone or actively writing
				if (j < avoid_x) {
					printf("|HLS DEBUG_LOG|%s|%s| SKIPPING (Avoid Zone) stream beat: %d, val=(\n", __func__, print_prompt, j);
				} else {
					printf("|HLS DEBUG_LOG|%s|%s| WRITING stream beat: %d to mem offset: %d, val=(\n", __func__, print_prompt, j, bank_offset);
				}

				// Print the payload using the original DataConv logic
				for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
				{
					DataConv conv;
					conv.i = tmp.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
					printf("     %f,\n", conv.f);
				}
				printf(")\n\n");
				printf("====================================================================================\n");
#endif
			}
        }
    }
}

/**
 * @brief redirect: Reorders and aligns strided tile data from multiple input streams to output streams.
 *
 * @details This function acts as a dynamic router and barrel shifter. It decodes a 160-bit 
 * configuration command to determine the 3D memory access pattern and the resulting bank 
 * wrap-around logic. It selectively reads from a partitioned array of input streams based 
 * on boundary conditions, and then combinatorially rotates (barrel shifts) the data 
 * to align it correctly across the output streams, compensating for the starting bank offset.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of the data elements in the streams.
 * @tparam NUM_BANKS The number of memory banks and corresponding streams. Must be a power 
 * of two to enable bitwise modulo masking and efficient barrel shifting.
 * @tparam BURST_SIZE Burst size for memory transfers (default: 32).
 * @tparam IN_ITR Initiation interval for the inner HLS pipeline (default: 2).
 *
 * @param[in] strm_in Array of input HLS streams (size NUM_BANKS) providing the raw, 
 * unaligned data read from memory.
 * @param[out] strm_out Array of output HLS streams (size NUM_BANKS) where the correctly 
 * aligned and shifted data will be written.
 * @param[in] command A 160-bit packed configuration word containing:
 * 		- [63:0]   offset_x: Base offset in the X dimension
 * 		- [95:80]  size_x:   Size in the X dimension
 * 		- [111:96] stride_y: Stride in the Y dimension
 * 		- [127:112] size_y:   Size in the Y dimension
 * 		- [143:128] stride_z: Stride in the Z dimension
 * 		- [159:144] size_z:   Size in the Z dimension
 *
 */

template <unsigned short MEM_DATA_WIDTH, unsigned short NUM_BANKS, unsigned short IN_ITR=2>
static void redirect(
        ::hls::stream<ap_uint<MEM_DATA_WIDTH>> strm_in[NUM_BANKS],
        ::hls::stream<ap_uint<MEM_DATA_WIDTH>> strm_out[NUM_BANKS],
        const size_t offset_x, const unsigned short size_x, const unsigned short stride_y, const unsigned short size_y, const unsigned short stride_z, const unsigned short size_z)
{
    // Enforce the power-of-two constraint for bitwise optimizations
#ifndef __SYNTHESIS__
    static_assert((NUM_BANKS != 0) && ((NUM_BANKS & (NUM_BANKS - 1)) == 0), "NUM_BANKS must be a power of two");
#endif 

    constexpr unsigned short NUM_BANKS_SHIFT = LOG2(NUM_BANKS);
    constexpr unsigned short BANK_MASK = NUM_BANKS - 1;

    // size_t offset_x = command.range(63,0);
    // ap_uint<16> size_x = command.range(95,80);
    // ap_uint<16> stride_y = command.range(111,96);
    // ap_uint<16> size_y = command.range(127,112);
    // ap_uint<16> stride_z = command.range(143,128);
    // ap_uint<16> size_z = command.range(159,144);
    
    unsigned short size_x_div_by_banks_ceil = (size_x + NUM_BANKS - 1) >> NUM_BANKS_SHIFT;
    unsigned short size_x_div_by_banks_floor = size_x >> NUM_BANKS_SHIFT;
    
    // Optimized modulo using bitwise AND
    unsigned int size_x_mod_banks = size_x & BANK_MASK;

#ifdef DEBUG_LOG_PRINT
    printf("==== DATAMOVER REDIRECT INITIAL PARAMETERS ====\n");
    printf("offset_x: %lu\n", offset_x);
    printf("size_x: %u\n", (unsigned int)size_x);
    printf("stride_y: %u\n", (unsigned int)stride_y);
    printf("size_y: %u\n", (unsigned int)size_y);
    printf("stride_z: %u\n", (unsigned int)stride_z);
    printf("size_z: %u\n", (unsigned int)size_z);
    printf("size_x_div_by_banks_ceil: %u\n", size_x_div_by_banks_ceil);
    printf("size_x_div_by_banks_floor: %u\n", size_x_div_by_banks_floor);
    printf("size_x_mod_banks: %u\n", size_x_mod_banks);
    printf("NUM_BANKS: %u\n", NUM_BANKS);
    printf("========================================\n");
#endif

    for (unsigned short z = 0; z < size_z; z++)
    {
        for (unsigned short y = 0; y < size_y; y++)
        {
            #pragma HLS LOOP_FLATTEN

            const size_t z_offset = offset_x + z * stride_z;
            const size_t abs_offset = z_offset + y * stride_y;
            
            // Replaced modulo with bitwise AND
            const unsigned int starting_bank = abs_offset & BANK_MASK;
            const unsigned int banks_upper_limit = (starting_bank + size_x_mod_banks) & BANK_MASK;
            const bool is_t1_or_t2 = banks_upper_limit < starting_bank;

            // Pre-calculate bank conditions outside the inner pipeline loop
            bool bank_cond[NUM_BANKS];
            #pragma HLS ARRAY_PARTITION variable=bank_cond complete dim=1

            for (unsigned int i = 0; i < NUM_BANKS; i++)
            {
                #pragma HLS UNROLL
                const bool is_gt_sb = i >= starting_bank;
                const bool is_lt_uplim = i < banks_upper_limit;

                if (is_t1_or_t2) {
                    bank_cond[i] = is_gt_sb || is_lt_uplim;
                } else {
                    bank_cond[i] = is_gt_sb && is_lt_uplim;
                }
            }

            for (unsigned short x = 0; x < size_x_div_by_banks_ceil; x++)
            {
                #pragma HLS PIPELINE II=IN_ITR

                // Fully partition these to map to discrete registers
                ap_uint<MEM_DATA_WIDTH> tmp[NUM_BANKS];
                ap_uint<MEM_DATA_WIDTH> swap[NUM_BANKS];
                #pragma HLS ARRAY_PARTITION variable=tmp complete dim=1
                #pragma HLS ARRAY_PARTITION variable=swap complete dim=1

                const bool is_x_lt_bank_x_size = x < size_x_div_by_banks_floor;

                // 1. Read from input streams based on conditions
                for (unsigned int i = 0; i < NUM_BANKS; i++)
                {
                    #pragma HLS UNROLL
                    const bool read_cond = is_x_lt_bank_x_size || bank_cond[i];
                    tmp[i] = read_cond ? register_it(strm_in[i].read()) : ap_uint<MEM_DATA_WIDTH>(0);

#ifdef DEBUG_LOG_PRINT
					auto read_val = tmp[i]; 
					if (read_cond) {
						printf("==== REDIRECT READ ====\n");
						printf("Bank[%u], X[%u], Y[%u], Z[%u]\n", i, x, (unsigned int)y, (unsigned int)z);
						printf("Values (float): (");
						for (unsigned int j = 0; j < MEM_DATA_WIDTH / (DEBUG_LOG_SIZE_OF * 8); j++) {
							DataConv conv;
							conv.i = read_val.range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
							if (j > 0) printf(", ");
							printf("%f", conv.f);
						}
						printf(")\n");
						printf("=====================\n");
					}
#endif
                }

                // 2. The Barrel Shifter (Replaces the large switch statement)
                for (unsigned int i = 0; i < NUM_BANKS; i++)
                {
                    #pragma HLS UNROLL
                    unsigned int src_idx = (starting_bank + i) & BANK_MASK;
                    swap[i] = tmp[src_idx];
                }

                // 3. Write to output streams
                for (unsigned int i = 0; i < NUM_BANKS; i++)
                {
                    #pragma HLS UNROLL
                    strm_out[i] << swap[i];

#ifdef DEBUG_LOG_PRINT
					auto write_val = swap[i];
					printf("==== REDIRECT WRITE ====\n");
					printf("Bank[%u],  X[%u], Y[%u], Z[%u]\n", i, x, (unsigned int)y, (unsigned int)z);
					printf("Values (float): (");
					for (unsigned int j = 0; j < MEM_DATA_WIDTH / (DEBUG_LOG_SIZE_OF * 8); j++) {
						DataConv conv;
						conv.i = write_val.range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
						if (j > 0) printf(", ");
						printf("%f", conv.f);
					}
					printf(")\n");
					printf("======================\n");
#endif
                }
            }
        }
    }
}

/**
 * @brief redirectV2: Reorders and aligns strided tile data from multiple input streams to output streams.
 *
 * @details This function acts as a dynamic router and barrel shifter. It decodes a 160-bit 
 * configuration command to determine the 3D memory access pattern and the resulting bank 
 * wrap-around logic. It selectively reads from a partitioned array of input streams based 
 * on boundary conditions, and then combinatorially rotates (barrel shifts) the data 
 * to align it correctly across the output streams, compensating for the starting bank offset.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of the data elements in the streams.
 * @tparam NUM_BANKS The number of memory banks and corresponding streams. Must be a power 
 * of two to enable bitwise modulo masking and efficient barrel shifting.
 * @tparam BURST_SIZE Burst size for memory transfers (default: 32).
 * @tparam IN_ITR Initiation interval for the inner HLS pipeline (default: 2).
 *
 * @param[in] strm_in Array of input HLS streams (size NUM_BANKS) providing the raw, 
 * unaligned data read from memory.
 * @param[out] strm_out Array of output HLS streams (size NUM_BANKS) where the correctly 
 * aligned and shifted data will be written.
 * @param[in] command A 160-bit packed configuration word containing:
 * 		- [63:0]   offset_x: Base offset in the X dimension
 * 		- [95:80]  size_x:   Size in the X dimension
 * 		- [111:96] stride_y: Stride in the Y dimension
 * 		- [127:112] size_y:   Size in the Y dimension
 * 		- [143:128] stride_z: Stride in the Z dimension
 * 		- [159:144] size_z:   Size in the Z dimension
 *
 */

template <unsigned short MEM_DATA_WIDTH, unsigned short NUM_BANKS, unsigned short IN_ITR=2>
static void redirectV2(
        ::hls::stream<ap_uint<MEM_DATA_WIDTH>> strm_in[NUM_BANKS],
        ::hls::stream<ap_uint<MEM_DATA_WIDTH>> strm_out[NUM_BANKS],
		const unsigned short bank_tile_size_x_floor, const unsigned short bank_tile_size_x_ceil, const ap_uint<NUM_BANKS> big_bank_tile_size_x_mask, const ap_uint<LOG2(NUM_BANKS)> big_offset_x_banks_upper, 
		/*const unsigned short tile_size_y, const unsigned short tile_size_z*/ const unsigned int total_rows)
{
    // Enforce the power-of-two constraint for bitwise optimizations
#ifndef __SYNTHESIS__
    static_assert((NUM_BANKS != 0) && ((NUM_BANKS & (NUM_BANKS - 1)) == 0), "NUM_BANKS must be a power of two");
#endif 

    constexpr unsigned short NUM_BANKS_SHIFT = LOG2(NUM_BANKS);
    constexpr unsigned short BANK_MASK = NUM_BANKS - 1;

#ifdef DEBUG_LOG_PRINT
    printf("==== DATAMOVER REDIRECT INITIAL PARAMETERS ====\n");
	printf("bank_tile_size_x_ceil: %u\n", bank_tile_size_x_ceil);
    printf("bank_tile_size_x_floor: %u\n", bank_tile_size_x_floor);
    printf("is_big_bank_size_x: {");
	for (ap_uint<LOG2(NUM_BANKS)+1> b_id = 0; b_id < NUM_BANKS; b_id++) {
		 printf("%d%s", (unsigned short)big_bank_tile_size_x_mask.range(b_id, b_id), (b_id == NUM_BANKS - 1 ? "" : ", "));
	}
	printf("}\n");
    // printf("tile_size_y: %u\n", (unsigned int)tile_size_y);
    // printf("tile_size_z: %u\n", (unsigned int)tile_size_z);
	printf("total_rows: %u\n", (unsigned int)total_rows);
    printf("NUM_BANKS: %u\n", NUM_BANKS);
    printf("========================================\n");
#endif

    // for (unsigned short z = 0; z < size_z; z++)
    // {
    //     for (unsigned short y = 0; y < size_y; y++)
    //     {
	for (unsigned int row_id = 0; row_id < total_rows; row_id++) 
	{
		#pragma HLS LOOP_FLATTEN

		// const size_t z_offset = offset_x + z * stride_z;
		// const size_t abs_offset = z_offset + y * stride_y;
		
		// Replaced modulo with bitwise AND
		const ap_uint<NUM_BANKS_SHIFT> starting_bank = big_offset_x_banks_upper;
		// const unsigned int banks_upper_limit = (starting_bank + size_x_mod_banks) & BANK_MASK;
		// const bool is_t1_or_t2 = banks_upper_limit < starting_bank;

		// Pre-calculate bank conditions outside the inner pipeline loop
		// bool bank_cond[NUM_BANKS];
		// #pragma HLS ARRAY_PARTITION variable=bank_cond complete dim=1

		// for (unsigned int i = 0; i < NUM_BANKS; i++)
		// {
		//     #pragma HLS UNROLL
		//     const bool is_gt_sb = i >= starting_bank;
		//     const bool is_lt_uplim = i < banks_upper_limit;

		//     if (is_t1_or_t2) {
		//         bank_cond[i] = is_gt_sb || is_lt_uplim;
		//     } else {
		//         bank_cond[i] = is_gt_sb && is_lt_uplim;
		//     }
		// }

		for (unsigned short x = 0; x < bank_tile_size_x_ceil; x++)
		{
			#pragma HLS PIPELINE II=IN_ITR

			// Fully partition these to map to discrete registers
			ap_uint<MEM_DATA_WIDTH> tmp[NUM_BANKS];
			ap_uint<MEM_DATA_WIDTH> swap[NUM_BANKS];
			#pragma HLS ARRAY_PARTITION variable=tmp complete dim=1
			#pragma HLS ARRAY_PARTITION variable=swap complete dim=1

			const bool is_x_lt_bank_x_size = x < bank_tile_size_x_floor;

			// 1. Read from input streams based on conditions
			for (unsigned int i = 0; i < NUM_BANKS; i++)
			{
				#pragma HLS UNROLL
				const bool read_cond = is_x_lt_bank_x_size || big_bank_tile_size_x_mask.range(i,i);
				tmp[i] = read_cond ? register_it(strm_in[i].read()) : ap_uint<MEM_DATA_WIDTH>(0);

#ifdef DEBUG_LOG_PRINT
				auto read_val = tmp[i]; 
				if (read_cond) {
					printf("==== REDIRECT READ ====\n");
					printf("Bank[%u], X[%u], ROW_ID[%u]\n", i, x, (unsigned int)row_id);
					printf("Values (float): (");
					for (unsigned int j = 0; j < MEM_DATA_WIDTH / (DEBUG_LOG_SIZE_OF * 8); j++) {
						DataConv conv;
						conv.i = read_val.range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
						if (j > 0) printf(", ");
						printf("%f", conv.f);
					}
					printf(")\n");
					printf("=====================\n");
				}
#endif
			}

			// 2. The Barrel Shifter (Replaces the large switch statement)
			for (unsigned int i = 0; i < NUM_BANKS; i++)
			{
				#pragma HLS UNROLL
				unsigned int src_idx = (starting_bank + i) & BANK_MASK;
				swap[i] = tmp[src_idx];
			}

			// 3. Write to output streams
			for (unsigned int i = 0; i < NUM_BANKS; i++)
			{
				#pragma HLS UNROLL
				strm_out[i] << swap[i];

#ifdef DEBUG_LOG_PRINT
				auto write_val = swap[i];
				printf("==== REDIRECT WRITE ====\n");
				printf("Bank[%u],  X[%u], ROW_ID[%u]\n", i, x, (unsigned int)row_id);
				printf("Values (float): (");
				for (unsigned int j = 0; j < MEM_DATA_WIDTH / (DEBUG_LOG_SIZE_OF * 8); j++) {
					DataConv conv;
					conv.i = write_val.range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
					if (j > 0) printf(", ");
					printf("%f", conv.f);
				}
				printf(")\n");
				printf("======================\n");
#endif
			}
        }
    }
}

/**
 * @brief aggregatedStream2streamStepdown: Reads multiple input streams with higher data width and distributes 
 * 		them as smaller chunks across multiple output streams.
 *
 * @details This function implements a stream width reduction and distribution mechanism. It reads packets
 * 		from multiple input streams with higher bit-width (STREAM1_DATA_WIDTH) at an initiation interval of IN_ITR,
 * 		and splits each packet into FACTOR smaller chunks. These chunks are then collected and distributed
 * 		round-robin across the output streams with lower bit-width (STREAM2_DATA_WIDTH), effectively converting
 * 		NUM_STRMS × STREAM1_DATA_WIDTH per cycle into FACTOR output waves of NUM_STRMS × STREAM2_DATA_WIDTH.
 *
 * @tparam STREAM1_DATA_WIDTH The bit-width of the input stream data (must be larger than STREAM2_DATA_WIDTH).
 * @tparam STREAM2_DATA_WIDTH The bit-width of the output stream data (must evenly divide STREAM1_DATA_WIDTH).
 * @tparam NUM_STRMS The number of input and output streams. Must be a power of two.
 * @tparam IN_ITR Initiation interval for the outer HLS pipeline reading input streams (default: 2).
 *
 * @param[in] strm_in Array of input HLS streams (size NUM_STRMS) with bit-width STREAM1_DATA_WIDTH,
 * 		providing wide data packets to be split and redistributed.
 * @param[out] strm_out Array of output HLS streams (size NUM_STRMS) with bit-width STREAM2_DATA_WIDTH,
 * 		receiving the split and redistributed data chunks in round-robin fashion.
 * @param[in] num_big_pkts The number of large packets (width STREAM1_DATA_WIDTH) to process from input streams.
 *
 */
template <unsigned short STREAM1_DATA_WIDTH, unsigned short STREAM2_DATA_WIDTH, unsigned short NUM_STRMS, unsigned short IN_ITR=2>
static void aggregatedStream2streamStepdown(::hls::stream<ap_uint<STREAM1_DATA_WIDTH>> strm_in[NUM_STRMS],
				::hls::stream<ap_uint<STREAM2_DATA_WIDTH>> strm_out[NUM_STRMS],
				const unsigned int num_big_pkts)
{
	constexpr unsigned short FACTOR = STREAM1_DATA_WIDTH / STREAM2_DATA_WIDTH;
    constexpr unsigned int ii_adj = (IN_ITR >= FACTOR) ? IN_ITR : FACTOR;
	constexpr unsigned short ii_out = ii_adj / FACTOR;
#ifndef __SYNTHESIS__
    static_assert((NUM_STRMS != 0) && ((NUM_STRMS & (NUM_STRMS - 1)) == 0), "NUM_STRMS must be a power of two");
	static_assert(STREAM1_DATA_WIDTH > STREAM2_DATA_WIDTH,
			"STREAM1_DATA_WIDTH has to be bigger than STREAM2_DATA_WIDTH");
    static_assert(STREAM1_DATA_WIDTH % STREAM2_DATA_WIDTH == 0, 
            "STREAM1_DATA_WIDTH has to be fully divisible by STREAM2_DATA_WIDTH");
    static_assert(STREAM1_DATA_WIDTH % 8 == 0, "STREAM1_DATA_WIDTH should be divisible by 8");
    static_assert(STREAM2_DATA_WIDTH % 8 == 0, "STREAM2_DATA_WIDTH should be divisible by 8");
	static_assert(ii_adj % FACTOR == 0, "Output ii is not divisible by input II");
#endif 



#ifdef DEBUG_LOG_PRINT
		printf("====================================================================================\n");
        printf("|HLS DEBUG_LOG| %s | adj_ii: %d, num_big_pkts: %d\n"
                , __func__, ii_adj , num_big_pkts);
        printf("====================================================================================\n");
#endif


	ap_uint<STREAM1_DATA_WIDTH> read_val[NUM_STRMS];
	#pragma HLS ARRAY_PARTITION variable=read_val

	for (unsigned int pkt = 0; pkt < num_big_pkts; pkt++)
	{
	#pragma HLS PIPELINE II=ii_adj

#ifdef DEBUG_LOG_PRINT
		printf("====================================================================================\n");
        printf("|HLS DEBUG_LOG| aggreatedStream2streamStepdown | reading pkt: %d, read values \n", pkt);
#endif 
		read: for (unsigned short num_strm = 0; num_strm < NUM_STRMS; num_strm++)
		{
		#pragma HLS UNROLL
			read_val[num_strm] = strm_in[num_strm].read();
#ifdef DEBUG_LOG_PRINT
		printf("[sream: %d] = (", num_strm);
			for (unsigned k = 0; k < STREAM1_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
			{
				DataConv conv;
				conv.i = read_val[num_strm].range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
				printf("%f%s ", conv.f, k != STREAM1_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8) ? "," : "");
			}
        printf(")\n");
#endif
		}
#ifdef DEBUG_LOG_PRINT
		printf("====================================================================================\n");
#endif 
 
		write: for (unsigned short output_cycle = 0; output_cycle < FACTOR; output_cycle++)
		{
		#pragma HLS PIPELINE II=ii_out
#ifdef DEBUG_LOG_PRINT
		printf("====================================================================================\n");
        printf("|HLS DEBUG_LOG| aggreatedStream2streamStepdown | output pkt: %d \n", pkt*FACTOR + output_cycle);
#endif			
			for (unsigned short out_strm = 0; out_strm < NUM_STRMS; out_strm++)
			{
			#pragma HLS UNROLL
				// Distribute all chunks round-robin across output streams
				unsigned short chunk_idx = output_cycle * NUM_STRMS + out_strm;
				unsigned short in_strm = chunk_idx / FACTOR;
				unsigned short chunk_k = chunk_idx % FACTOR;
				
				ap_uint<STREAM2_DATA_WIDTH> out_val = 
					read_val[in_strm].range((chunk_k+1) * STREAM2_DATA_WIDTH - 1, chunk_k * STREAM2_DATA_WIDTH);
				strm_out[out_strm].write(out_val);
#ifdef DEBUG_LOG_PRINT
				printf("[stream_out: %d, from_in_stream: %d, chunk: %d] = (", out_strm, in_strm, chunk_k);
				for (unsigned j = 0; j < STREAM2_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); j++)
				{
					DataConv conv;
					conv.i = out_val.range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
					printf("%f%s ", conv.f, j != STREAM2_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8) ? "," : "");
				}
				printf(")\n");
#endif
			}
		}
	}
}

/**
 * @brief aggregatedStream2streamStepdown: Reads multiple input streams with higher data width and distributes 
 * 		them as smaller chunks across multiple output streams.
 *
 * @details This function implements a stream width reduction and distribution mechanism. It reads packets
 * 		from multiple input streams with higher bit-width (STREAM1_DATA_WIDTH) at an initiation interval of IN_ITR,
 * 		and splits each packet into FACTOR smaller chunks. These chunks are then collected and distributed
 * 		round-robin across the output streams with lower bit-width (STREAM2_DATA_WIDTH), effectively converting
 * 		NUM_STRMS × STREAM1_DATA_WIDTH per cycle into FACTOR output waves of NUM_STRMS × STREAM2_DATA_WIDTH.
 *
 * @tparam STREAM1_DATA_WIDTH The bit-width of the input stream data (must be larger than STREAM2_DATA_WIDTH).
 * @tparam STREAM2_DATA_WIDTH The bit-width of the output stream data (must evenly divide STREAM1_DATA_WIDTH).
 * @tparam NUM_STRMS The number of input and output streams. Must be a power of two.
 * @tparam IN_ITR Initiation interval for the outer HLS pipeline reading input streams (default: 2).
 *
 * @param[in] strm_in Array of input HLS streams (size NUM_STRMS) with bit-width STREAM1_DATA_WIDTH,
 * 		providing wide data packets to be split and redistributed.
 * @param[out] strm_out Array of output HLS streams (size NUM_STRMS) with bit-width STREAM2_DATA_WIDTH,
 * 		receiving the split and redistributed data chunks in round-robin fashion.
 * @param[in] num_big_pkts The number of large packets (width STREAM1_DATA_WIDTH) to process from input streams.
 * @param[in] is_small_tile Flag to point small tile_x which is less than NUM_STRMS/2.
 *
 */
template <unsigned short STREAM1_DATA_WIDTH, unsigned short STREAM2_DATA_WIDTH, unsigned short NUM_STRMS, unsigned short IN_ITR=2>
static void aggregatedStream2streamStepdown(::hls::stream<ap_uint<STREAM1_DATA_WIDTH>> strm_in[NUM_STRMS],
				::hls::stream<ap_uint<STREAM2_DATA_WIDTH>> strm_out[NUM_STRMS],
				const unsigned int num_big_pkts,
				const bool is_small_tile)
{
	constexpr unsigned short FACTOR = STREAM1_DATA_WIDTH / STREAM2_DATA_WIDTH;
    constexpr unsigned int ii_adj = (IN_ITR >= FACTOR) ? IN_ITR : FACTOR;
	constexpr unsigned short ii_out = ii_adj / FACTOR;
	constexpr unsigned short HALF_FACTOR = FACTOR > 1 ? FACTOR >> 1 : FACTOR;

#ifndef __SYNTHESIS__
    static_assert((NUM_STRMS != 0) && ((NUM_STRMS & (NUM_STRMS - 1)) == 0), "NUM_STRMS must be a power of two");
	static_assert(STREAM1_DATA_WIDTH > STREAM2_DATA_WIDTH,
			"STREAM1_DATA_WIDTH has to be bigger than STREAM2_DATA_WIDTH");
    static_assert(STREAM1_DATA_WIDTH % STREAM2_DATA_WIDTH == 0, 
            "STREAM1_DATA_WIDTH has to be fully divisible by STREAM2_DATA_WIDTH");
    static_assert(STREAM1_DATA_WIDTH % 8 == 0, "STREAM1_DATA_WIDTH should be divisible by 8");
    static_assert(STREAM2_DATA_WIDTH % 8 == 0, "STREAM2_DATA_WIDTH should be divisible by 8");
	static_assert(ii_adj % FACTOR == 0, "Output ii is not divisible by input II");
#endif 


#ifdef DEBUG_LOG_PRINT
		printf("====================================================================================\n");
        printf("|HLS DEBUG_LOG| %s | adj_ii: %d, num_big_pkts: %d\n"
                , __func__, ii_adj , num_big_pkts);
        printf("====================================================================================\n");
#endif

	ap_uint<STREAM1_DATA_WIDTH> read_val[NUM_STRMS];
	#pragma HLS ARRAY_PARTITION variable=read_val

	for (unsigned int pkt = 0; pkt < num_big_pkts; pkt++)
	{
	#pragma HLS PIPELINE II=ii_adj

#ifdef DEBUG_LOG_PRINT
		printf("====================================================================================\n");
        printf("|HLS DEBUG_LOG| aggreatedStream2streamStepdown | reading pkt: %d, read values \n", pkt);
#endif 
		read: for (unsigned short num_strm = 0; num_strm < NUM_STRMS; num_strm++)
		{
		#pragma HLS UNROLL
			read_val[num_strm] = strm_in[num_strm].read();
#ifdef DEBUG_LOG_PRINT
		printf("[sream: %d] = (", num_strm);
			for (unsigned k = 0; k < STREAM1_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
			{
				DataConv conv;
				conv.i = read_val[num_strm].range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
				printf("%f%s ", conv.f, k != STREAM1_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8) ? "," : "");
			}
        printf(")\n");
#endif
		}
#ifdef DEBUG_LOG_PRINT
		printf("====================================================================================\n");
#endif 
 
		write: if (is_small_tile) {
			for (unsigned short output_cycle = 0; output_cycle < HALF_FACTOR; output_cycle++)
			{
			#pragma HLS PIPELINE II=ii_out
#ifdef DEBUG_LOG_PRINT
			printf("====================================================================================\n");
			printf("|HLS DEBUG_LOG| aggreatedStream2streamStepdown (small tile) | output pkt: %d \n", pkt*FACTOR + output_cycle);
#endif			
				for (unsigned short out_strm = 0; out_strm < NUM_STRMS; out_strm++)
				{
				#pragma HLS UNROLL
					// Distribute all chunks round-robin across output streams
					unsigned short chunk_idx = output_cycle * NUM_STRMS + out_strm;
					unsigned short in_strm = chunk_idx / FACTOR;
					unsigned short chunk_k = chunk_idx % FACTOR;
					
					ap_uint<STREAM2_DATA_WIDTH> out_val = 
						read_val[in_strm].range((chunk_k+1) * STREAM2_DATA_WIDTH - 1, chunk_k * STREAM2_DATA_WIDTH);
					strm_out[out_strm].write(out_val);
#ifdef DEBUG_LOG_PRINT
					printf("[stream_out: %d, from_in_stream: %d, chunk: %d] = (", out_strm, in_strm, chunk_k);
					for (unsigned j = 0; j < STREAM2_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); j++)
					{
						DataConv conv;
						conv.i = out_val.range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
						printf("%f%s ", conv.f, j != STREAM2_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8) ? "," : "");
					}
					printf(")\n");
#endif
				}
			}
		} else {
			for (unsigned short output_cycle = 0; output_cycle < FACTOR; output_cycle++)
			{
			#pragma HLS PIPELINE II=ii_out
#ifdef DEBUG_LOG_PRINT
			printf("====================================================================================\n");
			printf("|HLS DEBUG_LOG| aggreatedStream2streamStepdown | output pkt: %d \n", pkt*FACTOR + output_cycle);
#endif			
				for (unsigned short out_strm = 0; out_strm < NUM_STRMS; out_strm++)
				{
				#pragma HLS UNROLL
					// Distribute all chunks round-robin across output streams
					unsigned short chunk_idx = output_cycle * NUM_STRMS + out_strm;
					unsigned short in_strm = chunk_idx / FACTOR;
					unsigned short chunk_k = chunk_idx % FACTOR;
					
					ap_uint<STREAM2_DATA_WIDTH> out_val = 
						read_val[in_strm].range((chunk_k+1) * STREAM2_DATA_WIDTH - 1, chunk_k * STREAM2_DATA_WIDTH);
					strm_out[out_strm].write(out_val);
#ifdef DEBUG_LOG_PRINT
					printf("[stream_out: %d, from_in_stream: %d, chunk: %d] = (", out_strm, in_strm, chunk_k);
					for (unsigned j = 0; j < STREAM2_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); j++)
					{
						DataConv conv;
						conv.i = out_val.range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
						printf("%f%s ", conv.f, j != STREAM2_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8) ? "," : "");
					}
					printf(")\n");
#endif
				}
			}
		}
	}
}

/**
 * @brief aggregatedStream2streamStepup: Reads smaller chunks from multiple input streams and combines 
 * 		them into larger packets distributed across multiple output streams.
 *
 * @details This function implements the inverse operation of aggregatedStream2streamStepdown. It reads
 * 		data from multiple input streams with lower bit-width (STREAM1_DATA_WIDTH) organized in FACTOR 
 * 		cycles of round-robin distribution. It collects these chunks and reorganizes them back into 
 * 		larger packets with higher bit-width (STREAM2_DATA_WIDTH) for output across NUM_STRMS output streams.
 * 		Effectively converts FACTOR output waves of NUM_STRMS × STREAM1_DATA_WIDTH back into
 * 		NUM_STRMS × STREAM2_DATA_WIDTH per cycle.
 *
 * @tparam STREAM1_DATA_WIDTH The bit-width of the input stream data (must be smaller than STREAM2_DATA_WIDTH).
 * @tparam STREAM2_DATA_WIDTH The bit-width of the output stream data (must be a multiple of STREAM1_DATA_WIDTH).
 * @tparam NUM_STRMS The number of input and output streams. Must be a power of two.
 * @tparam IN_ITR Initiation interval for the outer HLS pipeline combining input chunks (default: 2).
 *
 * @param[in] strm_in Array of input HLS streams (size NUM_STRMS) with bit-width STREAM1_DATA_WIDTH,
 * 		providing chunks distributed across FACTOR cycles per output packet.
 * @param[out] strm_out Array of output HLS streams (size NUM_STRMS) with bit-width STREAM2_DATA_WIDTH,
 * 		receiving the recombined wider packets.
 * @param[in] num_big_pkts The number of large output packets (width STREAM2_DATA_WIDTH) to generate.
 *
 */
template <unsigned short STREAM1_DATA_WIDTH, unsigned short STREAM2_DATA_WIDTH, unsigned short NUM_STRMS, unsigned short IN_ITR=2>
static void aggregatedStream2streamStepup(::hls::stream<ap_uint<STREAM1_DATA_WIDTH>> strm_in[NUM_STRMS],
			::hls::stream<ap_uint<STREAM2_DATA_WIDTH>> strm_out[NUM_STRMS],
			const unsigned int num_big_pkts)
{
	constexpr unsigned short FACTOR = STREAM2_DATA_WIDTH / STREAM1_DATA_WIDTH;
    constexpr unsigned int ii_adj = (IN_ITR >= FACTOR) ? IN_ITR : FACTOR;
	constexpr unsigned short ii_out = ii_adj / FACTOR;
#ifndef __SYNTHESIS__
    static_assert((NUM_STRMS != 0) && ((NUM_STRMS & (NUM_STRMS - 1)) == 0), "NUM_STRMS must be a power of two");
	static_assert(STREAM2_DATA_WIDTH > STREAM1_DATA_WIDTH,
			"STREAM2_DATA_WIDTH has to be bigger than STREAM1_DATA_WIDTH");
    static_assert(STREAM2_DATA_WIDTH % STREAM1_DATA_WIDTH == 0, 
            "STREAM2_DATA_WIDTH has to be fully divisible by STREAM1_DATA_WIDTH");
    static_assert(STREAM1_DATA_WIDTH % 8 == 0, "STREAM1_DATA_WIDTH should be divisible by 8");
    static_assert(STREAM2_DATA_WIDTH % 8 == 0, "STREAM2_DATA_WIDTH should be divisible by 8");
	static_assert(ii_adj % FACTOR == 0, "Output ii is not divisible by input II");
#endif 


#ifdef DEBUG_LOG_PRINT
		printf("====================================================================================\n");
        printf("|HLS DEBUG_LOG| %s | adj_ii: %d, num_big_pkts: %d\n"
                , __func__, ii_adj , num_big_pkts);
        printf("====================================================================================\n");
#endif

	// Array to collect chunks for reconstruction (indexed by [input_stream][chunk_position])
	ap_uint<STREAM1_DATA_WIDTH> chunks[NUM_STRMS][FACTOR];
	#pragma HLS ARRAY_PARTITION variable=chunks dim=0 complete

	for (unsigned int pkt = 0; pkt < num_big_pkts; pkt++)
	{
	#pragma HLS PIPELINE II=ii_adj

#ifdef DEBUG_LOG_PRINT
		printf("====================================================================================\n");
        printf("|HLS DEBUG_LOG| aggregatedStream2streamStepup | combining pkt: %d \n", pkt);
#endif 
 
		// Read chunks from input streams using inverse mapping of stepdown distribution
		read: for (unsigned short output_cycle = 0; output_cycle < FACTOR; output_cycle++)
		{
		#pragma HLS PIPELINE II=ii_out
#ifdef DEBUG_LOG_PRINT
		printf("====================================================================================\n");
        printf("|HLS DEBUG_LOG| aggregatedStream2streamStepup | read cycle: %d \n", pkt*FACTOR + output_cycle);
#endif			
			for (unsigned short out_strm = 0; out_strm < NUM_STRMS; out_strm++)
			{
			#pragma HLS UNROLL
				// Reverse mapping: undo the round-robin distribution from stepdown
				unsigned short chunk_idx = output_cycle * NUM_STRMS + out_strm;
				unsigned short in_strm = chunk_idx / FACTOR;
				unsigned short chunk_k = chunk_idx % FACTOR;
				
				chunks[in_strm][chunk_k] = strm_in[out_strm].read();
#ifdef DEBUG_LOG_PRINT
				printf("[stream_in: %d → chunks[%d][%d]] = (", out_strm, in_strm, chunk_k);
				for (unsigned j = 0; j < STREAM1_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); j++)
				{
					DataConv conv;
					conv.i = chunks[in_strm][chunk_k].range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
					printf("%f%s ", conv.f, j != STREAM1_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8) ? "," : "");
				}
				printf(")\n");
#endif
			}
		}

		// Reconstruct and write wide packets from collected chunks
		write: for (unsigned short num_strm = 0; num_strm < NUM_STRMS; num_strm++)
		{
		#pragma HLS UNROLL
			ap_uint<STREAM2_DATA_WIDTH> out_val = 0;
			for (unsigned short k = 0; k < FACTOR; k++)
			{
			#pragma HLS UNROLL
				out_val.range((k+1) * STREAM1_DATA_WIDTH - 1, k * STREAM1_DATA_WIDTH) = chunks[num_strm][k];
			}
			strm_out[num_strm].write(out_val);
#ifdef DEBUG_LOG_PRINT
			printf("[stream_out: %d] = (", num_strm);
			for (unsigned j = 0; j < STREAM2_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); j++)
			{
				DataConv conv;
				conv.i = out_val.range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
				printf("%f%s ", conv.f, j != STREAM2_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8) ? "," : "");
			}
			printf(")\n");
#endif
		}
	}
}

/**
 * @brief aggregatedStream2streamStepup: Reads smaller chunks from multiple input streams and combines 
 * 		them into larger packets distributed across multiple output streams.
 *
 * @details This function implements the inverse operation of aggregatedStream2streamStepdown. It reads
 * 		data from multiple input streams with lower bit-width (STREAM1_DATA_WIDTH) organized in FACTOR 
 * 		cycles of round-robin distribution. It collects these chunks and reorganizes them back into 
 * 		larger packets with higher bit-width (STREAM2_DATA_WIDTH) for output across NUM_STRMS output streams.
 * 		Effectively converts FACTOR output waves of NUM_STRMS × STREAM1_DATA_WIDTH back into
 * 		NUM_STRMS × STREAM2_DATA_WIDTH per cycle.
 *
 * @tparam STREAM1_DATA_WIDTH The bit-width of the input stream data (must be smaller than STREAM2_DATA_WIDTH).
 * @tparam STREAM2_DATA_WIDTH The bit-width of the output stream data (must be a multiple of STREAM1_DATA_WIDTH).
 * @tparam NUM_STRMS The number of input and output streams. Must be a power of two.
 * @tparam IN_ITR Initiation interval for the outer HLS pipeline combining input chunks (default: 2).
 *
 * @param[in] strm_in Array of input HLS streams (size NUM_STRMS) with bit-width STREAM1_DATA_WIDTH,
 * 		providing chunks distributed across FACTOR cycles per output packet.
 * @param[out] strm_out Array of output HLS streams (size NUM_STRMS) with bit-width STREAM2_DATA_WIDTH,
 * 		receiving the recombined wider packets.
 * @param[in] num_big_pkts The number of large output packets (width STREAM2_DATA_WIDTH) to generate.
 * @param[in] is_small_tile Flag to point small tile_x which is less than NUM_STRMS/2.
 *
 */
template <unsigned short STREAM1_DATA_WIDTH, unsigned short STREAM2_DATA_WIDTH, unsigned short NUM_STRMS, unsigned short IN_ITR=2>
static void aggregatedStream2streamStepup(::hls::stream<ap_uint<STREAM1_DATA_WIDTH>> strm_in[NUM_STRMS],
			::hls::stream<ap_uint<STREAM2_DATA_WIDTH>> strm_out[NUM_STRMS],
			const unsigned int num_big_pkts,
			const bool is_small_tile)
{
	constexpr unsigned short FACTOR = STREAM2_DATA_WIDTH / STREAM1_DATA_WIDTH;
    constexpr unsigned int ii_adj = (IN_ITR >= FACTOR) ? IN_ITR : FACTOR;
	constexpr unsigned short ii_out = ii_adj / FACTOR;
	constexpr unsigned short HALF_FACTOR = FACTOR > 1 ? FACTOR >> 1 : FACTOR;

#ifndef __SYNTHESIS__
    static_assert((NUM_STRMS != 0) && ((NUM_STRMS & (NUM_STRMS - 1)) == 0), "NUM_STRMS must be a power of two");
	static_assert(STREAM2_DATA_WIDTH > STREAM1_DATA_WIDTH,
			"STREAM2_DATA_WIDTH has to be bigger than STREAM1_DATA_WIDTH");
    static_assert(STREAM2_DATA_WIDTH % STREAM1_DATA_WIDTH == 0, 
            "STREAM2_DATA_WIDTH has to be fully divisible by STREAM1_DATA_WIDTH");
    static_assert(STREAM1_DATA_WIDTH % 8 == 0, "STREAM1_DATA_WIDTH should be divisible by 8");
    static_assert(STREAM2_DATA_WIDTH % 8 == 0, "STREAM2_DATA_WIDTH should be divisible by 8");
	static_assert(ii_adj % FACTOR == 0, "Output ii is not divisible by input II");
#endif 


#ifdef DEBUG_LOG_PRINT
		printf("====================================================================================\n");
        printf("|HLS DEBUG_LOG| %s | adj_ii: %d, num_big_pkts: %d\n"
                , __func__, ii_adj , num_big_pkts);
        printf("====================================================================================\n");
#endif

	// Array to collect chunks for reconstruction (indexed by [input_stream][chunk_position])
	ap_uint<STREAM1_DATA_WIDTH> chunks[NUM_STRMS][FACTOR];
	#pragma HLS ARRAY_PARTITION variable=chunks dim=0 complete

	for (unsigned int pkt = 0; pkt < num_big_pkts; pkt++)
	{
	#pragma HLS PIPELINE II=ii_adj

#ifdef DEBUG_LOG_PRINT
		printf("====================================================================================\n");
        printf("|HLS DEBUG_LOG| aggregatedStream2streamStepup | combining pkt: %d \n", pkt);
#endif 
 
		// Read chunks from input streams using inverse mapping of stepdown distribution
		read: if (is_small_tile) {
				for (unsigned short output_cycle = 0; output_cycle < HALF_FACTOR; output_cycle++)
			{
			#pragma HLS PIPELINE II=ii_out
#ifdef DEBUG_LOG_PRINT
			printf("====================================================================================\n");
			printf("|HLS DEBUG_LOG| aggregatedStream2streamStepup (small tile) | read cycle: %d \n", pkt*FACTOR + output_cycle);
#endif			
				for (unsigned short out_strm = 0; out_strm < NUM_STRMS; out_strm++)
				{
				#pragma HLS UNROLL
					// Reverse mapping: undo the round-robin distribution from stepdown
					unsigned short chunk_idx = output_cycle * NUM_STRMS + out_strm;
					unsigned short in_strm = chunk_idx / FACTOR;
					unsigned short chunk_k = chunk_idx % FACTOR;
					
					chunks[in_strm][chunk_k] = strm_in[out_strm].read();
#ifdef DEBUG_LOG_PRINT
					printf("[stream_in: %d → chunks[%d][%d]] = (", out_strm, in_strm, chunk_k);
					for (unsigned j = 0; j < STREAM1_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); j++)
					{
						DataConv conv;
						conv.i = chunks[in_strm][chunk_k].range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
						printf("%f%s ", conv.f, j != STREAM1_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8) ? "," : "");
					}
					printf(")\n");
#endif
				}
			}
		} else {
			for (unsigned short output_cycle = 0; output_cycle < FACTOR; output_cycle++)
			{
			#pragma HLS PIPELINE II=ii_out
#ifdef DEBUG_LOG_PRINT
			printf("====================================================================================\n");
			printf("|HLS DEBUG_LOG| aggregatedStream2streamStepup | read cycle: %d \n", pkt*FACTOR + output_cycle);
#endif			
				for (unsigned short out_strm = 0; out_strm < NUM_STRMS; out_strm++)
				{
				#pragma HLS UNROLL
					// Reverse mapping: undo the round-robin distribution from stepdown
					unsigned short chunk_idx = output_cycle * NUM_STRMS + out_strm;
					unsigned short in_strm = chunk_idx / FACTOR;
					unsigned short chunk_k = chunk_idx % FACTOR;
					
					chunks[in_strm][chunk_k] = strm_in[out_strm].read();
#ifdef DEBUG_LOG_PRINT
					printf("[stream_in: %d → chunks[%d][%d]] = (", out_strm, in_strm, chunk_k);
					for (unsigned j = 0; j < STREAM1_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); j++)
					{
						DataConv conv;
						conv.i = chunks[in_strm][chunk_k].range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
						printf("%f%s ", conv.f, j != STREAM1_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8) ? "," : "");
					}
					printf(")\n");
#endif
				}
			}
		}
		// Reconstruct and write wide packets from collected chunks
		write: for (unsigned short num_strm = 0; num_strm < NUM_STRMS; num_strm++)
		{
		#pragma HLS UNROLL
			ap_uint<STREAM2_DATA_WIDTH> out_val = 0;
			for (unsigned short k = 0; k < FACTOR; k++)
			{
			#pragma HLS UNROLL
				out_val.range((k+1) * STREAM1_DATA_WIDTH - 1, k * STREAM1_DATA_WIDTH) = chunks[num_strm][k];
			}
			strm_out[num_strm].write(out_val);
#ifdef DEBUG_LOG_PRINT
			printf("[stream_out: %d] = (", num_strm);
			for (unsigned j = 0; j < STREAM2_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); j++)
			{
				DataConv conv;
				conv.i = out_val.range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
				printf("%f%s ", conv.f, j != STREAM2_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8) ? "," : "");
			}
			printf(")\n");
#endif
		}
	}
}

/**
 * @brief redirect varient 2: Reorders and aligns strided tile data from multiple input streams to output streams with
 * 		with diferent stream size from the original interleve memory width. 	
 *
 * @details This function acts as a dynamic router and barrel shifter. It decodes a 160-bit 
 * 		configuration command to determine the 3D memory access pattern and the resulting bank 
 * 		wrap-around logic. It selectively reads from a partitioned array of input streams based 
 * 		on boundary conditions, and then combinatorially rotates (barrel shifts) the data 
 * 		to align it correctly across the output streams, compensating for the starting bank offset.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of the original memory source.
 * @tparam STREAM_DATA_WIDTH The bit-width of the stream data elements.
 * @tparam NUM_BANKS The number of memory banks and corresponding streams. Must be a power 
 * of two to enable bitwise modulo masking and efficient barrel shifting.
 * @tparam IN_ITR Initiation interval for the inner HLS pipeline (default: 2).
 *
 * @param[in] strm_in Array of input HLS streams (size NUM_BANKS) providing the raw, 
 * unaligned data read from memory.
 * @param[out] strm_out Array of output HLS streams (size NUM_BANKS) where the correctly 
 * aligned and shifted data will be written.
 * @param[in] command A 160-bit packed configuration word containing:
 * 		- [63:0]   offset_x: Base offset in the X dimension
 * 		- [95:80]  size_x:   Size in the X dimension
 * 		- [111:96] stride_y: Stride in the Y dimension
 * 		- [127:112] size_y:   Size in the Y dimension
 * 		- [143:128] stride_z: Stride in the Z dimension
 * 		- [159:144] size_z:   Size in the Z dimension
 *
 */

template <unsigned short MEM_DATA_WIDTH, unsigned short STREAM_DATA_WIDTH, unsigned short NUM_BANKS, unsigned short IN_ITR=2>
static void redirect(
        ::hls::stream<ap_uint<STREAM_DATA_WIDTH>> strm_in[NUM_BANKS],
        ::hls::stream<ap_uint<STREAM_DATA_WIDTH>> strm_out[NUM_BANKS],
        const ap_uint<160>& command)
{
    // Enforce the power-of-two constraint for bitwise optimizations
#ifndef __SYNTHESIS__
	static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
			"MEM_DATA_WIDTH failed limit check");
	static_assert(STREAM_DATA_WIDTH >= min_axis_data_width && STREAM_DATA_WIDTH <= max_axis_data_width,
			"STREAM_DATA_WIDTH failed limit check");
	static_assert(MEM_DATA_WIDTH % STREAM_DATA_WIDTH == 0, 
            "MEM_DATA_WIDTH must be an exact multiple of STREAM_DATA_WIDTH");
    static_assert((NUM_BANKS != 0) && ((NUM_BANKS & (NUM_BANKS - 1)) == 0), "NUM_BANKS must be a power of two");
#endif 

	constexpr unsigned int pkts_per_beat = MEM_DATA_WIDTH / STREAM_DATA_WIDTH;
    constexpr unsigned int ii_adj = (IN_ITR > pkts_per_beat) ? IN_ITR : pkts_per_beat;
    constexpr unsigned short NUM_BANKS_SHIFT = LOG2(NUM_BANKS);
    constexpr unsigned int BANK_MASK = NUM_BANKS - 1;

    size_t offset_x = command.range(63,0);
    ap_uint<16> size_x = command.range(95,80);
    ap_uint<16> stride_y = command.range(111,96);
    ap_uint<16> size_y = command.range(127,112);
    ap_uint<16> stride_z = command.range(143,128);
    ap_uint<16> size_z = command.range(159,144);
    
    unsigned short size_x_div_by_banks_ceil = (size_x + NUM_BANKS - 1) >> NUM_BANKS_SHIFT;
    unsigned short size_x_div_by_banks_floor = size_x >> NUM_BANKS_SHIFT;
    
    // Optimized modulo using bitwise AND
    unsigned int size_x_mod_banks = size_x & BANK_MASK;

#ifdef DEBUG_LOG_PRINT
    printf("==== DATAMOVER REDIRECT V2 INITIAL PARAMETERS ====\n");
    printf("offset_x: %lu\n", offset_x);
    printf("size_x: %u\n", (unsigned int)size_x);
    printf("stride_y: %u\n", (unsigned int)stride_y);
    printf("size_y: %u\n", (unsigned int)size_y);
    printf("stride_z: %u\n", (unsigned int)stride_z);
    printf("size_z: %u\n", (unsigned int)size_z);
    printf("size_x_div_by_banks_ceil: %u\n", size_x_div_by_banks_ceil);
    printf("size_x_div_by_banks_floor: %u\n", size_x_div_by_banks_floor);
    printf("size_x_mod_banks: %u\n", size_x_mod_banks);
    printf("NUM_BANKS: %u\n", NUM_BANKS);
    printf("========================================\n");
#endif

    for (unsigned short z = 0; z < size_z; z++)
    {
        for (unsigned short y = 0; y < size_y; y++)
        {
            #pragma HLS LOOP_FLATTEN

            const size_t z_offset = offset_x + z * stride_z;
            const size_t abs_offset = z_offset + y * stride_y;
            
            // Replaced modulo with bitwise AND
            const unsigned int starting_bank = abs_offset & BANK_MASK;
            const unsigned int banks_upper_limit = (starting_bank + size_x_mod_banks) & BANK_MASK;
            const bool is_t1_or_t2 = banks_upper_limit < starting_bank;

            // Pre-calculate bank conditions outside the inner pipeline loop
            bool bank_cond[NUM_BANKS];
            #pragma HLS ARRAY_PARTITION variable=bank_cond complete dim=1

            for (unsigned int i = 0; i < NUM_BANKS; i++)
            {
                #pragma HLS UNROLL
                const bool is_gt_sb = i >= starting_bank;
                const bool is_lt_uplim = i < banks_upper_limit;

                if (is_t1_or_t2) {
                    bank_cond[i] = is_gt_sb || is_lt_uplim;
                } else {
                    bank_cond[i] = is_gt_sb && is_lt_uplim;
                }
            }

            for (unsigned short x = 0; x < size_x_div_by_banks_ceil; x++)
            {
                #pragma HLS PIPELINE II=IN_ITR

                // Fully partition these to map to discrete registers
                ap_uint<MEM_DATA_WIDTH> tmp[NUM_BANKS];
                ap_uint<MEM_DATA_WIDTH> swap[NUM_BANKS];
                #pragma HLS ARRAY_PARTITION variable=tmp complete dim=1
                #pragma HLS ARRAY_PARTITION variable=swap complete dim=1

                const bool is_x_lt_bank_x_size = x < size_x_div_by_banks_floor;

                // 1. Read from input streams based on conditions
                for (unsigned int i = 0; i < NUM_BANKS; i++)
                {
                    #pragma HLS UNROLL
                    const bool read_cond = is_x_lt_bank_x_size || bank_cond[i];
		
					for (unsigned int k = 0; k < pkts_per_beat; k++) 
					{
						#pragma HLS UNROLL
						ap_uint<STREAM_DATA_WIDTH> read_val = read_cond ? register_it(strm_in[i].read()) : ap_uint<STREAM_DATA_WIDTH>(0);
						tmp[i].range((k + 1) * STREAM_DATA_WIDTH - 1, k * STREAM_DATA_WIDTH) = read_val;

#ifdef DEBUG_LOG_PRINT
						if (read_cond) {
							printf("==== REDIRECT READ ====\n");
							printf("Bank[%u], Packet[%u], X[%u], Y[%u], Z[%u]\n", i, k, x, (unsigned int)y, (unsigned int)z);
							printf("Values (float): (");
							for (unsigned int j = 0; j < STREAM_DATA_WIDTH / (DEBUG_LOG_SIZE_OF * 8); j++) {
								DataConv conv;
								conv.i = read_val.range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
								if (j > 0) printf(", ");
								printf("%f", conv.f);
							}
							printf(")\n");
							printf("=====================\n");
						}
#endif
					}    
                }

                // 2. The Barrel Shifter 
                for (unsigned int i = 0; i < NUM_BANKS; i++)
                {
                    #pragma HLS UNROLL
                    unsigned int src_idx = (starting_bank + i) & BANK_MASK;
                    swap[i] = tmp[src_idx];
                }

                // 3. Write to output streams
                for (unsigned int i = 0; i < NUM_BANKS; i++)
                {
                    #pragma HLS UNROLL
					for (unsigned int k = 0; k < pkts_per_beat; k++) 
					{
					#pragma HLS UNROLL
						ap_uint<STREAM_DATA_WIDTH> write_val = swap[i].range((k + 1) * STREAM_DATA_WIDTH - 1, k * STREAM_DATA_WIDTH);
						strm_out[i] << write_val;

#ifdef DEBUG_LOG_PRINT
						printf("==== REDIRECT WRITE ====\n");
						printf("Bank[%u], Packet[%u], X[%u], Y[%u], Z[%u]\n", i, k, x, (unsigned int)y, (unsigned int)z);
						printf("Values (float): (");
						for (unsigned int j = 0; j < STREAM_DATA_WIDTH / (DEBUG_LOG_SIZE_OF * 8); j++) {
							DataConv conv;
							conv.i = write_val.range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
							if (j > 0) printf(", ");
							printf("%f", conv.f);
						}
						printf(")\n");
						printf("======================\n");
#endif
					}
                }
            }
        }
    }
}


/**
 * @brief reverseRedirect: Reverses the alignment of strided tile data from input streams back to memory layout order.
 *
 * @details This function acts as the inverse of the `redirect` function. It decodes a 160-bit 
 * configuration command to map memory wrap-around boundaries. It unconditionally reads 
 * buffered data from a partitioned array of input streams, combinatorially rotates 
 * (reverse barrel shifts) the data to undo the starting bank offset alignment, and 
 * selectively writes the ordered data to the output streams based on boundary conditions.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of the data elements in the streams.
 * @tparam NUM_BANKS The number of memory banks and corresponding streams. Must be a power 
 * of two to enable bitwise modulo masking and efficient barrel shifting.
 * @tparam IN_ITR Initiation interval for the inner HLS pipeline (default: 2).
 *
 * @param[in] strm_in Array of input HLS streams (size NUM_BANKS) containing shifted data.
 * @param[out] strm_out Array of output HLS streams (size NUM_BANKS) where correctly 
 * reordered data will be written back out.
 * @param[in] command A 160-bit packed configuration word containing 3D access dimensions.
 *
 * @see ops::hls::redirect
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short NUM_BANKS, unsigned short IN_ITR=2>
static void reverseRedirect(
        ::hls::stream<ap_uint<MEM_DATA_WIDTH>> strm_in[NUM_BANKS],
        ::hls::stream<ap_uint<MEM_DATA_WIDTH>> strm_out[NUM_BANKS],
        const size_t offset_x, const unsigned short size_x, const unsigned short stride_y, const unsigned short size_y, const unsigned short stride_z, const unsigned short size_z)
{
#ifndef __SYNTHESIS__
	static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
			"MEM_DATA_WIDTH failed limit check");
    static_assert((NUM_BANKS != 0) && ((NUM_BANKS & (NUM_BANKS - 1)) == 0), "NUM_BANKS must be a power of two");
#endif 

    const unsigned short NUM_BANKS_SHIFT = LOG2(NUM_BANKS);
    const unsigned int BANK_MASK = NUM_BANKS - 1;

    // size_t offset_x = command.range(63,0);
    // ap_uint<16> size_x = command.range(95,80);
    // ap_uint<16> stride_y = command.range(111,96);
    // ap_uint<16> size_y = command.range(127,112);
    // ap_uint<16> stride_z = command.range(143,128);
    // ap_uint<16> size_z = command.range(159,144);
    
    unsigned short size_x_div_by_banks_ceil = (size_x + NUM_BANKS - 1) >> NUM_BANKS_SHIFT;
    unsigned short size_x_div_by_banks_floor = size_x >> NUM_BANKS_SHIFT;
    
    unsigned int size_x_mod_banks = size_x & BANK_MASK; // size_x % NUM_BANKS

#ifdef DEBUG_LOG_PRINT
    printf("==== DATAMOVER REVERSE REDIRECT INITIAL PARAMETERS ====\n");
    printf("offset_x: %lu\n", offset_x);
    printf("size_x: %u\n", (unsigned int)size_x);
    printf("stride_y: %u\n", (unsigned int)stride_y);
    printf("size_y: %u\n", (unsigned int)size_y);
    printf("stride_z: %u\n", (unsigned int)stride_z);
    printf("size_z: %u\n", (unsigned int)size_z);
    printf("size_x_div_by_banks_ceil: %u\n", size_x_div_by_banks_ceil);
    printf("size_x_div_by_banks_floor: %u\n", size_x_div_by_banks_floor);
    printf("size_x_mod_banks: %u\n", size_x_mod_banks);
    printf("NUM_BANKS: %u\n", NUM_BANKS);
    printf("========================================\n");
#endif

    for (unsigned short z = 0; z < size_z; z++)
    {
        for (unsigned short y = 0; y < size_y; y++)
        {
            #pragma HLS LOOP_FLATTEN

            const size_t z_offset = offset_x + z * stride_z;
            const size_t abs_offset = z_offset + y * stride_y;
            
            const unsigned int starting_bank = abs_offset & BANK_MASK;
            const unsigned int banks_upper_limit = (starting_bank + size_x_mod_banks) & BANK_MASK;
            const bool is_t1_or_t2 = banks_upper_limit < starting_bank;

            // Pre-calculate boundary write conditions
            bool bank_cond[NUM_BANKS];
            #pragma HLS ARRAY_PARTITION variable=bank_cond complete dim=1

            for (unsigned int i = 0; i < NUM_BANKS; i++)
            {
                #pragma HLS UNROLL
                const bool is_gt_sb = i >= starting_bank;
                const bool is_lt_uplim = i < banks_upper_limit;

                if (is_t1_or_t2) {
                    bank_cond[i] = is_gt_sb || is_lt_uplim;
                } else {
                    bank_cond[i] = is_gt_sb && is_lt_uplim;
                }
            }

            for (unsigned short x = 0; x < size_x_div_by_banks_ceil; x++)
            {
                #pragma HLS PIPELINE II=IN_ITR

                ap_uint<MEM_DATA_WIDTH> tmp[NUM_BANKS];
                ap_uint<MEM_DATA_WIDTH> swap[NUM_BANKS];
                #pragma HLS ARRAY_PARTITION variable=tmp complete dim=1
                #pragma HLS ARRAY_PARTITION variable=swap complete dim=1

                const bool is_x_lt_bank_x_size = x < size_x_div_by_banks_floor;

                // 1. Unconditional Read
                for (unsigned int i = 0; i < NUM_BANKS; i++)
                {
                    #pragma HLS UNROLL
                    tmp[i] = register_it(strm_in[i].read());
#ifdef DEBUG_LOG_PRINT
                    printf("==== REVERSE REDIRECT READ ====\n");
                    printf("Bank[%u], X[%u], Y[%u], Z[%u]\n", i, x, (unsigned int)y, (unsigned int)z);
                    printf("Values (float): (");
                    for (unsigned int j = 0; j < MEM_DATA_WIDTH / (DEBUG_LOG_SIZE_OF * 8); j++) {
                        DataConv conv;
                        conv.i = tmp[i].range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
                        if (j > 0) printf(", ");
                        printf("%f", conv.f);
                    }
                    printf(")\n");
                    printf("============================\n");
#endif
                }

                // 2. Reverse Barrel Shifter
                for (unsigned int i = 0; i < NUM_BANKS; i++)
                {
                    #pragma HLS UNROLL
                    unsigned int src_idx = (i - starting_bank + NUM_BANKS) & BANK_MASK;
                    swap[i] = tmp[src_idx];
                }

                // 3. Conditional Write to Outputs
                for (unsigned int i = 0; i < NUM_BANKS; i++)
                {
                    #pragma HLS UNROLL
                    const bool write_cond = is_x_lt_bank_x_size || bank_cond[i];
                    
                    if (write_cond) {
                        strm_out[i] << swap[i];
#ifdef DEBUG_LOG_PRINT
                        printf("==== REVERSE REDIRECT WRITE ====\n");
                        printf("Bank[%u], X[%u], Y[%u], Z[%u]\n", i, x, (unsigned int)y, (unsigned int)z);
                        printf("Values (float): (");
                        for (unsigned int j = 0; j < MEM_DATA_WIDTH / (DEBUG_LOG_SIZE_OF * 8); j++) {
                            DataConv conv;
                            conv.i = swap[i].range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
                            if (j > 0) printf(", ");
                            printf("%f", conv.f);
                        }
                        printf(")\n");
                        printf("===============================\n");
#endif
                    }
                }
            }
        }
    }
}

/**
 * @brief reverseRedirectV2: Reverses the alignment of strided tile data from input streams back to memory layout order.
 *
 * @details This function acts as the inverse of the `redirect` function. It decodes a 160-bit 
 * configuration command to map memory wrap-around boundaries. It unconditionally reads 
 * buffered data from a partitioned array of input streams, combinatorially rotates 
 * (reverse barrel shifts) the data to undo the starting bank offset alignment, and 
 * selectively writes the ordered data to the output streams based on boundary conditions.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of the data elements in the streams.
 * @tparam NUM_BANKS The number of memory banks and corresponding streams. Must be a power 
 * of two to enable bitwise modulo masking and efficient barrel shifting.
 * @tparam IN_ITR Initiation interval for the inner HLS pipeline (default: 2).
 *
 * @param[in] strm_in Array of input HLS streams (size NUM_BANKS) containing shifted data.
 * @param[out] strm_out Array of output HLS streams (size NUM_BANKS) where correctly 
 * reordered data will be written back out.
 * @param[in] command A 160-bit packed configuration word containing 3D access dimensions.
 *
 * @see ops::hls::redirect
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short NUM_BANKS, unsigned short IN_ITR=2>
static void reverseRedirectV2(
        ::hls::stream<ap_uint<MEM_DATA_WIDTH>> strm_in[NUM_BANKS],
        ::hls::stream<ap_uint<MEM_DATA_WIDTH>> strm_out[NUM_BANKS],
        const unsigned short bank_tile_size_x_floor, const unsigned short bank_tile_size_x_ceil,  const ap_uint<NUM_BANKS> big_bank_tile_size_x_mask, const ap_uint<LOG2(NUM_BANKS)> big_offset_x_banks_upper, 
		/*const unsigned short size_y, const unsigned short size_z*/ const unsigned int total_rows)
{
#ifndef __SYNTHESIS__
	static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
			"MEM_DATA_WIDTH failed limit check");
    static_assert((NUM_BANKS != 0) && ((NUM_BANKS & (NUM_BANKS - 1)) == 0), "NUM_BANKS must be a power of two");
#endif 

    const unsigned short NUM_BANKS_SHIFT = LOG2(NUM_BANKS);
    const unsigned int BANK_MASK = NUM_BANKS - 1;

    // size_t offset_x = command.range(63,0);
    // ap_uint<16> size_x = command.range(95,80);
    // ap_uint<16> stride_y = command.range(111,96);
    // ap_uint<16> size_y = command.range(127,112);
    // ap_uint<16> stride_z = command.range(143,128);
    // ap_uint<16> size_z = command.range(159,144);
    
    // unsigned short size_x_div_by_banks_ceil = bank_size_x_floor + 1;
    // unsigned short size_x_div_by_banks_floor = size_x >> NUM_BANKS_SHIFT;
    
    // unsigned int size_x_mod_banks = size_x & BANK_MASK; // size_x % NUM_BANKS

#ifdef DEBUG_LOG_PRINT
    printf("==== DATAMOVER REDIRECT INITIAL PARAMETERS ====\n");
    printf("bank_tile_size_x_floor: %lu\n", bank_tile_size_x_floor);
	printf("bank_tile_size_x_ceil: %lu\n", bank_tile_size_x_ceil);
    printf("big_bank_tile_size_x_mask: {");
	for (ap_uint<LOG2(NUM_BANKS)+1> b_id = 0; b_id < NUM_BANKS; b_id++) {
		printf("%d", (unsigned short)big_bank_tile_size_x_mask[b_id]);
		if (b_id != NUM_BANKS - 1) 
            printf(", ");
	}
	printf("}\n");
    // printf("size_y: %u\n", (unsigned int)size_y);
    // printf("size_z: %u\n", (unsigned int)size_z);
	printf("total_rows: %d\n", (unsigned int)total_rows);
    printf("NUM_BANKS: %u\n", NUM_BANKS);
    printf("========================================\n");
#endif

    for (unsigned int row_id = 0; row_id < total_rows; row_id++)
    {
		#pragma HLS LOOP_FLATTEN
		const ap_uint<NUM_BANKS_SHIFT> starting_bank = big_offset_x_banks_upper;
		// const size_t z_offset = offset_x + z * stride_z;
		// const size_t abs_offset = z_offset + y * stride_y;
		
		// const unsigned int starting_bank = abs_offset & BANK_MASK;
		// const unsigned int banks_upper_limit = (starting_bank + size_x_mod_banks) & BANK_MASK;
		// const bool is_t1_or_t2 = banks_upper_limit < starting_bank;

		// Pre-calculate boundary write conditions
		// bool bank_cond[NUM_BANKS];
		// #pragma HLS ARRAY_PARTITION variable=bank_cond complete dim=1

		// for (unsigned int i = 0; i < NUM_BANKS; i++)
		// {
		//     #pragma HLS UNROLL
		//     const bool is_gt_sb = i >= starting_bank;
		//     const bool is_lt_uplim = i < banks_upper_limit;

		//     if (is_t1_or_t2) {
		//         bank_cond[i] = is_gt_sb || is_lt_uplim;
		//     } else {
		//         bank_cond[i] = is_gt_sb && is_lt_uplim;
		//     }
		// }

		for (unsigned short x = 0; x < bank_tile_size_x_ceil; x++)
		{
			#pragma HLS PIPELINE II=IN_ITR

			ap_uint<MEM_DATA_WIDTH> tmp[NUM_BANKS];
			ap_uint<MEM_DATA_WIDTH> swap[NUM_BANKS];
			#pragma HLS ARRAY_PARTITION variable=tmp complete dim=1
			#pragma HLS ARRAY_PARTITION variable=swap complete dim=1

			const bool is_x_lt_bank_x_size = x < bank_tile_size_x_floor;

			// 1. Unconditional Read
			for (unsigned int i = 0; i < NUM_BANKS; i++)
			{
				#pragma HLS UNROLL
				tmp[i] = register_it(strm_in[i].read());
#ifdef DEBUG_LOG_PRINT
				printf("==== REVERSE REDIRECT READ ====\n");
				printf("Bank[%u], X[%u], ROW_ID[%u] \n", i, x, (unsigned int)row_id);
				printf("Values (float): (");
				for (unsigned int j = 0; j < MEM_DATA_WIDTH / (DEBUG_LOG_SIZE_OF * 8); j++) {
					DataConv conv;
					conv.i = tmp[i].range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
					if (j > 0) printf(", ");
					printf("%f", conv.f);
				}
				printf(")\n");
				printf("============================\n");
#endif
			}

			// 2. Reverse Barrel Shifter
			for (unsigned int i = 0; i < NUM_BANKS; i++)
			{
				#pragma HLS UNROLL
				unsigned int src_idx = (i - starting_bank + NUM_BANKS) & BANK_MASK;
				swap[i] = tmp[src_idx];
			}

			// 3. Conditional Write to Outputs
			for (unsigned int i = 0; i < NUM_BANKS; i++)
			{
				#pragma HLS UNROLL
				const bool write_cond = is_x_lt_bank_x_size || big_bank_tile_size_x_mask.range(i,i);
				
				if (write_cond) {
					strm_out[i] << swap[i];
#ifdef DEBUG_LOG_PRINT
					printf("==== REVERSE REDIRECT WRITE ====\n");
					printf("Bank[%u], X[%u], ROW_ID[%u]\n", i, x, (unsigned int)row_id);
					printf("Values (float): (");
					for (unsigned int j = 0; j < MEM_DATA_WIDTH / (DEBUG_LOG_SIZE_OF * 8); j++) {
						DataConv conv;
						conv.i = swap[i].range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
						if (j > 0) printf(", ");
						printf("%f", conv.f);
					}
					printf(")\n");
					printf("===============================\n");
#endif
				}
			}
		}     
    }
}

/**
 * @brief reverseRedirect variant-2: Reverses the alignment of strided tile data from input streams back to memory layout order.
 *
 * @details This function acts as the inverse of the `redirect` function. It decodes a 160-bit 
 * configuration command to map memory wrap-around boundaries. It unconditionally reads 
 * buffered data from a partitioned array of input streams, combinatorially rotates 
 * (reverse barrel shifts) the data to undo the starting bank offset alignment, and 
 * selectively writes the ordered data to the output streams based on boundary conditions.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of the original memory source.
 * @tparam STREAM_DATA_WIDTH The bit-width of the stream data elements.
 * @tparam NUM_BANKS The number of memory banks and corresponding streams. Must be a power 
 * of two to enable bitwise modulo masking and efficient barrel shifting.
 * @tparam IN_ITR Initiation interval for the inner HLS pipeline (default: 2).
 *
 * @param[in] strm_in Array of input HLS streams (size NUM_BANKS) containing shifted data.
 * @param[out] strm_out Array of output HLS streams (size NUM_BANKS) where correctly 
 * reordered data will be written back out.
 * @param[in] command A 160-bit packed configuration word containing 3D access dimensions.
 *
 * @see ops::hls::redirect
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short STREAM_DATA_WIDTH, unsigned short NUM_BANKS, unsigned short IN_ITR=2>
static void reverseRedirect(
        ::hls::stream<ap_uint<STREAM_DATA_WIDTH>> strm_in[NUM_BANKS],
        ::hls::stream<ap_uint<STREAM_DATA_WIDTH>> strm_out[NUM_BANKS],
        const ap_uint<160>& command)
{
    // Enforce power-of-two constraint
#ifndef __SYNTHESIS__
	static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
			"MEM_DATA_WIDTH failed limit check");
	static_assert(STREAM_DATA_WIDTH >= min_axis_data_width && STREAM_DATA_WIDTH <= max_axis_data_width,
			"STREAM_DATA_WIDTH failed limit check");
	static_assert(MEM_DATA_WIDTH % STREAM_DATA_WIDTH == 0, 
            "MEM_DATA_WIDTH must be an exact multiple of STREAM_DATA_WIDTH");
    static_assert((NUM_BANKS != 0) && ((NUM_BANKS & (NUM_BANKS - 1)) == 0), "NUM_BANKS must be a power of two");
#endif 

    constexpr unsigned int pkts_per_beat = MEM_DATA_WIDTH / STREAM_DATA_WIDTH;
    constexpr unsigned int ii_adj = (IN_ITR > pkts_per_beat) ? IN_ITR : pkts_per_beat;
    constexpr unsigned short NUM_BANKS_SHIFT = LOG2(NUM_BANKS);
    constexpr unsigned int BANK_MASK = NUM_BANKS - 1;

    size_t offset_x = command.range(63,0);
    ap_uint<16> size_x = command.range(95,80);
    ap_uint<16> stride_y = command.range(111,96);
    ap_uint<16> size_y = command.range(127,112);
    ap_uint<16> stride_z = command.range(143,128);
    ap_uint<16> size_z = command.range(159,144);
    
    unsigned short size_x_div_by_banks_ceil = (size_x + NUM_BANKS - 1) >> NUM_BANKS_SHIFT;
    unsigned short size_x_div_by_banks_floor = size_x >> NUM_BANKS_SHIFT;
    
    unsigned int size_x_mod_banks = size_x & BANK_MASK; // size_x % NUM_BANKS

#ifdef DEBUG_LOG_PRINT
    printf("==== DATAMOVER REVERSE REDIRECT V2 INITIAL PARAMETERS ====\n");
    printf("offset_x: %lu\n", offset_x);
    printf("size_x: %u\n", (unsigned int)size_x);
    printf("stride_y: %u\n", (unsigned int)stride_y);
    printf("size_y: %u\n", (unsigned int)size_y);
    printf("stride_z: %u\n", (unsigned int)stride_z);
    printf("size_z: %u\n", (unsigned int)size_z);
    printf("size_x_div_by_banks_ceil: %u\n", size_x_div_by_banks_ceil);
    printf("size_x_div_by_banks_floor: %u\n", size_x_div_by_banks_floor);
    printf("size_x_mod_banks: %u\n", size_x_mod_banks);
    printf("NUM_BANKS: %u\n", NUM_BANKS);
    printf("pkts_per_beat: %u\n", (unsigned int)pkts_per_beat);
    printf("========================================\n");
#endif

    for (unsigned short z = 0; z < size_z; z++)
    {
        for (unsigned short y = 0; y < size_y; y++)
        {
            #pragma HLS LOOP_FLATTEN

            const size_t z_offset = offset_x + z * stride_z;
            const size_t abs_offset = z_offset + y * stride_y;
            
            const unsigned int starting_bank = abs_offset & BANK_MASK;
            const unsigned int banks_upper_limit = (starting_bank + size_x_mod_banks) & BANK_MASK;
            const bool is_t1_or_t2 = banks_upper_limit < starting_bank;

            // Pre-calculate boundary write conditions
            bool bank_cond[NUM_BANKS];
            #pragma HLS ARRAY_PARTITION variable=bank_cond complete dim=1

            for (unsigned int i = 0; i < NUM_BANKS; i++)
            {
                #pragma HLS UNROLL
                const bool is_gt_sb = i >= starting_bank;
                const bool is_lt_uplim = i < banks_upper_limit;

                if (is_t1_or_t2) {
                    bank_cond[i] = is_gt_sb || is_lt_uplim;
                } else {
                    bank_cond[i] = is_gt_sb && is_lt_uplim;
                }
            }

            for (unsigned short x = 0; x < size_x_div_by_banks_ceil; x++)
            {
                #pragma HLS PIPELINE II=IN_ITR

                ap_uint<MEM_DATA_WIDTH> tmp[NUM_BANKS];
                ap_uint<MEM_DATA_WIDTH> swap[NUM_BANKS];
                #pragma HLS ARRAY_PARTITION variable=tmp complete dim=1
                #pragma HLS ARRAY_PARTITION variable=swap complete dim=1

                const bool is_x_lt_bank_x_size = x < size_x_div_by_banks_floor;

                // 1. Unconditional Read
                for (unsigned int i = 0; i < NUM_BANKS; i++)
                {
                    #pragma HLS UNROLL
					for (unsigned int k = 0; k < pkts_per_beat; k++) 
					{
						#pragma HLS UNROLL
						ap_uint<STREAM_DATA_WIDTH> read_val = register_it(strm_in[i].read());
						tmp[i].range((k + 1) * STREAM_DATA_WIDTH - 1, k * STREAM_DATA_WIDTH) = read_val;
#ifdef DEBUG_LOG_PRINT
                            printf("==== REVERSE REDIRECT READ ====\n");
                            printf("Bank[%u], Packet[%u], X[%u], Y[%u], Z[%u]\n", i, k, x, (unsigned int)y, (unsigned int)z);
                            printf("Values (float): (");
                            for (unsigned int j = 0; j < STREAM_DATA_WIDTH / (DEBUG_LOG_SIZE_OF * 8); j++) {
                                DataConv conv;
                                conv.i = read_val.range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
                                if (j > 0) printf(", ");
                                printf("%f", conv.f);
                            }
                            printf(")\n");
                            printf("============================\n");
#endif
					}
                }

                // 2. Reverse Barrel Shifter
                for (unsigned int i = 0; i < NUM_BANKS; i++)
                {
                    #pragma HLS UNROLL
                    unsigned int src_idx = (i - starting_bank + NUM_BANKS) & BANK_MASK;
                    swap[i] = tmp[src_idx];
                }

                // 3. Conditional Write to Outputs
                for (unsigned int i = 0; i < NUM_BANKS; i++)
                {
                    #pragma HLS UNROLL
                    const bool write_cond = is_x_lt_bank_x_size || bank_cond[i];
                    
					for (unsigned int k = 0; k < pkts_per_beat; k++) 
					{
						#pragma HLS UNROLL
                    	if (write_cond) {
                        ap_uint<STREAM_DATA_WIDTH> write_val = swap[i].range((k + 1) * STREAM_DATA_WIDTH - 1, k * STREAM_DATA_WIDTH);
                        strm_out[i] << write_val;
#ifdef DEBUG_LOG_PRINT
                        printf("==== REVERSE REDIRECT WRITE ====\n");
                        printf("Bank[%u], Packet[%u], X[%u], Y[%u], Z[%u]\n", i, k, x, (unsigned int)y, (unsigned int)z);
                        printf("Values (float): (");
                        for (unsigned int j = 0; j < STREAM_DATA_WIDTH / (DEBUG_LOG_SIZE_OF * 8); j++) {
                            DataConv conv;
                            conv.i = write_val.range((j+1) * DEBUG_LOG_SIZE_OF * 8 - 1, j * DEBUG_LOG_SIZE_OF * 8);
                            if (j > 0) printf(", ");
                            printf("%f", conv.f);
                        }
                        printf(")\n");
                        printf("===============================\n");
#endif
                    	}
                    }
                }
            }
        }
    }
}

// template<std::size_t I = 0, unsigned short MEM_DATA_WIDTH, unsigned short BURST_SIZE, unsigned short IN_ITR, typename... Tp>
// inline typename std::enable_if<I == sizeof...(Tp), void>::type
// call_mem2stream(unsigned short target, const std::tuple<Tp...>& t, ::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm, unsigned int offset, unsigned short len) {
//     // Base case: do nothing
// }

// template<std::size_t I = 0, unsigned short MEM_DATA_WIDTH, unsigned short BURST_SIZE, unsigned short IN_ITR, typename... Tp>
// inline typename std::enable_if<I < sizeof...(Tp), void>::type
// call_mem2stream(unsigned short target, const std::tuple<Tp...>& t, ::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm, unsigned int offset, unsigned short len) {
//     if (target == I) {
//         // HLS will resolve this std::get<I> at compile time for each branch
//         mem2stream<MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>((ap_uint<MEM_DATA_WIDTH>*)(std::get<I>(t) + offset), strm, len);
//     } else {
//         call_mem2stream<I + 1, MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>(target, t, strm, offset, len);
//     }
// }

// template <unsigned short MEM_DATA_WIDTH, unsigned short NUM_BANKS, unsigned short BURST_SIZE=32, unsigned short IN_ITR=2, typename... BUFF_TYPE>
// static void stridedTileMem2stream(::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_out, const ops::hls::MemConfigTile& config, BUFF_TYPE*... mem_in)
// {
//     // #pragma HLS INLINE off
//     #ifdef DEBUG_LOG_PRINT
//         printf("|HLS DEBUG_LOG|%s| reading tile. tile_start:%d, tile_size:%d, stride_start:%d\n", __func__, config.start_offset, config.total_size_bytes, stride_start);
//     #endif

//     auto banks = std::forward_as_tuple(mem_in...);
//     const unsigned short z_diff = config.end_z - config.start_z;
//     const unsigned short tile_size_y_mul_z_diff = config.tile_size_y * z_diff;
//     const unsigned short last_tile_size_y_mul_z_diff = config.last_tile_size_y * z_diff;

//     for (unsigned short tile_y = 0; tile_y < config.tile_count_y; tile_y++)
//     {
//         const unsigned short tile_size_y = tile_y == (config.tile_count_y -1) ? config.last_tile_size_y : config.tile_size_y;
//         const unsigned short realized_tile_size_y_mul_z_diff = tile_y == (config.tile_count_y -1) ? last_tile_size_y_mul_z_diff : tile_size_y_mul_z_diff;
//         const unsigned int tile_y_offset = tile_y * config.effective_tile_size_y * config.grid_xblocks;
//         // const unsigned int abs_row_id_y_offset = config.tile_count_x * tile_y * tile_size_y_mul_z_diff;

//         for (unsigned short tile_x = 0; tile_x < config.tile_count_x; tile_x++)
//         {
//             const unsigned int abs_row_id_x_offset = tile_x * realized_tile_size_y_mul_z_diff;
//             const unsigned int tile_x_offset = tile_x * config.effective_tile_size_x; 

//             for (unsigned short k = 0; k < z_diff; k++)
//             {
//                 // const unsigned int abs_row_id_y_offset_k = k * realized_tile_size_y_mul_z_diff;
//                 const unsigned int k_offset = k * config.grid_xblocks * config.grid_size_y;

//                 for (unsigned short j = 0; j < tile_size_y/NUM_BANKS; j+= NUM_BANKS)
//                 {
//                     // #pragma HLS PIPELINE // TODO: Check whether this is effective or not. Most probaly not required as mem2stream already has PIPELINE pragma inside.
//                     const unsigned short tile_size_x = tile_x == (config.tile_count_x -1) ? config.last_tile_size_x : config.tile_size_x;
//                     unsigned int offset_1 = config.start_offset + tile_x_offset;
//                     unsigned int offset_2 = k_offset + tile_y_offset;
//                     unsigned int offset_3 = offset_1 + offset_2;
//                     unsigned int j_offset  = j * config.grid_xblocks;
//                     unsigned int offset = offset_3 + j_offset;

//                     #ifdef DEBUG_LOG_PRINT
//                         printf("|HLS DEBUG_LOG|%s| offset_1:%u offset_2:%u offset_3:%u j_offset:%u offset:%u tile_y:%u tile_x:%u k:%u j:%u tile_size_x:%u\n",
//                                __func__,
//                                (unsigned int)offset_1,
//                                (unsigned int)offset_2,
//                                (unsigned int)offset_3,
//                                (unsigned int)j_offset,
//                                (unsigned int)offset,
//                                (unsigned int)tile_y,
//                                (unsigned int)tile_x,
//                                (unsigned int)k,
//                                (unsigned int)j,
//                                (unsigned int)tile_size_x);
//                     #endif
//                     for (unsigned short b = 0; b  < NUM_BANKS; b++)
//                     {
//                     // #pragma HLS UNROLL type=complete
//                         unsigned short ad_j = j * NUM_BANKS + b;
//                         if (ad_j < tile_size_y) 
//                             call_mem2stream<0, MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>(b, banks, strm_out, offset, tile_size_x);
//                     }
//                 }
//             }
//         }
//     }
// }

/**
 * @brief Writes strided tile data from a stream to a specific memory bank with 2D boundary avoidance (X-tiling only).
 *
 * @details This function acts as the optimized write-side Address Generation Unit (AGU) for a physical memory bank 
 * within a 2D tiled layout. It calculates the specific `avoid_x` boundary size dynamically per-tile to eliminate 
 * redundant writes of overlap (ghost) zones. Because 2D grids are only tiled on the X-axis, the Y-axis is traversed 
 * contiguously based on the subset of rows (`total_rows`) mathematically assigned to this `BANK_ID`. This ensures 
 * perfect AXI write bursts without branching penalties inside the inner loop.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of each memory access (e.g., 64, 128, 256).
 * @tparam NUM_BANKS The total number of interleaved memory banks. Must be a power of two.
 * @tparam BANK_ID The logical ID of the writing bank. Used at compile-time to determine local row bounds.
 * @tparam BURST_SIZE Maximum AXI burst size for memory transfers (default: 32).
 * @tparam IN_ITR Initiation interval for the pipeline (default: 2).
 *
 * @param[in] strm_in Reference to the input HLS stream returning processed data from the compute PE.
 * @param[out] mem_out Pointer to the specific physical AXI memory bank where data will be written.
 * @param[in] config Reference to MemConfigTile configuration containing:
 * - start_offset: Base offset in memory
 * - tile_size_y: Total size of the Y dimension (treated as full grid_size_y in 2D)
 * - tile_count_x: Number of tiles in the X dimension
 * - effective_tile_size_x: Effective X-tile dimension for offset calculation
 * - grid_xblocks: Grid dimension for contiguous memory stride calculation
 * - tile_overlap_size_x: Overlap size used to compute avoid_x boundaries
 *
 * @note Supports non-uniform tile sizes at the grid boundaries via `last_tile_size_x`.
 * @note Debug logging is actively supported when `DEBUG_LOG_PRINT` is defined.
 *
 * @see ops::hls::MemConfigTile
 * @see ops::hls::stream2memWithAvoidV2
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short NUM_BANKS=2, unsigned short BANK_ID=0, unsigned short BURST_SIZE=32, unsigned short IN_ITR=2>
static void stridedTileStream2mem2D(::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in, ap_uint<MEM_DATA_WIDTH>* mem_out, const ops::hls::MemConfigTile& config)
{
    #ifdef DEBUG_LOG_PRINT
    printf("|HLS DEBUG_LOG|%s|bank-%u| Initializing 2D write (X-tiling only). start_offset:%u, total_size_bytes:%u, tile_count_x:%u, total_y_rows:%u\n", 
           __func__, BANK_ID,config.start_offset, config.total_size_bytes, config.tile_count_x, config.tile_size_y);
    #endif

	constexpr unsigned short BANK_SHIFT = LOG2(NUM_BANKS);
	constexpr unsigned short BANK_MASK = NUM_BANKS - 1;

	const bool is_additional_row = (config.grid_size_y & BANK_MASK) > BANK_ID;
	const unsigned short total_rows = (config.grid_size_y >> BANK_SHIFT) + (is_additional_row ? 1 : 0);

    for (unsigned short tile_x = 0; tile_x < config.tile_count_x; tile_x++)
    {
		const unsigned short avoid_x = tile_x == 0 ? 0 : config.tile_overlap_size_x >> 1;
        const unsigned int tile_x_offset = tile_x * config.effective_tile_size_x; 
        const unsigned short tile_size_x = tile_x == (config.tile_count_x -1) ? config.last_tile_size_x : config.tile_size_x;

        // Traverse the entire Y dimension, dynamically striding by NUM_BANKS
        for (unsigned short j = 0; j < total_rows; j++)
        {
            unsigned int offset_1 = config.start_offset + tile_x_offset;
            unsigned int j_offset  = j * config.grid_xblocks;
            unsigned int offset = offset_1 + j_offset;
            
        #ifdef DEBUG_LOG_PRINT
            printf("|HLS DEBUG_LOG|%s| offset_1:%u j_offset:%u final_offset:%u | tile_x_id:%u j_row:%u avoid_x: %u, tile_size_x:%u\n",
                        __func__, offset_1, j_offset, offset, (unsigned int)tile_x, (unsigned int)j, (unsigned int)avoid_x, (unsigned int)tile_size_x);
        #endif
            stream2memWithAvoidV2<MEM_DATA_WIDTH, IN_ITR>((ap_uint<MEM_DATA_WIDTH>* )(mem_out + offset), strm_in, tile_size_x, avoid_x);
        }
    }
}

/**
 * @brief Writes strided tile data from a stream to memory with 3D tiling and 2D boundary avoidance.
 *
 * @details This function acts as the optimized write-side Address Generation Unit (AGU) for a physical memory bank 
 * within a 3D tiled layout. It calculates boundary avoidance dynamically for both the X and Y dimensions to prevent 
 * writing overlapping ghost zones to memory. If the current local row `j` falls within the `local_avoid_y` threshold, 
 * it forces `current_avoid_x` to equal `tile_size_x`, ensuring the stream is drained for that row without writing 
 * any elements to memory.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of each memory access (e.g., 64, 128, 256).
 * @tparam NUM_BANKS The total number of interleaved memory banks. Must be a power of two.
 * @tparam BANK_ID The logical ID of the writing bank. Used at compile-time to determine local bounds.
 * @tparam BURST_SIZE Maximum AXI burst size for memory transfers (default: 32).
 * @tparam IN_ITR Initiation interval for the pipeline (default: 2).
 *
 * @param[in] strm_in Reference to the input HLS stream returning processed data from the compute PE.
 * @param[out] mem_out Pointer to the specific physical AXI memory bank where data will be written.
 * @param[in] config Reference to MemConfigTile configuration containing:
 * - start_offset: Base offset in memory
 * - start_z, end_z: Z-dimension iteration range
 * - tile_size_y, last_tile_size_y: Y-tile dimensions
 * - tile_count_x, tile_count_y: Number of tiles in each dimension
 * - effective_tile_size_x, effective_tile_size_y: Effective dimensions for offset calculation
 * - grid_xblocks, grid_size_y: Grid dimensions for stride calculation
 * - tile_overlap_size_x, tile_overlap_size_y: Overlap sizes used to compute avoid bounds
 *
 * @note Supports non-uniform tile sizes at the grid boundaries.
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short NUM_BANKS=2, unsigned short BANK_ID=0, unsigned short BURST_SIZE=32, unsigned short IN_ITR=2>
static void stridedTileStream2mem3D(::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in, ap_uint<MEM_DATA_WIDTH>* mem_out, const ops::hls::MemConfigTile& config)
{
    constexpr unsigned short BANK_SHIFT = LOG2(NUM_BANKS);
    constexpr unsigned short BANK_MASK = NUM_BANKS - 1;
    
    const bool is_additional_row = (config.grid_size_y & BANK_MASK) > BANK_ID;
    const unsigned short bank_grid_size_y = (config.grid_size_y >> BANK_SHIFT) + (is_additional_row ? 1 : 0);
    const unsigned int bank_z_stride = bank_grid_size_y * config.grid_xblocks;
    const unsigned short z_diff = config.end_z - config.start_z;

    #ifdef DEBUG_LOG_PRINT
    printf("|HLS DEBUG_LOG|%s|bank-%u| Initializing 3D write. start_offset:%u, tile_count_y:%u, tile_count_x:%u, z_diff:%u\n", 
           __func__, BANK_ID, config.start_offset, config.tile_count_y, config.tile_count_x, z_diff);
    #endif

    for (unsigned short tile_y = 0; tile_y < config.tile_count_y; tile_y++)
    {
        unsigned int global_y_start = tile_y * config.effective_tile_size_y;
        ap_uint<BANK_SHIFT> start_bank = global_y_start;
        ap_uint<BANK_SHIFT> bank_offset_in_tile = BANK_ID - start_bank; 

        // 1. Calculate how many global avoid rows physically reside in this bank
        const unsigned short global_avoid_y = (tile_y == 0) ? 0 : (config.tile_overlap_size_y >> 1);
        const unsigned short avoid_remainder = global_avoid_y & BANK_MASK;
        const bool avoid_is_additional = bank_offset_in_tile < avoid_remainder;
        const unsigned short local_avoid_y = (global_avoid_y >> BANK_SHIFT) + (avoid_is_additional ? 1 : 0);

        const unsigned short tile_size_y = (tile_y == config.tile_count_y - 1) ? config.last_tile_size_y : config.tile_size_y;
        
        // 2. Resolve the local boundary for tile_y inside this bank
        const unsigned short remainder = tile_size_y & BANK_MASK;
        const bool tile_is_additional_row = bank_offset_in_tile < remainder;
        const unsigned short local_tile_size_y = (tile_size_y >> BANK_SHIFT) + (tile_is_additional_row ? 1 : 0);
        
        // Physical offset for this tile in the specific memory bank's Y dimension
        const unsigned int local_tile_y_offset = ((global_y_start + bank_offset_in_tile) >> BANK_SHIFT) * config.grid_xblocks;

        for (unsigned short tile_x = 0; tile_x < config.tile_count_x; tile_x++)
        {
            const unsigned short base_avoid_x = tile_x == 0 ? 0 : config.tile_overlap_size_x >> 1;
            
            const unsigned int tile_x_offset = tile_x * config.effective_tile_size_x; 
            const unsigned short tile_size_x = (tile_x == config.tile_count_x -1) ? config.last_tile_size_x : config.tile_size_x;

            for (unsigned short k = 0; k < z_diff; k++)
            {
                const unsigned int k_offset = k * bank_z_stride;

                for (unsigned short j = 0; j < local_tile_size_y; j++)
                {
                    unsigned int offset_1 = config.start_offset + tile_x_offset;
                    unsigned int offset_2 = k_offset + local_tile_y_offset;
                    unsigned int j_offset = j * config.grid_xblocks;
                    unsigned int offset = offset_1 + offset_2 + j_offset;

                    // 3. If in the Y-avoid zone, mask the entire row out by forcing avoid_x == tile_size_x
                    unsigned short current_avoid_x = (j < local_avoid_y) ? tile_size_x : base_avoid_x;

                #ifdef DEBUG_LOG_PRINT
                    printf("|HLS DEBUG_LOG|%s|bank-%u| offset_1:%u offset_2:%u j_offset:%u final_offset:%u | tile_y:%u tile_x:%u k:%u j:%u current_avoid_x:%u tile_size_x:%u\n",
                                __func__, BANK_ID, offset_1, offset_2, j_offset, offset, (unsigned int)tile_y, (unsigned int)tile_x, (unsigned int)k, (unsigned int)j, (unsigned int)current_avoid_x, (unsigned int)tile_size_x);
                #endif
                    stream2memWithAvoidV2<MEM_DATA_WIDTH, IN_ITR>((ap_uint<MEM_DATA_WIDTH>* )(mem_out + offset), strm_in, tile_size_x, current_avoid_x);
                }
            }
        }
    }
}

// template<std::size_t I = 0, unsigned short MEM_DATA_WIDTH, unsigned short BURST_SIZE, unsigned short IN_ITR, typename... Tp>
// inline typename std::enable_if<I == sizeof...(Tp), void>::type
// call_stream2mem(unsigned short target, const std::tuple<Tp...>& t, ::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm, unsigned int offset, unsigned short len) {
//     // Base case: do nothing
// }

// template<std::size_t I = 0, unsigned short MEM_DATA_WIDTH, unsigned short BURST_SIZE, unsigned short IN_ITR, typename... Tp>
// inline typename std::enable_if<I < sizeof...(Tp), void>::type
// call_stream2mem(unsigned short target, const std::tuple<Tp...>& t, ::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm, unsigned int offset, unsigned short len) {
//     if (target == I) {
//         // HLS will resolve this std::get<I> at compile time for each branch
//         stream2mem<MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>((ap_uint<MEM_DATA_WIDTH>*)(std::get<I>(t) + offset), strm, len);
//     } else {
//         call_stream2mem<I + 1, MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>(target, t, strm, offset, len);
//     }
// }

// template <unsigned short MEM_DATA_WIDTH, unsigned short NUM_BANKS, unsigned short BURST_SIZE=32, unsigned short IN_ITR=2, typename... BUFF_TYPE>
// static void stridedTileStream2mem(::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in, const ops::hls::MemConfigTile& config, BUFF_TYPE*... mem_out)
// {
//     // #pragma HLS INLINE off
//     #ifdef DEBUG_LOG_PRINT
//         printf("|HLS DEBUG_LOG|%s| writing tile. tile_start:%d, tile_size:%d\n", __func__, config.start_offset, config.total_size_bytes);
//     #endif

//     auto banks = std::forward_as_tuple(mem_out...);
//     const unsigned short z_diff = config.end_z - config.start_z;
//     const unsigned short tile_size_y_mul_z_diff = config.tile_size_y * z_diff;
//     const unsigned short last_tile_size_y_mul_z_diff = config.last_tile_size_y * z_diff;

//     for (unsigned short tile_y = 0; tile_y < config.tile_count_y; tile_y++)
//     {
//         const unsigned short tile_size_y = tile_y == (config.tile_count_y -1) ? config.last_tile_size_y : config.tile_size_y;
//         const unsigned short realized_tile_size_y_mul_z_diff = tile_y == (config.tile_count_y -1) ? last_tile_size_y_mul_z_diff : tile_size_y_mul_z_diff;
//         const unsigned int tile_y_offset = tile_y * config.effective_tile_size_y * config.grid_xblocks;

//         for (unsigned short tile_x = 0; tile_x < config.tile_count_x; tile_x++)
//         {
//             const unsigned int abs_row_id_x_offset = tile_x * realized_tile_size_y_mul_z_diff;
//             const unsigned int tile_x_offset = tile_x * config.effective_tile_size_x; 

//             for (unsigned short k = 0; k < z_diff; k++)
//             {
//                 const unsigned int k_offset = k * config.grid_xblocks * config.grid_size_y;

//                 for (unsigned short j = 0; j < tile_size_y/NUM_BANKS; j+=NUM_BANKS)
//                 {
//                     // #pragma HLS PIPELINE // TODO: Check whether this is effective or not. Most probaly not required as stream2mem already has PIPELINE pragma inside.
//                     const unsigned short tile_size_x = tile_x == (config.tile_count_x -1) ? config.last_tile_size_x : config.tile_size_x;
                    
//                     unsigned int offset_1 = config.start_offset + tile_x_offset;
//                     unsigned int offset_2 = k_offset + tile_y_offset;
//                     unsigned int offset_3 = offset_1 + offset_2;
//                     unsigned int j_offset  = j * config.grid_xblocks;
//                     unsigned int offset = offset_3 + j_offset;
//                     #ifdef DEBUG_LOG_PRINT
//                         printf("|HLS DEBUG_LOG|%s| offset:%u tile_y:%u tile_x:%u k:%u j:%u tile_size_x:%u\n",
//                                __func__,
//                                (unsigned int)offset,
//                                (unsigned int)tile_y,
//                                (unsigned int)tile_x,
//                                (unsigned int)k,
//                                (unsigned int)j,
//                                (unsigned int)tile_size_x);
//                     #endif
//                     for (unsigned short b = 0; b  < NUM_BANKS; b++)
//                     {
//                     // #pragma HLS UNROLL type=complete
//                         unsigned short ad_j = j * NUM_BANKS + b;
//                         if (ad_j < tile_size_y) 
//                             call_stream2mem<0, MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>(b, banks, strm_in, offset, tile_size_x);
//                     }
//                 }
//             }
//         }
//     }
// }

/**
 * @brief Merges an array of parallel input streams into a single output stream based on 2D row parity.
 *
 * @details This component acts as a deterministic, static multiplexer bridging the decoupled memory datamovers 
 * and the central compute PE. It sequentially iterates over the complete 2D grid space (tiled in X, full span in Y), 
 * dynamically extracting data from the correct bank stream. Routing is dictated purely by the global Y-row 
 * index (`j`), utilizing a highly efficient power-of-two bitwise mask (`BANK_MASK`) to resolve the target bank 
 * ID in hardware without pipeline stalls.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of each stream element.
 * @tparam NUM_BANKS The total number of physical memory banks/streams. Must be a power of two.
 * @tparam IN_ITR Initiation interval for the pipeline (default: 2).
 *
 * @param[in] strm_in Array of input HLS streams corresponding to each interleaved memory bank.
 * @param[out] strm_out Reference to the merged sequential output HLS stream feeding the compute kernel.
 * @param[in] config Reference to MemConfigTile configuration guiding the global coordinate bounds.
 *
 * @note Enforces a static compile-time check ensuring `NUM_BANKS` is a power of two.
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short NUM_BANKS, unsigned short IN_ITR=2>
static void combineStreams2D(
    ::hls::stream<ap_uint<MEM_DATA_WIDTH>> strm_in[NUM_BANKS],
    ::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_out,
    const ops::hls::MemConfigTile& config)
{
#ifndef __SYNTHESIS__
    static_assert((NUM_BANKS != 0) && ((NUM_BANKS & (NUM_BANKS - 1)) == 0), "NUM_BANKS must be a power of two");
#endif

#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| Begining \n", __func__);
#endif 
	constexpr unsigned short BANK_MASK = NUM_BANKS - 1;

    for (unsigned short tile_x = 0; tile_x < config.tile_count_x; tile_x++)
    {
        const unsigned short tile_size_x = tile_x == (config.tile_count_x -1) ? config.last_tile_size_x : config.tile_size_x;

        for (unsigned short j = 0; j < config.grid_size_y; j++)
        {
            unsigned int bank_id = j & BANK_MASK;

            for (unsigned short i = 0; i < tile_size_x; i++)
            {
            #pragma HLS PIPELINE II=IN_ITR
                ap_uint<MEM_DATA_WIDTH> data;

				data = strm_in[bank_id].read();
#ifdef DEBUG_LOG_PRINT
				printf("|HLS DEBUG_LOG|%s| Reading and forwaring tile:%u, i: %u, j:%u, bank: %d, val=(\n", __func__, tile_x, i, j, bank_id);

				for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
				{
					DataConv conv;
					conv.i = data.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
					printf("%f,", conv.f);
				}
				printf(")\n");
#endif
                strm_out.write(data);
            }
        }
    }
#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| Exiting \n", __func__);
#endif 
}

/**
 * @brief Merges multiple input streams into a single output stream based on row parity for a 3D tiled layout.
 *
 * @details This function reads data from an array of NUM_BANKS input streams and writes to an output stream,
 * alternating between the streams based on the computed absolute row index within the tiled 3D region.
 * Uses bitwise AND operations for power-of-two NUM_BANKS optimization to determine the correct bank.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of each stream element
 * @tparam NUM_BANKS The number of memory banks/streams. Must be a power of two.
 * @tparam IN_ITR Initiation interval for the pipeline (default: 2)
 *
 * @param[in] strm_in Array of input HLS streams corresponding to each bank.
 * @param[out] strm_out Reference to the output HLS stream receiving merged data
 * @param[in] config Reference to MemConfigTile configuration containing:
 * - start_z, end_z: Z-dimension range
 * - tile_size_y, last_tile_size_y: Y-tile dimensions
 * - tile_size_x, last_tile_size_x: X-tile dimensions
 * - tile_count_x, tile_count_y: Number of tiles in each dimension
 *
 * @note Stream selection relies on an unrolled multiplexer to satisfy HLS dataflow constraints.
 * @note Debug logging available when DEBUG_LOG_PRINT is defined
 *
 * @see ops::hls::MemConfigTile
 * @see ops::hls::stridedTileMem2stream3D
 * @see ops::hls::stridedTileStream2mem3D
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short NUM_BANKS, unsigned short IN_ITR=2>
static void combineStreams3D(
    ::hls::stream<ap_uint<MEM_DATA_WIDTH>> strm_in[NUM_BANKS],
    ::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_out,
    const ops::hls::MemConfigTile& config)
{
#ifndef __SYNTHESIS__
    static_assert((NUM_BANKS != 0) && ((NUM_BANKS & (NUM_BANKS - 1)) == 0), "NUM_BANKS must be a power of two");
#endif

#ifdef DEBUG_LOG_PRINT
    printf("|HLS DEBUG_LOG|%s| Beginning 3D stream combination \n", __func__);
#endif 
    constexpr unsigned short BANK_SHIFT = LOG2(NUM_BANKS);
    constexpr unsigned short BANK_MASK = NUM_BANKS - 1;
    const unsigned short z_diff = config.end_z - config.start_z;

    for (unsigned short tile_y = 0; tile_y < config.tile_count_y; tile_y++)
    {
        const unsigned short tile_size_y = (tile_y == config.tile_count_y -1) ? config.last_tile_size_y : config.tile_size_y;
        ap_uint<BANK_SHIFT> start_bank = tile_y * config.effective_tile_size_y;

        for (unsigned short tile_x = 0; tile_x < config.tile_count_x; tile_x++)
        {
            const unsigned short tile_size_x = (tile_x == config.tile_count_x -1) ? config.last_tile_size_x : config.tile_size_x;

            for (unsigned short k = 0; k < z_diff; k++)
            {
                for (unsigned short j = 0; j < tile_size_y; j++)
                {
                    ap_uint<BANK_SHIFT> bank_id_init = j & BANK_MASK;
                    ap_uint<BANK_SHIFT> bank_id = bank_id_init + start_bank;

                    for (unsigned short i = 0; i < tile_size_x; i++)
                    {
                    #pragma HLS PIPELINE II=IN_ITR
                        ap_uint<MEM_DATA_WIDTH> data = strm_in[bank_id].read();
                        
        #ifdef DEBUG_LOG_PRINT
                        printf("|HLS DEBUG_LOG|%s| Reading and forwarding tile_y:%u, tile_x:%u, k:%u, j:%u, i:%u, bank:%u, val=(\n", __func__, tile_y, tile_x, k, j, i, (unsigned int)bank_id);

                        for (unsigned k_idx = 0; k_idx < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k_idx++)
                        {
                            DataConv conv;
                            conv.i = data.range((k_idx+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k_idx * DEBUG_LOG_SIZE_OF * 8);
                            printf("%f,", conv.f);
                        }
                        printf(")\n");
        #endif
                        strm_out.write(data);
                    }
                }
            }
        }
    }
#ifdef DEBUG_LOG_PRINT
    printf("|HLS DEBUG_LOG|%s| Exiting \n", __func__);
#endif 
}

/**
 * @brief Demultiplexes a sequential data stream into parallel bank-specific streams based on 2D row parity.
 *
 * @details This component acts as a deterministic demultiplexer routing output data from the central compute PE 
 * back to the isolated, bank-specific datamovers. It iterates over the complete 2D grid space (tiled in X, 
 * full span in Y) and routes elements based on the global Y-row index. It uses a power-of-two bitwise mask 
 * (`BANK_MASK`) for zero-cost hardware routing, forwarding the payload to the corresponding index in the `strm_out` array.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of each stream element.
 * @tparam NUM_BANKS The total number of physical memory banks/streams. Must be a power of two.
 * @tparam IN_ITR Initiation interval for the pipeline (default: 2).
 *
 * @param[in] strm_in Reference to the sequential input HLS stream returning from the compute kernel.
 * @param[out] strm_out Array of output HLS streams corresponding to each interleaved memory bank.
 * @param[in] config Reference to MemConfigTile configuration guiding the global coordinate bounds.
 *
 * @note Enforces a static compile-time check ensuring `NUM_BANKS` is a power of two.
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short NUM_BANKS, unsigned short IN_ITR=2>
static void splitStream2D(
    ::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in,
    ::hls::stream<ap_uint<MEM_DATA_WIDTH>> strm_out[NUM_BANKS],
    const ops::hls::MemConfigTile& config)
{
#ifndef __SYNTHESIS__
    static_assert((NUM_BANKS != 0) && ((NUM_BANKS & (NUM_BANKS - 1)) == 0), "NUM_BANKS must be a power of two");
#endif

#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| Begining \n", __func__);
#endif 
	constexpr unsigned short BANK_MASK = NUM_BANKS - 1;

    for (unsigned short tile_x = 0; tile_x < config.tile_count_x; tile_x++)
    {
        const unsigned short tile_size_x = tile_x == (config.tile_count_x -1) ? config.last_tile_size_x : config.tile_size_x;
        
        for (unsigned short j = 0; j < config.grid_size_y; j++)
        {
            unsigned int bank_id = j & BANK_MASK; 
            
            for (unsigned short i = 0; i < tile_size_x; i++)
            {
            #pragma HLS PIPELINE II=IN_ITR
                ap_uint<MEM_DATA_WIDTH> data = strm_in.read();
#ifdef DEBUG_LOG_PRINT
				printf("|HLS DEBUG_LOG|%s| Reading and forwaring tile:%u, i: %u, j:%u, bank: %d, val=(\n", __func__, tile_x, i, j, bank_id);

				for (unsigned k = 0; k < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
				{
					DataConv conv;
					conv.i = data.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
					printf("%f,", conv.f);
				}
				printf(")\n");
#endif
                strm_out[bank_id].write(data);
            }
        }
    }
#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| Exit \n", __func__);
#endif 
}

/**
 * @brief Splits an input stream into multiple output streams based on row parity for a 3D tiled layout.
 *
 * @details This function reads data from an input stream and writes to an array of NUM_BANKS output streams.
 * Routing is determined by the absolute row index within the tiled 3D region. 
 * Uses bitwise AND operations for power-of-two NUM_BANKS optimization.
 *
 * @tparam MEM_DATA_WIDTH The bit-width of each stream element
 * @tparam NUM_BANKS The number of memory banks/streams. Must be a power of two.
 * @tparam IN_ITR Initiation interval for the pipeline (default: 2)
 *
 * @param[in] strm_in Reference to the input HLS stream containing the data to be split
 * @param[out] strm_out Array of output HLS streams corresponding to each bank.
 * @param[in] config Reference to MemConfigTile configuration containing:
 * - start_z, end_z: Z-dimension range
 * - tile_size_y, last_tile_size_y: Y-tile dimensions
 * - tile_size_x, last_tile_size_x: X-tile dimensions
 * - tile_count_x, tile_count_y: Number of tiles in each dimension
 *
 * @note Stream selection relies on an unrolled demultiplexer to satisfy HLS dataflow constraints.
 * @note Debug logging available when DEBUG_LOG_PRINT is defined
 *
 * @see ops::hls::MemConfigTile
 * @see ops::hls::stridedTileMem2stream3D
 * @see ops::hls::stridedTileStream2mem3D
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short NUM_BANKS, unsigned short IN_ITR=2>
static void splitStream3D(
    ::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in,
    ::hls::stream<ap_uint<MEM_DATA_WIDTH>> strm_out[NUM_BANKS],
    const ops::hls::MemConfigTile& config)
{
#ifndef __SYNTHESIS__
    static_assert((NUM_BANKS != 0) && ((NUM_BANKS & (NUM_BANKS - 1)) == 0), "NUM_BANKS must be a power of two");
#endif

#ifdef DEBUG_LOG_PRINT
    printf("|HLS DEBUG_LOG|%s| Beginning 3D stream split \n", __func__);
#endif 
    constexpr unsigned short BANK_SHIFT = LOG2(NUM_BANKS);
    constexpr unsigned short BANK_MASK = NUM_BANKS - 1;
    const unsigned short z_diff = config.end_z - config.start_z;

    for (unsigned short tile_y = 0; tile_y < config.tile_count_y; tile_y++)
    {
        const unsigned short tile_size_y = (tile_y == config.tile_count_y -1) ? config.last_tile_size_y : config.tile_size_y;
        ap_uint<BANK_SHIFT> start_bank = tile_y * config.effective_tile_size_y;

        for (unsigned short tile_x = 0; tile_x < config.tile_count_x; tile_x++)
        {
            const unsigned short tile_size_x = (tile_x == config.tile_count_x -1) ? config.last_tile_size_x : config.tile_size_x;

            for (unsigned short k = 0; k < z_diff; k++)
            {
                for (unsigned short j = 0; j < tile_size_y; j++)
                {
                    ap_uint<BANK_SHIFT> bank_id_init = j & BANK_MASK;
                    ap_uint<BANK_SHIFT> bank_id = bank_id_init + start_bank;

                    for (unsigned short i = 0; i < tile_size_x; i++)
                    {
                    #pragma HLS PIPELINE II=IN_ITR
                        ap_uint<MEM_DATA_WIDTH> data = strm_in.read();

        #ifdef DEBUG_LOG_PRINT
                        printf("|HLS DEBUG_LOG|%s| Reading and forwarding tile_y:%u, tile_x:%u, k:%u, j:%u, i:%u, bank:%u, val=(\n", __func__, tile_y, tile_x, k, j, i, (unsigned int)bank_id);

                        for (unsigned k_idx = 0; k_idx < MEM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k_idx++)
                        {
                            DataConv conv;
                            conv.i = data.range((k_idx+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k_idx * DEBUG_LOG_SIZE_OF * 8);
                            printf("%f,", conv.f);
                        }
                        printf(")\n");
        #endif
                        strm_out[bank_id].write(data);
                    }
                }
            }
        }
    }
#ifdef DEBUG_LOG_PRINT
    printf("|HLS DEBUG_LOG|%s| Exit \n", __func__);
#endif 
}

// /**
//  * @brief 	mem2streamTiled reads from a memory location in tiled manner to an hls stream.
//  *  		This is optimized to read from AXI4 with burst and to utilize width maximum througput
//  *          by utilizing two banks to read from.
//  *
//  * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port and the hls stream port
//  * @tparam IN_ITR: II of the mem read
//  * @tparam BURST_SIZE : Burst length of the AXI4 (max beats < 256)
//  * 
//  *
//  * @param mem_in : input memory port
//  * @param stream_out : output hls-stream
//  * @param config : MemconfigTile to guide reading
//  */
// template <unsigned short MEM_DATA_WIDTH,  unsigned short BURST_SIZE=32, unsigned short IN_ITR=2>
// void mem2streamTiled(ap_uint<MEM_DATA_WIDTH>* mem_in_b1,
//                 ap_uint<MEM_DATA_WIDTH>* mem_in_b2,
// 				::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_out,
// 				const ops::hls::MemConfigTile& config)
// {
// #ifdef DEBUG_LOG_PRINT
// 	printf("|HLS DEBUG_LOG|%s| starting\n", __func__);
// #endif
// 	if (config.isContinous)
// 	{
// #ifdef DEBUG_LOG_PRINT
// 	printf("|HLS DEBUG_LOG|%s| continuous read\n", __func__);
// #endif

// #ifdef DEBUG_LOG_PRINT
// 	printf("|HLS DEBUG_LOG|%s| init offset:%d, size_bytes:%d\n", __func__, config.start_offset, config.total_size_bytes);
// #endif
// 	ops::hls::mem2stream<MEM_DATA_WIDTH>((ap_uint<MEM_DATA_WIDTH>* )(mem_in_b1 + config.start_offset), strm_out, config.total_xblocks);
// 	}
// 	else
// 	{
//         static ::hls::stream<ap_uint<MEM_DATA_WIDTH>> strm_in_b1;
//         #pragma HLS STREAM variable = strm_in_b1
//         static ::hls::stream<ap_uint<MEM_DATA_WIDTH>> strm_in_b2;
//         #pragma HLS STREAM variable = strm_in_b2
//         #pragma HLS DATAFLOW
//         stridedTileMem2stream<MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>(mem_in_b1, strm_in_b1, config, 0);
//         stridedTileMem2stream<MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>(mem_in_b2, strm_in_b2, config, 1);
//         combineSteams<MEM_DATA_WIDTH, IN_ITR>(strm_in_b1, strm_in_b2, strm_out, config);
// 	}
// #ifdef DEBUG_LOG_PRINT
// 	printf("|HLS DEBUG_LOG|%s| exiting.\n"
// 			, __func__);
// #endif
// }

// /**
//  * @brief 	stream2memTiled writes from a stream to memory in tiled manner.
//  *  		This is optimized to write to AXI4 with burst and to utilize maximum throughput
//  *          by utilizing two banks to write to.
//  *
//  * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port and the hls stream port
//  * @tparam IN_ITR: II of the mem write
//  * @tparam BURST_SIZE : Burst length of the AXI4 (max beats < 256)
//  *
//  * @param mem_out_b1 : output memory port for bank 1
//  * @param mem_out_b2 : output memory port for bank 2
//  * @param stream_in : input hls-stream
//  * @param config : MemConfigTile to guide writing
//  */
// template <unsigned short MEM_DATA_WIDTH, unsigned short BURST_SIZE=32, unsigned short IN_ITR=2>
// void stream2memTiled(ap_uint<MEM_DATA_WIDTH>* mem_out_b1,
//                 ap_uint<MEM_DATA_WIDTH>* mem_out_b2,
//                 ::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in,
// 				const ops::hls::MemConfigTile& config)
// {
// #ifdef DEBUG_LOG_PRINT
// 	printf("|HLS DEBUG_LOG|%s| starting\n", __func__);
// #endif
// 	if (config.isContinous)
// 	{
// #ifdef DEBUG_LOG_PRINT
// 		printf("|HLS DEBUG_LOG|%s| continuous write\n", __func__);
// #endif

// #ifdef DEBUG_LOG_PRINT
// 		printf("|HLS DEBUG_LOG|%s| init offset:%d, size_bytes:%d\n", __func__, config.start_offset, config.total_size_bytes);
// #endif
// 		ops::hls::stream2mem<MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>((ap_uint<MEM_DATA_WIDTH>* )(mem_out_b1 + config.start_offset), strm_in, config.total_xblocks);
// 	}
// 	else
// 	{
//         static ::hls::stream<ap_uint<MEM_DATA_WIDTH>> strm_out_b1;
//         #pragma HLS STREAM variable = strm_out_b1
//         static ::hls::stream<ap_uint<MEM_DATA_WIDTH>> strm_out_b2;
//         #pragma HLS STREAM variable = strm_out_b2
//         #pragma HLS DATAFLOW
//         splitStream<MEM_DATA_WIDTH, IN_ITR>(strm_in, strm_out_b1, strm_out_b2, config);
//         stridedTileStream2mem<MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>(strm_out_b1, mem_out_b1,  config, 0);
//         stridedTileStream2mem<MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>(strm_out_b2, mem_out_b2,  config, 1);
// 	}
// #ifdef DEBUG_LOG_PRINT
// 	printf("|HLS DEBUG_LOG|%s| exiting.\n"
// 			, __func__);
// #endif
// }
/****************************** END of TILED ops *******************************/

/**
 * @brief stream2stream Converts from one hls-stream to another with different size.
 *
 * @tparam STREAM1_DATA_WIDTH : Data width of the hls port1
 * @tparam STREAM1_DATA_WIDTH : Data width of the hls port1
 *
 * @param num_big_pkts : Number of pkts of the wider stream
 */
template <unsigned int STREAM1_DATA_WIDTH, unsigned int STREAM2_DATA_WIDTH>
DEPRECATED void stream2stream(::hls::stream<ap_uint<STREAM1_DATA_WIDTH>>& strm_in,
				::hls::stream<ap_uint<STREAM2_DATA_WIDTH>>& strm_out,
				const unsigned int num_big_pkts)
{
#ifndef __SYNTHESIS__
	static_assert(STREAM1_DATA_WIDTH > STREAM2_DATA_WIDTH ? STREAM1_DATA_WIDTH % STREAM2_DATA_WIDTH == 0 : STREAM2_DATA_WIDTH % STREAM1_DATA_WIDTH == 0,
			"Bigger stream has to be fully divisible by smaller stream");
#endif
	constexpr unsigned short BIG_STREAM_DATA_WIDTH = STREAM1_DATA_WIDTH > STREAM2_DATA_WIDTH ? STREAM1_DATA_WIDTH : STREAM2_DATA_WIDTH;
	constexpr unsigned short SMALL_STREAM_DATA_WIDTH = STREAM1_DATA_WIDTH > STREAM2_DATA_WIDTH ? STREAM2_DATA_WIDTH : STREAM1_DATA_WIDTH;
	constexpr unsigned short FACTOR = BIG_STREAM_DATA_WIDTH / SMALL_STREAM_DATA_WIDTH;
	constexpr unsigned short BIG_STREAM_DATA_WIDTH_BYTES = BIG_STREAM_DATA_WIDTH / 8;
	constexpr unsigned short TYPE = STREAM1_DATA_WIDTH > STREAM2_DATA_WIDTH ? 1 : 0;


#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG| %s | type: %d, num_big_pkts: %d\n"
			, __func__,TYPE, num_big_pkts);
	printf("====================================================================================\n");
#endif

	if (TYPE)
	{
		ap_uint<STREAM1_DATA_WIDTH> tmp1;
		for (int pkt = 0; pkt < num_big_pkts; pkt++)
		{
			for (unsigned n = 0; n < FACTOR; n++)
			{
			#pragma HLS PIPELINE II=1
			#pragma HLS LOOP_FLATTEN

				if (n == 0)
				{
					tmp1 = strm_in.read();

#ifdef DEBUG_LOG_PRINT
					printf("   |HLS DEBUG_LOG||%s| receiving pkt: %d, val=(", __func__, pkt);

					for (unsigned n = 0; n < STREAM1_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); n++)
					{
						DataConv conv;
						conv.i = tmp1.range((n+1) * DEBUG_LOG_SIZE_OF * 8 - 1, n * DEBUG_LOG_SIZE_OF * 8);
						printf("%f,", conv.f);
					}
					printf(")\n");
#endif

				}

				ap_uint<STREAM2_DATA_WIDTH> tmp2 = tmp1.range((n+1)* STREAM2_DATA_WIDTH -1, n * STREAM2_DATA_WIDTH);
				strm_out.write(tmp2);

#ifdef DEBUG_LOG_PRINT
				printf("   |HLS DEBUG_LOG||%s| writing pkt: %d, val=(", __func__, pkt*FACTOR + n);

				for (unsigned k = 0; k < STREAM2_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
				{
					DataConv conv;
					conv.i = tmp2.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
					printf("%f,", conv.f);
				}
				printf(")\n");
#endif

			}
		}
	}
	else
	{
		ap_uint<STREAM2_DATA_WIDTH> tmp2;
		for (int pkt = 0; pkt < num_big_pkts; pkt++)
		{
			for (unsigned n = 0; n < FACTOR; n++)
			{
			#pragma HLS PIPELINE II=1
			#pragma HLS LOOP_FLATTEN

				ap_uint<STREAM1_DATA_WIDTH> tmp1 = strm_in.read();

#ifdef DEBUG_LOG_PRINT
				printf("   |HLS DEBUG_LOG||%s| reading pkt: %d, val=(", __func__, pkt*FACTOR + n);

				for (unsigned k = 0; k < STREAM1_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
				{
					DataConv conv;
					conv.i = tmp1.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
					printf("%f,", conv.f);
				}
				printf(")\n");
#endif

				tmp2.range((n+1)* STREAM1_DATA_WIDTH -1, n * STREAM1_DATA_WIDTH) = tmp1;

				if (n == FACTOR - 1)
				{

#ifdef DEBUG_LOG_PRINT
				printf("   |HLS DEBUG_LOG||%s| writing pkt: %d, val=(", __func__, pkt*FACTOR + n);

				for (unsigned k = 0; k < STREAM2_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
				{
					DataConv conv;
					conv.i = tmp2.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
					printf("%f,", conv.f);
				}
				printf(")\n");
#endif

				strm_out.write(tmp2);
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
 * @brief stream2streamStepup Converts from one hls-stream to another with bigger size.
 *
 * @tparam STREAM1_DATA_WIDTH : Data width of the hls port1
 * @tparam STREAM2_DATA_WIDTH : Data width of the hls port2
 *
 * @param num_big_pkts : Number of pkts of the wider stream
 */
template <unsigned int STREAM1_DATA_WIDTH, unsigned int STREAM2_DATA_WIDTH>
void stream2streamStepup(::hls::stream<ap_uint<STREAM1_DATA_WIDTH>>& strm_in,
				::hls::stream<ap_uint<STREAM2_DATA_WIDTH>>& strm_out,
				const unsigned int num_big_pkts)
{
#ifndef __SYNTHESIS__
	static_assert(STREAM1_DATA_WIDTH < STREAM2_DATA_WIDTH,
			"STREAM1_DATA_WIDTH has to be smaller than STREAM2_DATA_WIDTH");
    static_assert(STREAM2_DATA_WIDTH % STREAM1_DATA_WIDTH == 0, 
            "STREAM2_DATA_WIDTH has to be fully divisible by STREAM1_DATA_WIDTH");
    static_assert(STREAM1_DATA_WIDTH % 8 == 0, "STREAM1_DATA_WIDTH should be divisible by 8");
    static_assert(STREAM2_DATA_WIDTH % 8 == 0, "STREAM2_DATA_WIDTH should be divisible by 8");
#endif

constexpr unsigned short FACTOR = STREAM2_DATA_WIDTH / STREAM1_DATA_WIDTH;


#ifdef DEBUG_LOG_PRINT
        printf("|HLS DEBUG_LOG| %s | num_big_pkts: %d\n"
                , __func__, num_big_pkts);
        printf("====================================================================================\n");
#endif


    ap_uint<STREAM2_DATA_WIDTH> tmp2;
    for (unsigned int pkt = 0; pkt < num_big_pkts; pkt++)
    {
        for (unsigned short n = 0; n < FACTOR; n++)
        {
        #pragma HLS PIPELINE II=1
        #pragma HLS LOOP_FLATTEN

            ap_uint<STREAM1_DATA_WIDTH> tmp1 = strm_in.read();

#ifdef DEBUG_LOG_PRINT
            printf("   |HLS DEBUG_LOG||%s| reading pkt: %d, val=(", __func__, pkt*FACTOR + n);

            for (unsigned k = 0; k < STREAM1_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
            {
                DataConv conv;
                conv.i = tmp1.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
                printf("%f,", conv.f);
            }
            printf(")\n");
#endif
            tmp2.range((n+1)* STREAM1_DATA_WIDTH -1, n * STREAM1_DATA_WIDTH) = tmp1;

            if (n == FACTOR - 1)
            {
#ifdef DEBUG_LOG_PRINT
            printf("   |HLS DEBUG_LOG||%s| writing pkt: %d, val=(", __func__, pkt*FACTOR + n);

            for (unsigned k = 0; k < STREAM2_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
            {
                DataConv conv;
                conv.i = tmp2.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
                printf("%f,", conv.f);
            }
            printf(")\n");
#endif


            strm_out.write(tmp2);
            }
        }
    }

#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| exiting.\n"
			, __func__);
#endif
}

/**
 * @brief stream2streamStepdown Converts from one hls-stream to another with smaller size.
 *
 * @tparam STREAM1_DATA_WIDTH : Data width of the hls port1
 * @tparam STREAM2_DATA_WIDTH : Data width of the hls port2
 *
 * @param num_big_pkts : Number of pkts of the wider stream
 */
template <unsigned int STREAM1_DATA_WIDTH, unsigned int STREAM2_DATA_WIDTH>
void stream2streamStepdown(::hls::stream<ap_uint<STREAM1_DATA_WIDTH>>& strm_in,
				::hls::stream<ap_uint<STREAM2_DATA_WIDTH>>& strm_out,
				const unsigned int num_big_pkts)
{
#ifndef __SYNTHESIS__
	static_assert(STREAM1_DATA_WIDTH > STREAM2_DATA_WIDTH,
			"STREAM1_DATA_WIDTH has to be bigger than STREAM2_DATA_WIDTH");
    static_assert(STREAM1_DATA_WIDTH % STREAM2_DATA_WIDTH == 0, 
            "STREAM1_DATA_WIDTH has to be fully divisible by STREAM2_DATA_WIDTH");
    static_assert(STREAM1_DATA_WIDTH % 8 == 0, "STREAM1_DATA_WIDTH should be divisible by 8");
    static_assert(STREAM2_DATA_WIDTH % 8 == 0, "STREAM2_DATA_WIDTH should be divisible by 8");
#endif

constexpr unsigned short FACTOR = STREAM1_DATA_WIDTH / STREAM2_DATA_WIDTH;

#ifdef DEBUG_LOG_PRINT
        printf("|HLS DEBUG_LOG| %s | num_big_pkts: %d\n"
                , __func__, num_big_pkts);
        printf("====================================================================================\n");
#endif

    ap_uint<STREAM1_DATA_WIDTH> tmp1;
    
    for (unsigned int pkt = 0; pkt < num_big_pkts; pkt++)
    {
        for (unsigned short n = 0; n < FACTOR; n++)
        {
        #pragma HLS PIPELINE II=1
        #pragma HLS LOOP_FLATTEN

            if (n == 0)
            {
                tmp1 = strm_in.read();

#ifdef DEBUG_LOG_PRINT
                    printf("   |HLS DEBUG_LOG||%s| receiving pkt: %d, val=(", __func__, pkt);

                    for (unsigned n = 0; n < STREAM1_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); n++)
                    {
                        DataConv conv;
                        conv.i = tmp1.range((n+1) * DEBUG_LOG_SIZE_OF * 8 - 1, n * DEBUG_LOG_SIZE_OF * 8);
                        printf("%f,", conv.f);
                    }
                    printf(")\n");
#endif

            }

            ap_uint<STREAM2_DATA_WIDTH> tmp2 = tmp1.range((n+1)* STREAM2_DATA_WIDTH -1, n * STREAM2_DATA_WIDTH);
            strm_out.write(tmp2);

#ifdef DEBUG_LOG_PRINT
                printf("   |HLS DEBUG_LOG||%s| writing pkt: %d, val=(", __func__, pkt*FACTOR + n);

                for (unsigned k = 0; k < STREAM2_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); k++)
                {
                    DataConv conv;
                    conv.i = tmp2.range((k+1) * DEBUG_LOG_SIZE_OF * 8 - 1, k * DEBUG_LOG_SIZE_OF * 8);
                    printf("%f,", conv.f);
                }
                printf(")\n");
#endif

        }
    }

}

/**
 * @brief stream2axis Converts from one hls-stream to axis4-stream with same_size
 *
 * @tparam STREAM_DATA_WIDTH : Data width of the streams
 *
 * @param pkts : Number of pkts
 */
template <unsigned int STREAM_DATA_WIDTH>
void stream2axis(::hls::stream<ap_uint<STREAM_DATA_WIDTH>>& strm_in,
				::hls::stream<ap_axiu<STREAM_DATA_WIDTH,0,0,0>>& strm_out,
				const unsigned int pkts
#ifdef DEBUG_LOG_PRINT
				, const bool print_enbl = true
#endif
			)
{

#ifdef DEBUG_LOG_PRINT

	if (print_enbl) {
		// print("\n=======================================================\n");
		print("|HLS DEBUG_LOG| stream2axis | starting. pkts: %d\n", pkts);
	}
#endif

	for (unsigned itr = 0; itr < pkts; itr++){
		#pragma HLS PIPELINE II=1

		ap_axiu<STREAM_DATA_WIDTH,0,0,0> tmp;
		tmp.data = strm_in.read();

#ifdef DEBUG_LOG_PRINT
			if (print_enbl) {
				// print("------------------------------------------------------------------------------------\n");
				print(" |HLS DEBUG_LOG| stream2axis | sending axis pkt: %d, val=(\n", itr);
			}
			for (unsigned n = 0; n < STREAM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); n++)
			{
				DataConv conv;
				conv.i = tmp.data.range((n+1) * DEBUG_LOG_SIZE_OF * 8 - 1, n * DEBUG_LOG_SIZE_OF * 8);
				if (print_enbl) {print("|HLS DEBUG_LOG| stream2axis | %f\n", conv.f);}
			}
			// if (print_enbl) {
			// 	print(")\n");
			// 	print("------------------------------------------------------------------------------------\n");
			// }
#endif
		strm_out << tmp;
	}
#ifdef DEBUG_LOG_PRINT
	if (print_enbl){print("|HLS DEBUG_LOG| stream2axis | Exit \n");}
#endif
}

/**
 * @brief axis2stream Converts from one axis4-stream to hls-stream with same_size
 *
 * @tparam STREAM_DATA_WIDTH : Data width of the streams
 *
 * @param pkts : Number of pkts
 */
template <unsigned int STREAM_DATA_WIDTH>
void axis2stream(::hls::stream<ap_axiu<STREAM_DATA_WIDTH,0,0,0>>& strm_in,
		::hls::stream<ap_uint<STREAM_DATA_WIDTH>>& strm_out,
		unsigned int pkts
#ifdef DEBUG_LOG_PRINT
		, const bool print_enbl = true
#endif	
	)
{

#ifdef DEBUG_LOG_PRINT
	if (print_enbl) {
		print("\n=======================================================\n");
		print("|HLS DEBUG_LOG| axis2stream | starting. pkts: %d\n", pkts);
	}
#endif

	for (int itr = 0; itr < pkts; itr++){
		#pragma HLS PIPELINE II=1

		ap_axiu<STREAM_DATA_WIDTH,0,0,0> tmp = strm_in.read();

#ifdef DEBUG_LOG_PRINT
			if (print_enbl) {
				// print("------------------------------------------------------------------------------------\n");
				print(" |HLS DEBUG_LOG| axis2stream | sending hls pkt: %d, val=(\n", itr);
			}
			for (unsigned n = 0; n < STREAM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); n++)
			{
				DataConv conv;
				conv.i = tmp.data.range((n+1) * DEBUG_LOG_SIZE_OF * 8 - 1, n * DEBUG_LOG_SIZE_OF * 8);
				if (print_enbl) {print("|HLS DEBUG_LOG| axis2stream | %f\n", conv.f);}
			}
			// if (print_enbl) {
			// 	print(")\n");
			// 	print("------------------------------------------------------------------------------------\n");
			// }
#endif

		strm_out << tmp.data;
	}
#ifdef DEBUG_LOG_PRINT
	if (print_enbl) {print("|HLS DEBUG_LOG| axis2stream | Exit \n");}
#endif
}

/**
 * @brief 	mem2axis reads from a memory location with to an AXI4 stream.
 *  		This is optimized to read from AXI4 with burst and to utilize width conversion in-between
 *  		AXI4 to AXI4-stream.
 * 
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port
 * @tparam AXIS_DATA_WIDTH : Data width of the AXI4-stream port
 * @tparam BURST_SIZE : Burst length of the AXI4 (max beats - 256)
 * 
 * @param mem_in : input memory port
 * @param stream_out : output AXI4-stream
 * @param size : Number of bytes of the data
 */
template <unsigned int MEM_DATA_WIDTH, unsigned int AXIS_DATA_WIDTH, unsigned int BURST_SIZE=32>
void mem2axis(ap_uint<MEM_DATA_WIDTH>* mem_in,
				::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_out,
				unsigned int size)
{
#ifndef __SYNTHESIS__
	static_assert(MEM_DATA_WIDTH % AXIS_DATA_WIDTH == 0, 
			"MEM_DATA_WIDTH has to be fully divided by AXIS_DATA_WIDTH");
	static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
			"MEM_DATA_WIDTH failed limit check");
	static_assert(AXIS_DATA_WIDTH >= min_axis_data_width && AXIS_DATA_WIDTH <= max_axis_data_width,
			"AXIS_DATA_WIDTH failed limit check");
	static_assert(BURST_SIZE >= min_burst_len && BURST_SIZE <= max_burst_len,
			" BURST_SIZE has failed limit check");
#endif

	constexpr unsigned int bytes_per_beat = MEM_DATA_WIDTH / 8;
	constexpr unsigned int bytes_per_axis_pkt = AXIS_DATA_WIDTH / 8;
	constexpr unsigned int num_strm_pkts_per_beat = MEM_DATA_WIDTH / AXIS_DATA_WIDTH;

	const unsigned int num_beats = (size + bytes_per_beat - 1) / bytes_per_beat;
	const unsigned int num_bursts = num_beats / BURST_SIZE;
	const unsigned int non_burst_beats = num_beats % BURST_SIZE;

#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG| %s | size: %d, num_beats: %d, num_burst: %d, non_burst_beats: %d\n"
			, __func__, size, num_beats, num_bursts, non_burst_beats);
	printf("====================================================================================\n");
#endif

	unsigned int index = 0;

	for (unsigned int brst = 0; brst < num_bursts; brst++)
	{
	#pragma HLS LOOP_TRIPCOUNT avg=avg_num_of_bursts max=max_num_of_bursts

		for (unsigned int beat = 0; beat < BURST_SIZE; beat++)
		{
		#pragma HLS PIPELINE II=num_strm_pkts_per_beat
		#pragma HLS LOOP_TRIPCOUNT min=min_burst_len avg=avg_burst_len max=max_burst_len

			convMemBeat2axisPkt<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>(mem_in, strm_out, size, index);
			index++;
		}
	}
	
	for (unsigned int beat = 0; beat < non_burst_beats; beat++)
	{
	#pragma HLS PIPELINE II=num_strm_pkts_per_beat
		convMemBeat2axisPkt<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>(mem_in, strm_out, size, index);
		index++;
	}
}


/**
 * @brief 	mem2axisV2 reads from a memory location with to an AXI4 stream.
 *  		This is optimized to read from AXI4 with burst and to utilize width conversion in-between
 *  		AXI4 to AXI4-stream.
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port
 * @tparam AXIS_DATA_WIDTH : Data width of the AXI4-stream port
 * @tparam BURST_SIZE : Burst length of the AXI4 (max beats - 256)
 *
 * @param mem_in : input memory port
 * @param stream_out : output AXI4-stream
 * @param size : Number of bytes of the data
 */
template <unsigned int MEM_DATA_WIDTH, unsigned int AXIS_DATA_WIDTH, unsigned int BURST_SIZE=32>
void mem2axisV2(ap_uint<MEM_DATA_WIDTH>* mem_in,
				::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_out,
				unsigned int size)
{
#ifndef __SYNTHESIS__
	static_assert(MEM_DATA_WIDTH % AXIS_DATA_WIDTH == 0,
			"MEM_DATA_WIDTH has to be fully divided by AXIS_DATA_WIDTH");
	static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
			"MEM_DATA_WIDTH failed limit check");
	static_assert(AXIS_DATA_WIDTH >= min_axis_data_width && AXIS_DATA_WIDTH <= max_axis_data_width,
			"AXIS_DATA_WIDTH failed limit check");
	static_assert(BURST_SIZE >= min_burst_len && BURST_SIZE <= max_burst_len,
			" BURST_SIZE has failed limit check");
#endif

	constexpr unsigned int bytes_per_beat = MEM_DATA_WIDTH / 8;
	constexpr unsigned int bytes_per_axis_pkt = AXIS_DATA_WIDTH / 8;
	constexpr unsigned int num_strm_pkts_per_beat = MEM_DATA_WIDTH / AXIS_DATA_WIDTH;

	const unsigned int num_beats = (size + bytes_per_beat - 1) / bytes_per_beat;

#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG| %s | size: %d, num_beats: %d\n"
			, __func__, size, num_beats);
	printf("====================================================================================\n");
#endif

	::hls::stream<ap_uint<MEM_DATA_WIDTH>> mem_strm;
	::hls::stream<ap_uint<AXIS_DATA_WIDTH>> red_mem_strm;
#pragma HLS STREAM variable = mem_strm depth = max_depth_v16
#pragma HLS STREAM variable = red_mem_strm depth = max_depth_v8

#pragma HLS DATAFLOW
	mem2stream<MEM_DATA_WIDTH>(mem_in, mem_strm, num_beats);
	stream2stream<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>(mem_strm, red_mem_strm, num_beats);
	stream2axis<AXIS_DATA_WIDTH>(red_mem_strm, strm_out, num_beats*num_strm_pkts_per_beat);
}
/**
 * @brief 	mem2axis reads from two memory location with a selector mux to an AXI4 stream.
 *  		This is optimized to read from AXI4 with burst and to utilize width conversion in-between
 *  		AXI4 to AXI4-stream
 * 
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port
 * @tparam AXIS_DATA_WIDTH : Data width of the AXI4-stream port
 * @tparam BURST_SIZE : Burst length of the AXI4 (max beats - 256)
 * 
 * @param mem_in0 : first memory port
 * @param mem_in1 : second memory port
 * @param stream_out : output AXI4-stream
 * @param size : Number of bytes of the data
 * @param selector : Memory location selector. 0 for mem_in0 and 1 for mem_in1
 */
template <unsigned int MEM_DATA_WIDTH, unsigned int AXIS_DATA_WIDTH, unsigned int BURST_SIZE=32>
void mem2axis(ap_uint<MEM_DATA_WIDTH>* mem_in0, 
				ap_uint<MEM_DATA_WIDTH>* mem_in1,
				::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_out,
				unsigned int size,
				unsigned int selector)
{
#ifndef __SYNTHESIS__
	static_assert(MEM_DATA_WIDTH % AXIS_DATA_WIDTH == 0, 
			"MEM_DATA_WIDTH has to be fully divided by AXIS_DATA_WIDTH");
	static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
			"MEM_DATA_WIDTH failed limit check");
	static_assert(AXIS_DATA_WIDTH >= min_axis_data_width && AXIS_DATA_WIDTH <= max_axis_data_width,
			"AXIS_DATA_WIDTH failed limit check");
	static_assert(BURST_SIZE >= min_burst_len && BURST_SIZE <= max_burst_len,
			" BURST_SIZE has failed limit check");
#endif
	
	constexpr unsigned int bytes_per_beat = MEM_DATA_WIDTH / 8;
	constexpr unsigned int bytes_per_axis_pkt = AXIS_DATA_WIDTH / 8;
	constexpr unsigned int num_strm_pkts_per_beat = MEM_DATA_WIDTH / AXIS_DATA_WIDTH;

	const unsigned int num_beats = (size + bytes_per_beat - 1) / bytes_per_beat;
	const unsigned int num_bursts = num_beats / BURST_SIZE;
	const unsigned int non_burst_beats = num_beats % BURST_SIZE;
	
	unsigned int index = 0;
	
	
	switch (selector)
	{
		case 0:
			for (unsigned int brst = 0; brst < num_bursts; brst++)
			{
			#pragma HLS LOOP_TRIPCOUNT avg=avg_num_of_bursts max=max_num_of_bursts

				for (unsigned int beat = 0; beat < BURST_SIZE; beat++)
				{
				#pragma HLS PIPELINE II=num_strm_pkts_per_beat
				#pragma HLS LOOP_TRIPCOUNT min=min_burst_len avg=avg_burst_len max=max_burst_len

					convMemBeat2axisPkt<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>(mem_in0, strm_out, size, index);
					index++;
				}
			}
			
			for (unsigned int beat = 0; beat < non_burst_beats; beat++)
			{
			#pragma HLS PIPELINE II=num_strm_pkts_per_beat
				convMemBeat2axisPkt<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>(mem_in0, strm_out, size, index);
				index++;
			}
			
			break;
		case 1:
			for (unsigned int brst = 0; brst < num_bursts; brst++)
			{
			#pragma HLS LOOP_TRIPCOUNT avg=avg_num_of_bursts max=max_num_of_bursts

				for (unsigned int beat = 0; beat < BURST_SIZE; beat++)
				{
				#pragma HLS PIPELINE II=num_strm_pkts_per_beat
				#pragma HLS LOOP_TRIPCOUNT min=min_burst_len avg=avg_burst_len max=max_burst_len

					convMemBeat2axisPkt<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>(mem_in1, strm_out, size, index);
					index++;
				}
			}
			
			for (unsigned int beat = 0; beat < non_burst_beats; beat++)
			{
			#pragma HLS PIPELINE II=num_strm_pkts_per_beat
				convMemBeat2axisPkt<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>(mem_in1, strm_out, size, index);
				index++;
			}
			
			break;	
	}
}

/**
 * @brief 	streamSpliter is a component reads one hlsStream and split to ndim split
 *
 * @tparam STREAM_DATA_WIDTH : Data width of the hls-stream port
 * @tparam ...T : variadic parameter read channel types
 *
 * @param stream_in : input hls-stream
 * @param ...steam_out : variadic stream out
 */
//TODO: inmplement to split hls channels in daflow split

/**
 * @brief 	axis2mem reads from an AXI4 stream into a memory location.
 *  		This is optimized to write to AXI4 with burst and to utilize width conversion in-between
 *  		AXI4 to AXI4-stream
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port
 * @tparam AXIS_DATA_WIDTH : Data width of the AXI4-stream port
 * @tparam BURST_SIZE : Burst length of the AXI4 (max beats - 256)
 *
 * @param mem_out : output memory port
 * @param stream_in : input AXI4-stream
 * @param size : Number of bytes of the data
 */
template <unsigned int MEM_DATA_WIDTH, unsigned int AXIS_DATA_WIDTH, unsigned int BURST_SIZE=32>
void axis2mem(ap_uint<MEM_DATA_WIDTH>* mem_out,
				::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_in,
				unsigned int size)
{
#ifndef __SYNTHESIS__
	static_assert(MEM_DATA_WIDTH % AXIS_DATA_WIDTH == 0,
			"MEM_DATA_WIDTH has to be fully divided by AXIS_DATA_WIDTH");
	static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
			"MEM_DATA_WIDTH failed limit check");
	static_assert(AXIS_DATA_WIDTH >= min_axis_data_width && AXIS_DATA_WIDTH <= max_axis_data_width,
			"AXIS_DATA_WIDTH failed limit check");
	static_assert(BURST_SIZE >= min_burst_len && BURST_SIZE <= max_burst_len,
			" BURST_SIZE has failed limit check");
#endif

	constexpr unsigned int bytes_per_beat = MEM_DATA_WIDTH / 8;
	constexpr unsigned int bytes_per_axis_pkt = AXIS_DATA_WIDTH / 8;
	constexpr unsigned int num_strm_pkts_per_beat = MEM_DATA_WIDTH / AXIS_DATA_WIDTH;

	const unsigned int num_beats = (size + bytes_per_beat - 1) / bytes_per_beat;
	const unsigned int num_bursts = num_beats / BURST_SIZE;
	const unsigned int non_burst_beats = num_beats % BURST_SIZE;

	unsigned int index = 0;

	for (unsigned int brst = 0; brst < num_bursts; brst++)
	{
	#pragma HLS LOOP_TRIPCOUNT avg=avg_num_of_bursts max=max_num_of_bursts

		for (unsigned int beat = 0; beat < BURST_SIZE; beat++)
		{
		#pragma HLS PIPELINE II=num_strm_pkts_per_beat
		#pragma HLS LOOP_TRIPCOUNT min=min_burst_len avg=avg_burst_len max=max_burst_len

			convAxisPkt2memBeat<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>(mem_out, strm_in, size, index);
			index++;
		}
	}

	for (unsigned int beat = 0; beat < non_burst_beats; beat++)
	{
	#pragma HLS PIPELINE II=num_strm_pkts_per_beat
		convAxisPkt2memBeat<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>(mem_out, strm_in, size, index);
		index++;
	}

}

/**
 * @brief 	axis2memV2 reads from an AXI4 stream into a memory location.
 *  		This is optimized to write to AXI4 with burst and to utilize width conversion in-between
 *  		AXI4 to AXI4-stream
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port
 * @tparam AXIS_DATA_WIDTH : Data width of the AXI4-stream port
 * @tparam BURST_SIZE : Burst length of the AXI4 (max beats - 256)
 *
 * @param mem_out : output memory port
 * @param stream_in : input AXI4-stream
 * @param size : Number of bytes of the data
 */
template <unsigned int MEM_DATA_WIDTH, unsigned int AXIS_DATA_WIDTH, unsigned int BURST_SIZE=32>
void axis2memV2(ap_uint<MEM_DATA_WIDTH>* mem_out,
				::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_in,
				unsigned int size)
{
#ifndef __SYNTHESIS__
	static_assert(MEM_DATA_WIDTH % AXIS_DATA_WIDTH == 0,
			"MEM_DATA_WIDTH has to be fully divided by AXIS_DATA_WIDTH");
	static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
			"MEM_DATA_WIDTH failed limit check");
	static_assert(AXIS_DATA_WIDTH >= min_axis_data_width && AXIS_DATA_WIDTH <= max_axis_data_width,
			"AXIS_DATA_WIDTH failed limit check");
	static_assert(BURST_SIZE >= min_burst_len && BURST_SIZE <= max_burst_len,
			" BURST_SIZE has failed limit check");
#endif

	constexpr unsigned int bytes_per_beat = MEM_DATA_WIDTH / 8;
	constexpr unsigned int bytes_per_axis_pkt = AXIS_DATA_WIDTH / 8;
	constexpr unsigned int num_strm_pkts_per_beat = MEM_DATA_WIDTH / AXIS_DATA_WIDTH;

	const unsigned int num_beats = (size + bytes_per_beat - 1) / bytes_per_beat;


#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG| %s | size: %d, num_beats: %d\n"
			, __func__, size, num_beats);
	printf("====================================================================================\n");
#endif

	::hls::stream<ap_uint<MEM_DATA_WIDTH>> mem_strm;
	::hls::stream<ap_uint<AXIS_DATA_WIDTH>> red_mem_strm;
#pragma HLS STREAM variable = mem_strm depth = max_depth_v16
#pragma HLS STREAM variable = red_mem_strm depth = max_depth_v8

#pragma HLS DATAFLOW
	axis2stream<AXIS_DATA_WIDTH>(strm_in, red_mem_strm, num_beats*num_strm_pkts_per_beat);
	stream2stream<AXIS_DATA_WIDTH,MEM_DATA_WIDTH>(red_mem_strm, mem_strm, num_beats);
	stream2mem<MEM_DATA_WIDTH>(mem_out, mem_strm, num_beats);
}


/**
 * @brief 	axis2mem reads from an AXI4 stream into a memory location with a selector mux to select out of two.
 *  		This is optimized to write to AXI4 with burst and to utilize width conversion in-between
 *  		AXI4 to AXI4-stream
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port
 * @tparam AXIS_DATA_WIDTH : Data width of the AXI4-stream port
 * @tparam BURST_SIZE : Burst length of the AXI4 (max beats - 256)
 *
 * @param mem_out0 : first memory port
 * @param mem_out1 : second memory port
 * @param stream_in : input AXI4-stream
 * @param size : Number of bytes of the data
 * @param selector : Memory location selector. 0 for mem_out0 and 1 for mem_out1
 */
template <unsigned int MEM_DATA_WIDTH, unsigned int AXIS_DATA_WIDTH, unsigned int BURST_SIZE=32>
void axis2mem(ap_uint<MEM_DATA_WIDTH>* mem_out0,
				ap_uint<MEM_DATA_WIDTH>* mem_out1,
				::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_in,
				unsigned int size,
				unsigned int selector)
{
#ifndef __SYNTHESIS__
	static_assert(MEM_DATA_WIDTH % AXIS_DATA_WIDTH == 0,
			"MEM_DATA_WIDTH has to be fully divided by AXIS_DATA_WIDTH");
	static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
			"MEM_DATA_WIDTH failed limit check");
	static_assert(AXIS_DATA_WIDTH >= min_axis_data_width && AXIS_DATA_WIDTH <= max_axis_data_width,
			"AXIS_DATA_WIDTH failed limit check");
	static_assert(BURST_SIZE >= min_burst_len && BURST_SIZE <= max_burst_len,
			" BURST_SIZE has failed limit check");
#endif

	constexpr unsigned int bytes_per_beat = MEM_DATA_WIDTH / 8;
	constexpr unsigned int bytes_per_axis_pkt = AXIS_DATA_WIDTH / 8;
	constexpr unsigned int num_strm_pkts_per_beat = MEM_DATA_WIDTH / AXIS_DATA_WIDTH;

	const unsigned int num_beats = (size + bytes_per_beat - 1) / bytes_per_beat;
	const unsigned int num_bursts = num_beats / BURST_SIZE;
	const unsigned int non_burst_beats = num_beats % BURST_SIZE;

	unsigned int index = 0;

	switch (selector)
	{
		case 0:
			for (unsigned int brst = 0; brst < num_bursts; brst++)
			{
			#pragma HLS LOOP_TRIPCOUNT avg=avg_num_of_bursts max=max_num_of_bursts

				for (unsigned int beat = 0; beat < BURST_SIZE; beat++)
				{
				#pragma HLS PIPELINE II=num_strm_pkts_per_beat
				#pragma HLS LOOP_TRIPCOUNT min=min_burst_len avg=avg_burst_len max=max_burst_len

					convAxisPkt2memBeat<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>(mem_out0, strm_in, size, index);
					index++;
				}
			}

			for (unsigned int beat = 0; beat < non_burst_beats; beat++)
			{
			#pragma HLS PIPELINE II=num_strm_pkts_per_beat
				convAxisPkt2memBeat<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>(mem_out0, strm_in, size, index);
				index++;
			}

			break;
		case 1:
			for (unsigned int brst = 0; brst < num_bursts; brst++)
			{
			#pragma HLS LOOP_TRIPCOUNT avg=avg_num_of_bursts max=max_num_of_bursts

				for (unsigned int beat = 0; beat < BURST_SIZE; beat++)
				{
				#pragma HLS PIPELINE II=num_strm_pkts_per_beat
				#pragma HLS LOOP_TRIPCOUNT min=min_burst_len avg=avg_burst_len max=max_burst_len

					convAxisPkt2memBeat<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>(mem_out1, strm_in, size, index);
					index++;
				}
			}

			for (unsigned int beat = 0; beat < non_burst_beats; beat++)
			{
			#pragma HLS PIPELINE II=num_strm_pkts_per_beat
				convAxisPkt2memBeat<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>(mem_out1, strm_in, size, index);
				index++;
			}

			break;
	}
}

/**
 * @brief 	axis2mem reads from an AXI4 stream into a memory location.
 *  		This is optimized to write to AXI4 with burst and to utilize width conversion in-between
 *  		AXI4 to AXI4-stream
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port
 * @tparam AXIS_DATA_WIDTH : Data width of the AXI4-stream port
 * @tparam BURST_SIZE : Burst length of the AXI4 (max beats - 256)
 *
 * @param mem_out : output memory port
 * @param stream_in : input AXI4-stream
 * @param size : Number of bytes of the data
 */
template <unsigned int DATA_WIDTH, unsigned int AXIS_DATA_WIDTH>
void axis2memMasked(ap_uint<DATA_WIDTH>* mem_out,
				::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_in,
				unsigned int size)
{
#ifndef __SYNTHESIS__
	static_assert(AXIS_DATA_WIDTH % DATA_WIDTH == 0,
			"AXIS_DATA_WIDTH has to be fully divided by DATA_WIDTH");
	static_assert(AXIS_DATA_WIDTH >= min_axis_data_width && AXIS_DATA_WIDTH <= max_axis_data_width,
			"AXIS_DATA_WIDTH failed limit check");
#endif

	constexpr unsigned int bytes_per_axis_pkt = AXIS_DATA_WIDTH / 8;
	constexpr unsigned int data_per_axis_pkt = AXIS_DATA_WIDTH / DATA_WIDTH;
	const unsigned int num_of_axis_pkt = (size + bytes_per_axis_pkt - 1) / bytes_per_axis_pkt;
//	const unsigned int num_bursts = num_beats / BURST_SIZE;
//	const unsigned int non_burst_beats = num_beats % BURST_SIZE;

	unsigned int index = 0;

	for (unsigned int brst = 0; brst < num_of_axis_pkt; brst++)
	{
	#pragma HLS PIPELINE II=data_per_axis_pkt
	#pragma HLS LOOP_TRIPCOUNT min=1 avg=8

		convAxisPkt2memBeatMasked<DATA_WIDTH,AXIS_DATA_WIDTH>(mem_out, strm_in, size, index);
		index += data_per_axis_pkt;
	}
}

/**
 * @brief axis2stream converts AXI4-stream to hls-stream	
 *
 * @tparam AXIS_DATA_WIDTH : Data width of the AXI4-stream port
 * @tparam STREAM_DATA_WIDTH : Data width of the HLS-stream port
 * 
 * @param axis_in : AXI4-stream input port
 * @param strm_out : HLS-stream output port
 * @param size : Number of the bytes of data
 */
template <unsigned int AXIS_DATA_WIDTH, unsigned int STREAM_DATA_WIDTH>
void axis2stream(::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& axis_in,
				::hls::stream<ap_uint<STREAM_DATA_WIDTH>>& strm_out,
				unsigned int size)
{
#ifndef __SYNTHESIS__
	static_assert(AXIS_DATA_WIDTH % STREAM_DATA_WIDTH == 0,
			"AXIS_DATA_WIDTH has to be fully divided by STREAM_DATA_WIDTH");
	static_assert(AXIS_DATA_WIDTH >= min_axis_data_width && AXIS_DATA_WIDTH <= max_axis_data_width,
			"AXIS_DATA_WIDTH failed limit check");
	static_assert(STREAM_DATA_WIDTH >= min_hls_stream_data_width && AXIS_DATA_WIDTH <= max_hls_stream_data_width,
			"STREAM_DATA_WIDTH failed limit check");
#endif

	constexpr unsigned int num_hls_pkt_per_axis_pkt = AXIS_DATA_WIDTH / STREAM_DATA_WIDTH;
	constexpr unsigned int bytes_per_axis_pkt = AXIS_DATA_WIDTH / 8;
	
	const unsigned int num_axis_pkts = (size + bytes_per_axis_pkt - 1) / bytes_per_axis_pkt;

#ifdef DEBUG_LOG_PRINT
			printf("|HLS DEBUG_LOG|%s| starting. size_bytes: %d, num_axis_pkt: %d, num_hls_pkt_per_axis_pkt: %d\n"
						, __func__, size, num_axis_pkts, num_hls_pkt_per_axis_pkt);
#endif

	ap_axiu<AXIS_DATA_WIDTH,0,0,0> axisPkt;

	for (unsigned int itr = 0; itr < num_axis_pkts; itr++)
	{
		for (unsigned int j = 0; j < num_hls_pkt_per_axis_pkt; j++)
		{
		#pragma HLS PIPELINE II=1
		#pragma HLS LOOP_FLATTEN

			if (j == 0)
			{
				axisPkt = axis_in.read();
			}
			strm_out <<  axisPkt.data.range((j+1) * STREAM_DATA_WIDTH - 1, j * STREAM_DATA_WIDTH);

#ifdef DEBUG_LOG_PRINT
			printf("   |HLS DEBUG_LOG|%s| sending axis pkt: %d, val=(",__func__, itr);

			for (unsigned n = 0; n < STREAM_DATA_WIDTH/(DEBUG_LOG_SIZE_OF * 8); n++)
			{
				DataConv conv;
				conv.i = axisPkt.data.range((n+1) * DEBUG_LOG_SIZE_OF * 8 - 1, n * DEBUG_LOG_SIZE_OF * 8);
				printf("%f,", conv.f);
			}
			printf(")\n");
#endif

#ifdef DEBUG_LOG_PRINT
			printf("|HLS DEBUG_LOG|%s| reading axis pkt: %d.\n"
						, __func__, itr);
#endif
		}
	}
#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| exiting.\n"
			, __func__);
#endif
}

/**
 * @brief axis2streamMasked converts AXI4-stream with strb channel to
 * hls-stream data stream and hls-stream mask stream
 *
 * @tparam AXIS_DATA_WIDTH : Data width of the AXI4-stream port
 * @tparam STREAM_DATA_WIDTH : Data width of the HLS-stream port
 *
 * @param axis_in : AXI4-stream input port
 * @param data_out : HLS-stream data output port
 * @param mask_out : HLS-stream mask output port
 * @param size : Number of the bytes of data
 */
template <unsigned int AXIS_DATA_WIDTH, unsigned int STREAM_DATA_WIDTH>
void axis2streamMasked(::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& axis_in,
				::hls::stream<ap_uint<STREAM_DATA_WIDTH>>& data_out,
				::hls::stream<ap_uint<STREAM_DATA_WIDTH/8>>& mask_out,
				unsigned int size,
				unsigned short shiftBytes = 0)
{
#ifndef __SYNTHESIS__
	static_assert(AXIS_DATA_WIDTH % STREAM_DATA_WIDTH == 0,
			"AXIS_DATA_WIDTH has to be fully divided by STREAM_DATA_WIDTH");
	static_assert(AXIS_DATA_WIDTH >= min_axis_data_width && AXIS_DATA_WIDTH <= max_axis_data_width,
			"AXIS_DATA_WIDTH failed limit check");
	static_assert(STREAM_DATA_WIDTH >= min_hls_stream_data_width && AXIS_DATA_WIDTH <= max_hls_stream_data_width,
			"STREAM_DATA_WIDTH failed limit check");
#endif

	constexpr unsigned int num_hls_pkt_per_axis_pkt = AXIS_DATA_WIDTH / STREAM_DATA_WIDTH;
	constexpr unsigned int bytes_per_axis_pkt = AXIS_DATA_WIDTH / 8;
	constexpr unsigned int bytes_per_hls_pkt = AXIS_DATA_WIDTH / 8;
	const unsigned int num_axis_pkts = (size + bytes_per_axis_pkt - 1) / bytes_per_axis_pkt;
	const unsigned short shift_bits = shiftBytes * 8;

	ap_axiu<AXIS_DATA_WIDTH,0,0,0> cycleBuf[2];
	ap_uint<1> cycleBufIndex = 0;
#pragma HLS ARRAY_PARTITION variable=cycleBuf type=complete


	//first read
//	if (num_axis_pkts)
//	{
	cycleBuf[cycleBufIndex] = axis_in.read();

#ifdef DEBUG_LOG_PRINT
	printf("   |HLS DEBUG_LOG||%s| Iter 0. Cycle Buff, ", __func__);

	for (unsigned int i = 0; i < 2; i++)
	{
		printf(" id:%d - val=(",i);
		for (unsigned int n = 0; n < bytes_per_axis_pkt/DEBUG_LOG_SIZE_OF; n++)
		{
			DataConv tmp;
			tmp.i = cycleBuf[i].data.range((n+1) * DEBUG_LOG_SIZE_OF * 8 - 1, n * DEBUG_LOG_SIZE_OF * 8);
			printf("%f,", tmp.f);
		}

		printf(") strb=(%x)", cycleBuf[i].strb);
	}
	printf("\n");
#endif

	for (unsigned int j = 0; j < num_hls_pkt_per_axis_pkt; j++)
	{
	#pragma HLS PIPELINE II=1
		data_out << cycleBuf[cycleBufIndex].data.range((j+1) * STREAM_DATA_WIDTH - 1, j * STREAM_DATA_WIDTH);
		mask_out << cycleBuf[cycleBufIndex].strb.range((j+1) * bytes_per_hls_pkt -1, j * bytes_per_hls_pkt);
	}
	cycleBufIndex += 1;
//	}

	for (unsigned int itr = 1; itr < num_axis_pkts; itr++)
	{
		cycleBuf[cycleBufIndex] = axis_in.read();
#ifdef DEBUG_LOG_PRINT
		printf("   |HLS DEBUG_LOG||%s| Iter %d. Cycle Buff, ", __func__, itr);

		for (unsigned int i = 0; i < 2; i++)
		{
			printf(" id:%d - val=(",i);
			for (unsigned int n = 0; n < bytes_per_axis_pkt/DEBUG_LOG_SIZE_OF; n++)
			{
				DataConv tmp;
				tmp.i = cycleBuf[i].data.range((n+1) * DEBUG_LOG_SIZE_OF * 8 - 1, n * DEBUG_LOG_SIZE_OF * 8);
				printf("%f,", tmp.f);
			}

			printf(") strb=(%x)", cycleBuf[i].strb);
		}
		printf("\n");
#endif
		ap_uint<1> cycleBufIndexPlusOne = cycleBufIndex + 1;

		ap_axiu<AXIS_DATA_WIDTH,0,0,0> axisPkt;
		axisPkt.data.range(AXIS_DATA_WIDTH - 1, shift_bits)
				= cycleBuf[cycleBufIndex].data.range(AXIS_DATA_WIDTH - 1 - shift_bits, 0);
		axisPkt.strb.range(bytes_per_axis_pkt - 1, shiftBytes)
				= cycleBuf[cycleBufIndex].strb.range(bytes_per_axis_pkt - 1 - shiftBytes, 0);

		if (shift_bits)
		{
			axisPkt.data.range(shift_bits - 1, 0)
					= cycleBuf[cycleBufIndexPlusOne].data.range(AXIS_DATA_WIDTH - 1, AXIS_DATA_WIDTH - shift_bits);
			axisPkt.strb.range(shiftBytes - 1, 0)
					= cycleBuf[cycleBufIndexPlusOne].strb.range(bytes_per_axis_pkt - 1, bytes_per_axis_pkt - shiftBytes);
		}

	#ifdef DEBUG_LOG_PRINT
		printf("   |HLS DEBUG_LOG||%s| shifted axis , val=(", __func__);

		for (unsigned n = 0; n < bytes_per_axis_pkt/DEBUG_LOG_SIZE_OF; n++)
		{
			DataConv tmp;
			tmp.i = axisPkt.data.range((n+1) * DEBUG_LOG_SIZE_OF * 8 - 1, n * DEBUG_LOG_SIZE_OF * 8);
			printf("%f,", tmp.f);
		}

		printf(") strb=(%x)\n", axisPkt.strb);
	#endif

		for (unsigned int j = 0; j < num_hls_pkt_per_axis_pkt; j++)
		{
		#pragma HLS PIPELINE II=1
			data_out << axisPkt.data.range((j+1) * STREAM_DATA_WIDTH - 1, j * STREAM_DATA_WIDTH);
			mask_out << axisPkt.strb.range((j+1) * bytes_per_hls_pkt - 1, j * bytes_per_hls_pkt);
		}
		cycleBufIndex += 1;
	}
}

/**
 * @brief getNumStreamPkts produced number of stream pkts need for read grid with given range
 *
 * @details
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port
 * @tparam AXIS_DATA_WIDTH : Data width of the AXIS-stream port
 * @tparam DATA_WIDTH : Data width of the base datatype
 *
 * @param range : read range
 * @param n_pkts : number_of axis pkts need
 */
template <unsigned int MEM_DATA_WIDTH, unsigned int AXIS_DATA_WIDTH, unsigned int DATA_WIDTH=32>
void getNumStreamPkts(const AccessRange& range, unsigned int& n_pkts)
{
	constexpr unsigned short data_vector_factor = MEM_DATA_WIDTH / DATA_WIDTH;
	constexpr unsigned short num_pkts_per_mem_beat = MEM_DATA_WIDTH/AXIS_DATA_WIDTH;
	unsigned short ShiftBits = (unsigned short)LOG2(data_vector_factor);
	unsigned short DataShiftBits = (unsigned short)LOG2(DATA_WIDTH/8);
	unsigned short start_x = range.start[0] >> ShiftBits;
	unsigned short end_x = (range.end[0] + data_vector_factor - 1) >> ShiftBits;
	unsigned short num_xblocks = end_x - start_x;
	unsigned short num_xpkts = num_xblocks * num_pkts_per_mem_beat;

#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| range: (%d, %d, %d) -> (%d, %d, %d)\n"
			, __func__, range.start[0], range.start[1], range.start[2], range.end[0], range.end[1], range.end[2]);
#endif
	unsigned short diff_y = range.end[1] - range.start[1];
	unsigned short diff_z = range.end[2] - range.start[2];

#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| num_xpkts: %d, diff_y: %d, diff_z: %d \n"
			, __func__, num_xpkts, diff_y, diff_z);
#endif

	n_pkts = num_xpkts * diff_y * diff_z;
}
/**
 * @brief axisLoopback reads from one grid data axis-stream to another grid data axis stream
 *
 * @details This component is to support memory looback from the output of PE to the input in next iteration
 * without copying back to memory.
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port
 * @tparam AXIS_DATA_WIDTH : Data width of the AXIS-stream port
 * @tparam DATA_WIDTH : Data width of the base datatype
 *
 * @param strm_in : AXIS-stream input port
 * @param strm_out : AXIS-stream output port
 * @param n_pkts : number_of axis pkts need to be loopback
 */
template <unsigned int MEM_DATA_WIDTH, unsigned int AXIS_DATA_WIDTH, unsigned int DATA_WIDTH=32>
void axisLoopbackV2(
		::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_in,
		 ::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_out,
		 const unsigned int n_pkts)
{
	ap_axiu<AXIS_DATA_WIDTH,0,0,0> tmp;
	for (unsigned short pkt = 0; pkt < n_pkts; pkt++)
	{
#pragma HLS PIPELINE II=1

#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| writing. pkt:%d\n"
			, __func__, pkt);
#endif
		tmp = strm_in.read();
		strm_out.write(tmp);
	}

#ifdef DEBUG_LOG_PRINT
			printf("|HLS DEBUG_LOG|%s| exiting. \n"
					, __func__);
#endif
}

/**
 * @brief axisLoopback reads from one grid data axis-stream to another grid data axis stream
 *
 * @details This component is to support memory looback from the output of PE to the input in next iteration
 * without copying back to memory.
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port
 * @tparam AXIS_DATA_WIDTH : Data width of the AXIS-stream port
 * @tparam DATA_WIDTH : Data width of the base datatype
 *
 * @param strm_in : AXIS-stream input port
 * @param strm_out : AXIS-stream output port
 * @param gridSize : gridSize of the original grid
 * @param range : Access range to be writen data
 */
template <unsigned int MEM_DATA_WIDTH, unsigned int AXIS_DATA_WIDTH, unsigned int DATA_WIDTH=32>
void axisLoopback(
		::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_in,
		 ::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_out,
		SizeType& gridSize,
		AccessRange& range)
{
	constexpr unsigned short data_vector_factor = MEM_DATA_WIDTH / DATA_WIDTH;
	constexpr unsigned short pkts_per_beat = MEM_DATA_WIDTH / AXIS_DATA_WIDTH;
	const unsigned short ShiftBits = (unsigned short)LOG2(data_vector_factor);
	const unsigned short PktsShiftBits = (unsigned short)LOG2(pkts_per_beat);
	unsigned short start_x = range.start[0] >> ShiftBits;
	start_x = start_x << PktsShiftBits;
	unsigned short end_x = (range.end[0] + data_vector_factor - 1) >> ShiftBits;
	end_x = end_x << PktsShiftBits;
	unsigned short grid_xblocks = gridSize[0] >> ShiftBits; //GridSize[0] has to be MEM_DATA_WIDTH aligned
	unsigned short num_xpkts = end_x - start_x;

#ifdef DEBUG_LOG_PRINT
			printf("|HLS DEBUG_LOG|%s| starting range: (%d,%d,%d) -> (%d,%d,%d), start_x: %d, end_x:%d, num xpkts: %d\n"
					, __func__, range.start[0], range.start[1], range.start[2], range.end[0], range.end[1], range.end[2],
					start_x, end_x, num_xpkts);
#endif
	if (range.dim < 3)
	{
		range.start[2] = 0;
		range.end[2] = 1;
	}
	else if (range.dim < 2)
	{
		range.start[1] = 0;
		range.end[1] = 1;
	}

	for (unsigned short k = range.start[2]; k < range.end[2]; k++)
	{
		for (unsigned short j = range.start[1]; j < range.end[1]; j++)
		{
			for (unsigned short x_pkt = start_x; x_pkt < end_x; x_pkt++)
			{
 #pragma HLS PIPELINE II = 1

#ifdef DEBUG_LOG_PRINT
			printf("|HLS DEBUG_LOG|%s| writing. offset:%d, j:%d, k:%d\n"
					, __func__, x_pkt, j, k);
#endif
				ap_axiu<AXIS_DATA_WIDTH,0,0,0> pkt = strm_in.read();
				strm_out.write(pkt);
			}
		}
	}
#ifdef DEBUG_LOG_PRINT
			printf("|HLS DEBUG_LOG|%s| exiting. \n"
					, __func__);
#endif
}

/**
 * @brief axisTerminate read from axis stream and discard the packets
 *
 * @tparam AXIS_DATA_WIDTH : Data width of the AXI4-stream port
 * @tparam II : Initial Interval of the read
 * 
 * @param axis_in : AXI4-stream input
 * @param num_pkts: number of axis pkts
 */
template <unsigned int AXIS_DATA_WIDTH, unsigned int II=1>
void axisTerminate(::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& axis_in,
		unsigned int num_pkts)
{
#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| starting.\n"
			, __func__);
#endif
	for (unsigned int i = 0; i < num_pkts; i++)
	{
#pragma HLS PIPELINE II=II
#ifdef DEBUG_LOG_PRINT
		printf("|HLS DEBUG_LOG|%s| terminating pkt:%d.\n"
				, __func__, i);
#endif
		auto pkt = axis_in.read();
	}
#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| exiting.\n"
			, __func__);
#endif
}

/**
 * @brief stream2axis converts hls-stream to AXI4-stream
 *
 * @tparam AXIS_DATA_WIDTH : Data width of the AXI4-stream port
 * @tparam STREAM_DATA_WIDTH : Data width of the HLS-stream port
 * 
 * @param axis_out : AXI4-stream output port
 * @param strm_in : HLS-stream input port
 * @param size : Number of the bytes of data
 */

template <unsigned int AXIS_DATA_WIDTH, unsigned int STREAM_DATA_WIDTH>
void stream2axis(::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& axis_out,
				::hls::stream<ap_uint<STREAM_DATA_WIDTH>>& strm_in,
				unsigned int size)
{
#ifndef __SYNTHESIS__
	static_assert(AXIS_DATA_WIDTH % STREAM_DATA_WIDTH == 0,
			"AXIS_DATA_WIDTH has to be fully divided by STREAM_DATA_WIDTH");
	static_assert(AXIS_DATA_WIDTH >= min_axis_data_width && AXIS_DATA_WIDTH <= max_axis_data_width,
			"AXIS_DATA_WIDTH failed limit check");
	static_assert(STREAM_DATA_WIDTH >= min_hls_stream_data_width && AXIS_DATA_WIDTH <= max_hls_stream_data_width,
			"STREAM_DATA_WIDTH failed limit check");
#endif

	constexpr unsigned int num_hls_pkt_per_axis_pkt = AXIS_DATA_WIDTH / STREAM_DATA_WIDTH;
	constexpr unsigned int bytes_per_axis_pkt = AXIS_DATA_WIDTH / 8;
	constexpr unsigned int bytes_per_hls_pkt = STREAM_DATA_WIDTH / 8;
	
	const unsigned int num_axis_pkts = (size + bytes_per_axis_pkt - 1) / bytes_per_axis_pkt;

#ifdef DEBUG_LOG_PRINT
		printf("|HLS DEBUG_LOG|%s| stream to axis. size (bytes): %d, num_hls_pkt_per_axis_pkt: %d, bytes_per_axis_pkt: %d, bytes_per_hls_pkt:%d, num_axis_pkts:%d\n"
				, __func__, size, num_hls_pkt_per_axis_pkt, bytes_per_axis_pkt, bytes_per_hls_pkt, num_axis_pkts);
		printf("====================================================================================\n");
#endif

	for (unsigned int itr = 0; itr < num_axis_pkts; itr++)
	{
		ap_axiu<AXIS_DATA_WIDTH,0,0,0> axisPkt;
#ifdef DEBUG_LOG_PRINT
		printf("|HLS DEBUG_LOG|%s| flag 2. itr:%d\n",
							__func__, itr);
#endif
		for (unsigned int j = 0; j < num_hls_pkt_per_axis_pkt; j++)
		{
		#pragma HLS PIPELINE II=1
#ifdef DEBUG_LOG_PRINT
			unsigned int hls_pkt_index = itr * num_hls_pkt_per_axis_pkt + j;
			printf("|HLS DEBUG_LOG|%s| reading hls pkt: %d\n",
					__func__, hls_pkt_index);
#endif
			axisPkt.data.range((j+1) * STREAM_DATA_WIDTH - 1, j * STREAM_DATA_WIDTH) = strm_in.read();
		}
#ifdef DEBUG_LOG_PRINT
		printf("|HLS DEBUG_LOG|%s| wrting axis pkt: %d\n",
				__func__, itr);
#endif
		axis_out.write(axisPkt);
	}
#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| exiting.\n"
			, __func__);
#endif
}

/**
 * @brief stream2axisMasked converts hls-stream to AXI4-stream with bit-mask for using
 * tsrb channel of axi4-stream protocol
 *
 * @tparam AXIS_DATA_WIDTH : Data width of the AXI4-stream port
 * @tparam STREAM_DATA_WIDTH : Data width of the HLS-stream port
 * 
 * @param axis_out : AXI4-stream output port
 * @param strm_in : HLS-stream input port
 * @param mask_in : mask HLS-stream port for mask bits to be coverted to strb channel
 * @param size : Number of the bytes of data
 */

template <unsigned int AXIS_DATA_WIDTH, unsigned int STREAM_DATA_WIDTH>
void stream2axisMasked(::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& axis_out,
				::hls::stream<ap_uint<STREAM_DATA_WIDTH>>& strm_in,
				::hls::stream<ap_uint<STREAM_DATA_WIDTH/8>>& mask_in,
				unsigned int size)
{
#ifndef __SYNTHESIS__
	static_assert(AXIS_DATA_WIDTH % STREAM_DATA_WIDTH == 0,
			"AXIS_DATA_WIDTH has to be fully divided by STREAM_DATA_WIDTH");
	static_assert(AXIS_DATA_WIDTH >= min_axis_data_width && AXIS_DATA_WIDTH <= max_axis_data_width,
			"AXIS_DATA_WIDTH failed limit check");
	static_assert(STREAM_DATA_WIDTH >= min_hls_stream_data_width && AXIS_DATA_WIDTH <= max_hls_stream_data_width,
			"STREAM_DATA_WIDTH failed limit check");
#endif

	constexpr unsigned int num_hls_pkt_per_axis_pkt = AXIS_DATA_WIDTH / STREAM_DATA_WIDTH;
	constexpr unsigned int bytes_per_axis_pkt = AXIS_DATA_WIDTH / 8;
	constexpr unsigned int bytes_per_hls_pkt = STREAM_DATA_WIDTH / 8;
	
	const unsigned int num_axis_pkts = (size + bytes_per_axis_pkt - 1) / bytes_per_axis_pkt;

#ifdef DEBUG_LOG_PRINT
		printf("|HLS DEBUG_LOG|%s| stream to axis. size (bytes): %d, num_hls_pkt_per_axis_pkt: %d, bytes_per_axis_pkt: %d, bytes_per_hls_pkt:%d, num_axis_pkts:%d\n"
				, __func__, size, num_hls_pkt_per_axis_pkt, bytes_per_axis_pkt, bytes_per_hls_pkt, num_axis_pkts);
		printf("====================================================================================\n");
#endif

	for (unsigned int itr = 0; itr < num_axis_pkts; itr++)
	{
		ap_axiu<AXIS_DATA_WIDTH,0,0,0> axisPkt;
#ifdef DEBUG_LOG_PRINT
		printf("|HLS DEBUG_LOG|%s| flag 2. itr:%d\n",
							__func__, itr);
#endif
		for (unsigned int j = 0; j < num_hls_pkt_per_axis_pkt; j++)
		{
		#pragma HLS PIPELINE II=1
#ifdef DEBUG_LOG_PRINT
			unsigned int hls_pkt_index = itr * num_hls_pkt_per_axis_pkt + j;
			printf("|HLS DEBUG_LOG|%s| reading hls pkt: %d\n",
					__func__, hls_pkt_index);
#endif
			axisPkt.data.range((j+1) * STREAM_DATA_WIDTH - 1, j * STREAM_DATA_WIDTH) = strm_in.read();
			axisPkt.strb.range((j+1) * bytes_per_hls_pkt - 1, j * bytes_per_hls_pkt) = mask_in.read();

		}
#ifdef DEBUG_LOG_PRINT
		printf("|HLS DEBUG_LOG|%s| wrting axis pkt: %d\n",
				__func__, itr);
#endif
		axis_out.write(axisPkt);
	}
#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| exiting.\n"
			, __func__);
#endif
}

/**
 * @brief stream2memMasked converts hls-stream with strb mask to memory
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port
 * @tparam STREAM_DATA_WIDTH : Data width of the HLS-stream port
 * @tparam BURST_SIZE : Burst length of the AXI4 (max beats - 256)
 *
 * @param mem_out : AXI4-master memory output port
 * @param stream_in : HLS-stream input port
 * @param size : Number of the bytes of data
 */

//template <unsigned int MEM_DATA_WIDTH, unsigned int STREAM_DATA_WIDTH, unsigned int BURST_SIZE=32>
//void stream2mem(ap_uint<MEM_DATA_WIDTH>* mem_out,
//		::hls::stream<ap_uint<STREAM_DATA_WIDTH>>& strm_in,
//				unsigned int size)
//{
//#ifndef __SYNTHESIS__
//	static_assert(MEM_DATA_WIDTH % STREAM_DATA_WIDTH == 0,
//			"MEM_DATA_WIDTH has to be fully divided by STREAM_DATA_WIDTH");
//	static_assert(MEM_DATA_WIDTH >= min_mem_data_width && MEM_DATA_WIDTH <= max_mem_data_width,
//			"MEM_DATA_WIDTH failed limit check");
//	static_assert(STREAM_DATA_WIDTH >= min_hls_stream_data_width && STREAM_DATA_WIDTH <= max_hls_stream_data_width,
//			"STREAM_DATA_WIDTH failed limit check");
//	static_assert(BURST_SIZE >= min_burst_len && BURST_SIZE <= max_burst_len,
//			" BURST_SIZE has failed limit check");
//#endif
//
//	constexpr unsigned int bytes_per_beat = MEM_DATA_WIDTH / 8;
//	constexpr unsigned int bytes_per_stream_pkt = STREAM_DATA_WIDTH / 8;
//	constexpr unsigned int num_strm_pkts_per_beat = MEM_DATA_WIDTH / STREAM_DATA_WIDTH;
//
//	const unsigned int num_beats = (size + bytes_per_beat - 1) / bytes_per_beat;
//	const unsigned int num_bursts = num_beats / BURST_SIZE;
//	const unsigned int non_burst_beats = num_beats % BURST_SIZE;
//
//	unsigned int index = 0;
//
//	for (unsigned int brst = 0; brst < num_bursts; brst++)
//	{
//	#pragma HLS LOOP_TRIPCOUNT avg=avg_num_of_bursts max=max_num_of_bursts
//
//		for (unsigned int beat = 0; beat < BURST_SIZE; beat++)
//		{
//		#pragma HLS PIPELINE II=num_strm_pkts_per_beat
//		#pragma HLS LOOP_TRIPCOUNT min=min_burst_len avg=avg_burst_len max=max_burst_len
//
//			convStreamPkt2memBeat<MEM_DATA_WIDTH, STREAM_DATA_WIDTH>(mem_out, strm_in, size, index);
//			index++;
//		}
//	}
//
//	for (unsigned int beat = 0; beat < non_burst_beats; beat++)
//	{
//	#pragma HLS PIPELINE II=num_strm_pkts_per_beat
//		convStreamPkt2memBeat<MEM_DATA_WIDTH, STREAM_DATA_WIDTH>(mem_out, strm_in, size, index);
//		index++;
//	}
//
//}

/**
 * @brief stream2memMasked converts hls-stream with strb mask to memory
 *
 * @tparam STREAM_DATA_WIDTH : Data width of the HLS-stream port
 * @tparam DATA_WIDTH : Data width of the base datatype
 *
 * @param mem_out : AXI4-master memory output port
 * @param data_in : HLS-stream data input port
 * @param mask_in : HLS-stream mask input port
 * @param size : Number of the bytes of data
 */

template <unsigned int STREAM_DATA_WIDTH, unsigned int DATA_WIDTH>
void stream2memMasked(ap_uint<DATA_WIDTH>* mem_out,
		::hls::stream<ap_uint<STREAM_DATA_WIDTH>>& data_in,
		::hls::stream<ap_uint<STREAM_DATA_WIDTH/8>>& mask_in,
				unsigned int size)
{
#ifndef __SYNTHESIS__
	static_assert(STREAM_DATA_WIDTH % DATA_WIDTH == 0,
			"STREAM_DATA_WIDTH has to be fully divided by DATA_WIDTH");
	static_assert(STREAM_DATA_WIDTH >= min_hls_stream_data_width && STREAM_DATA_WIDTH <= max_hls_stream_data_width,
			"STREAM_DATA_WIDTH failed limit check");
#endif

	constexpr unsigned int bytes_per_strm_pkt = STREAM_DATA_WIDTH / 8;
	constexpr unsigned int data_per_strm_pkt = STREAM_DATA_WIDTH / DATA_WIDTH;
	const unsigned int num_of_strm_pkt = (size + bytes_per_strm_pkt - 1) / bytes_per_strm_pkt;

	unsigned int index = 0;

	for (unsigned int brst = 0; brst < num_of_strm_pkt; brst++)
	{
	#pragma HLS PIPELINE II=data_per_strm_pkt
	#pragma HLS LOOP_TRIPCOUNT min=1 avg=8

		convStreamPkt2memBeatMasked<DATA_WIDTH,STREAM_DATA_WIDTH>(mem_out, data_in, mask_in, size, index);
		index += data_per_strm_pkt;
	}
}


template <unsigned int MEM_DATA_WIDTH, unsigned int AXIS_DATA_WIDTH, unsigned int DATA_WIDTH=32>
void memReadGrid(ap_uint<DATA_WIDTH>* mem_in,
		::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_out,
		unsigned int size,
		SizeType gridSize,
		SizeType offset)
{
	unsigned int init_offset = offset[0] + offset[1] * gridSize[0] + offset[2] * gridSize[0] * gridSize[1];
	mem2axis<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>((ap_uint<MEM_DATA_WIDTH>*)(mem_in + init_offset), strm_out, size);
}

template <unsigned int MEM_DATA_WIDTH, unsigned int AXIS_DATA_WIDTH, unsigned int DATA_WIDTH=32>
void memReadGrid(ap_uint<DATA_WIDTH>* mem_in,
		::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_out,
		SizeType gridSize,
		AccessRange& range)
{
	constexpr unsigned short vectorFactor = AXIS_DATA_WIDTH / DATA_WIDTH;
	unsigned short ShiftBits = LOG2(vectorFactor);
	unsigned short DataShiftBits = LOG2(DATA_WIDTH/8);
	unsigned short num_xblocks = (range.end[0] - range.start[0] + vectorFactor - 1) >> ShiftBits;
	unsigned short x_tile_size = num_xblocks << ShiftBits;
	unsigned int x_tile_size_bytes = x_tile_size << DataShiftBits;

	if (range.dim < 3)
	{
		range.start[2] = 0;
		range.end[2] = 1;
	}
	else if (range.dim < 2)
	{
		range.start[1] = 0;
		range.end[1] = 1;
	}

#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| starting memReadGrid. grid_size: (%d, %d, %d), range: (%d, %d, %d) --> (%d, %d, %d)\n "
			"x_tile_size:%d, x_tile_size_bytes:%d, num_xblocks:%d\n"
			, __func__, ShiftBits, DataShiftBits, gridSize[0], gridSize[1], gridSize[2], range.start[0], range.start[1], range.start[2],
			range.end[0], range.end[1], range.end[2], x_tile_size, x_tile_size_bytes, num_xblocks);
	printf("===========================================================================================\n");
#endif

	for (unsigned short k = range.start[2]; k < range.end[2]; k++)
	{
		for (unsigned short j = range.start[1]; j < range.end[1]; j++)
		{
			unsigned int offset = range.start[0] + j * gridSize[0] + k * gridSize[1] * gridSize[0];

#ifdef DEBUG_LOG_PRINT
			printf("|HLS DEBUG_LOG|%s| reading. offset:%d, j:%d, k:%d\n"
					, __func__,offset, j, k);
#endif
			mem2axis<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>((ap_uint<MEM_DATA_WIDTH>*)(mem_in + offset), strm_out, x_tile_size_bytes);
		}
	}
#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| exiting.\n"
			, __func__);
#endif
}

template <unsigned int MEM_DATA_WIDTH, unsigned int AXIS_DATA_WIDTH, unsigned int DATA_WIDTH=32>
void memReadGridV2(ap_uint<MEM_DATA_WIDTH>* mem_in,
		::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_out,
		SizeType gridSize,
		AccessRange& range)
{
	constexpr unsigned short data_vector_factor = MEM_DATA_WIDTH / DATA_WIDTH;
	unsigned short ShiftBits = (unsigned short)LOG2(data_vector_factor);
	unsigned short DataShiftBits = (unsigned short)LOG2(DATA_WIDTH/8);
	unsigned short start_x = range.start[0] >> ShiftBits;
	unsigned short end_x = (range.end[0] + data_vector_factor - 1) >> ShiftBits;
	unsigned short grid_xblocks = gridSize[0] >> ShiftBits; //GridSize[0] has to be MEM_DATA_WIDTH aligned
	unsigned short num_xblocks = end_x - start_x;
	unsigned short x_tile_size = num_xblocks << ShiftBits;
	unsigned int x_tile_size_bytes = x_tile_size << DataShiftBits;

	if (range.dim < 3)
	{
		range.start[2] = 0;
		range.end[2] = 1;
	}
	if (range.dim < 2)
	{
		range.start[1] = 0;
		range.end[1] = 1;
	}

#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| starting memReadGrid. shiftbits: %d, data shift bits: %d, grid_size: (%d, %d, %d), grid_xblocks: %d, range: (%d, %d, %d) --> (%d, %d, %d)\n "
			"start_x_block: %d --> end_x_block: %d, x_tile_size:%d, x_tile_size_bytes:%d, num_xblocks:%d\n"
			, __func__, ShiftBits, DataShiftBits, gridSize[0], gridSize[1], gridSize[2], grid_xblocks, range.start[0], range.start[1], range.start[2],
			range.end[0], range.end[1], range.end[2], start_x, end_x, x_tile_size, x_tile_size_bytes, num_xblocks);
	printf("===========================================================================================\n");
#endif
	unsigned short diff_y = range.end[1] - range.start[1];
	bool isContinous = (grid_xblocks == num_xblocks and (diff_y == gridSize[1] or not range.dim == 3)) or range.dim == 1;

	if (isContinous)
	{
#ifdef DEBUG_LOG_PRINT
		printf("|HLS DEBUG_LOG|%s| continuous read\n", __func__);
#endif
		unsigned int offset = start_x + range.start[1] * grid_xblocks + range.start[2] * gridSize[1] * grid_xblocks;
		unsigned int size_y = range.end[1] - range.start[1];
		unsigned int size_z = range.end[2] - range.start[2];
		unsigned int  size_bytes = x_tile_size_bytes * size_y * size_z;

#ifdef DEBUG_LOG_PRINT
		printf("|HLS DEBUG_LOG|%s| init offset:%d, size_y:%d, size_z:%d, size_bytes:%d\n", __func__, offset, size_y, size_z, size_bytes);
#endif
		mem2axisV2<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>((ap_uint<MEM_DATA_WIDTH>* )(mem_in + offset), strm_out, size_bytes);
	}
	else
	{
		for (unsigned short k = range.start[2]; k < range.end[2]; k++)
		{
			for (unsigned short j = range.start[1]; j < range.end[1]; j++)
			{
				unsigned int offset = start_x + j * grid_xblocks + k * gridSize[1] * grid_xblocks;

		#ifdef DEBUG_LOG_PRINT
				printf("|HLS DEBUG_LOG|%s| reading. offset:%d, j:%d, k:%d\n"
						, __func__,offset, j, k);
		#endif
				mem2axisV2<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>((ap_uint<MEM_DATA_WIDTH>* )(mem_in + offset), strm_out, x_tile_size_bytes);
			}
		}
	}
#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| exiting.\n"
			, __func__);
#endif
}

template <unsigned int MEM_DATA_WIDTH, unsigned int AXIS_DATA_WIDTH, unsigned int DATA_WIDTH=32>
void memWriteGrid(ap_uint<DATA_WIDTH>* mem_out,
		::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_in,
		unsigned int size,
		SizeType gridSize,
		SizeType offset)
{
	unsigned int init_offset = offset[0] + offset[1] * gridSize[0] + offset[2] * gridSize[0] * gridSize[1];
	constexpr unsigned short mem_data_bytes = MEM_DATA_WIDTH / 8;
	constexpr unsigned short data_bytes = DATA_WIDTH /8;
	unsigned int num_whole_axi_blocks = size / mem_data_bytes;
	unsigned int whole_axi_size = num_whole_axi_blocks * mem_data_bytes;
	unsigned int partial_offset = whole_axi_size / data_bytes;
	unsigned int partial_axi_size = size - whole_axi_size;

	axis2mem<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>((ap_uint<MEM_DATA_WIDTH>*)(mem_out + init_offset), strm_in, whole_axi_size);
	axis2memMasked<DATA_WIDTH, AXIS_DATA_WIDTH>((mem_out + init_offset + partial_offset), strm_in, partial_axi_size);
}

template <unsigned int MEM_DATA_WIDTH, unsigned int AXIS_DATA_WIDTH, unsigned int DATA_WIDTH=32>
void memWriteGrid(ap_uint<DATA_WIDTH>* mem_out,
		::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_in,
		SizeType gridSize,
		AccessRange& range)
{
//	unsigned int init_offset = offset[0] + offset[1] * gridSize[0] + offset[2] * gridSize[0] * gridSize[1];
//	constexpr unsigned short mem_data_bytes = MEM_DATA_WIDTH / 8;
//	constexpr unsigned short data_bytes = DATA_WIDTH /8;
//	unsigned int num_whole_axi_reads = size / mem_data_bytes;
//	unsigned int whole_axi_size = num_whole_axi_reads * mem_data_bytes;
//	unsigned int partial_offset = whole_axi_size / data_bytes;
//	unsigned int partial_axi_size = size - whole_axi_size;
	constexpr unsigned short vectorFactor = MEM_DATA_WIDTH / DATA_WIDTH;
	unsigned short ShiftBits = LOG2(vectorFactor);
	unsigned short DataShiftBits = LOG2(DATA_WIDTH/8);
	unsigned short xtile_size = (range.end[0] - range.start[0]);

	unsigned short xtile_alignment_point = ((range.start[0] >> ShiftBits) + 1) * vectorFactor;//This point where alligned widen read start
	unsigned short front_part_xtile_size = xtile_alignment_point < range.end[0] ? xtile_alignment_point - range.start[0] : 0;
	unsigned short num_whole_xblocks = (xtile_size - front_part_xtile_size)  >> ShiftBits;
	unsigned short whole_xtile_size = num_whole_xblocks << ShiftBits;
	unsigned short back_part_xtile_size = xtile_size - whole_xtile_size - front_part_xtile_size;

	if (whole_xtile_size ==  0)
	{
		front_part_xtile_size += back_part_xtile_size;
		back_part_xtile_size = 0;
	}

//	unsigned short xtile_size_bytes = xtile_size << DataShiftBits;
	unsigned short front_part_xtile_bytes = front_part_xtile_size << DataShiftBits;
	unsigned short whole_xtile_bytes = whole_xtile_size << DataShiftBits;
	unsigned short back_part_xtile_bytes = back_part_xtile_size << DataShiftBits;


	if (range.dim < 3)
	{
		range.start[2] = 0;
		range.end[2] = 1;
	}
	else if (range.dim < 2)
	{
		range.start[1] = 0;
		range.end[1] = 1;
	}

	unsigned short init_offset = range.start[0];
	unsigned short k_coef = gridSize[1] * gridSize[0];
	unsigned short j_coef = gridSize[0];
#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| starting memWriteGrid. grid_size: (%d, %d, %d), range: (%d, %d, %d) --> (%d, %d, %d)\n "
			"front partial xtile size:%d, whole xtile size:%d, back partial xtile size:%d, xtile_alignment_point:%d\n"
			, __func__, gridSize[0], gridSize[1], gridSize[2], range.start[0], range.start[1], range.start[2],
			range.end[0], range.end[1], range.end[2], front_part_xtile_size, whole_xtile_size, back_part_xtile_size, xtile_alignment_point);
	printf("===========================================================================================\n");
#endif
	for (unsigned short k = range.start[2]; k < range.end[2]; k++)
	{
		for (unsigned short j = range.start[1]; j < range.end[1]; j++)
		{
#pragma HLS PIPELINE II=1
			unsigned short offset = init_offset + k * k_coef + j * j_coef;
			unsigned short whole_offset = front_part_xtile_size + offset;
			unsigned short back_part_offset = whole_offset + whole_xtile_size;
#ifdef DEBUG_LOG_PRINT
			printf("|HLS DEBUG_LOG|%s| writing. front_offset:%d, whole_offset:%d, back_part_offset:%d, j:%d, k:%d\n"
					, __func__, offset, whole_offset, back_part_offset, j, k);
#endif
//			if (front_part_xtile_size)
				axis2memMasked<DATA_WIDTH, AXIS_DATA_WIDTH>(mem_out + offset, strm_in, front_part_xtile_bytes);
//			if (whole_xtile_size)
				axis2mem<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>((ap_uint<MEM_DATA_WIDTH>*)(mem_out + whole_offset), strm_in, whole_xtile_bytes);
//			if (back_part_xtile_size)
				axis2memMasked<DATA_WIDTH, AXIS_DATA_WIDTH>(mem_out + back_part_offset, strm_in, back_part_xtile_bytes);

		}
	}
#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| exiting.\n"
			, __func__);
#endif
}


/**
 * @brief memWriteGridSimple writes to memory with range with MEM_DATA_WIDTH alignment
 *
 * @details Writes to a grid with ranges with range.start[0] is aligned with MEM_DATA_WIDTH.
 * Other components of range will be supported in granual level.
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port
 * @tparam AXIS_DATA_WIDTH : Data width of the AXIS-stream port
 * @tparam DATA_WIDTH : Data width of the base datatype
 *
 * @param mem_out : AXI4-master memory output port
 * @param strm_in : AXIS-stream input port
 * @param gridSize : gridSize of the original grid
 * @param range : Access range to be writen data
 */

template <unsigned int MEM_DATA_WIDTH, unsigned int AXIS_DATA_WIDTH, unsigned int DATA_WIDTH=32>
void memWriteGridSimple(ap_uint<DATA_WIDTH>* mem_out,
		::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_in,
		SizeType gridSize,
		AccessRange& range)
{

	constexpr unsigned short vectorFactor = MEM_DATA_WIDTH / DATA_WIDTH;
	unsigned short ShiftBits = LOG2(vectorFactor);
	unsigned short DataShiftBits = LOG2(DATA_WIDTH/8);
	unsigned short xtile_size = (range.end[0] - range.start[0]);
	unsigned short num_whole_xblocks = xtile_size >> ShiftBits;
	unsigned short whole_xtile_size = num_whole_xblocks << ShiftBits;
	unsigned short whole_xtile_size_bytes = whole_xtile_size << DataShiftBits;
	unsigned short partial_xtile_size = xtile_size - whole_xtile_size;
	unsigned short partial_xtile_size_bytes = partial_xtile_size << DataShiftBits;
	unsigned short xtile_size_bytes = xtile_size << DataShiftBits;

	if (range.dim < 3)
	{
		range.start[2] = 0;
		range.end[2] = 1;
	}
	else if (range.dim < 2)
	{
		range.start[1] = 0;
		range.end[1] = 1;
	}

#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| starting memWriteGrid. grid_size: (%d, %d, %d), range: (%d, %d, %d) --> (%d, %d, %d)\n whole_xtile_size:%d, partial_xtile_size:%d\n"
			, __func__, gridSize[0], gridSize[1], gridSize[2], range.start[0], range.start[1], range.start[2],
			range.end[0], range.end[1], range.end[2], whole_xtile_size, partial_xtile_size);
	printf("====================================================================================\n");
#endif
	for (unsigned short k = range.start[2]; k < range.end[2]; k++)
	{
		for (unsigned short j = range.start[1]; j < range.end[1]; j++)
		{
#pragma HLS PIPELINE II = 1
			unsigned short offset = register_it(k * gridSize[1]) + j;
			offset = offset * gridSize[0];
			offset = range.start[0] + offset;
#ifdef DEBUG_LOG_PRINT
			printf("|HLS DEBUG_LOG|%s| writing. offset:%d, j:%d, k:%d\n"
					, __func__, offset, j, k);
#endif
			axis2mem<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>((ap_uint<MEM_DATA_WIDTH>*)(mem_out + offset), strm_in, whole_xtile_size_bytes);
			axis2memMasked<DATA_WIDTH, AXIS_DATA_WIDTH>(mem_out + offset + whole_xtile_size, strm_in, partial_xtile_size_bytes);
		}
	}
#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| exiting.\n"
			, __func__);
#endif
}

/**
 * @brief memWriteGridSimpleV2 writes to memory with range with MEM_DATA_WIDTH alignment with detecting continous range for
 * optimized burst write to memory
 *
 * @details Writes to a grid with ranges with range.start[0] is aligned with MEM_DATA_WIDTH.
 * Other components of range will be supported in granual level.
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port
 * @tparam AXIS_DATA_WIDTH : Data width of the AXIS-stream port
 * @tparam DATA_WIDTH : Data width of the base datatype
 *
 * @param mem_out : AXI4-master memory output port
 * @param strm_in : AXIS-stream input port
 * @param gridSize : gridSize of the original grid
 * @param range : Access range to be writen data
 */

template <unsigned int MEM_DATA_WIDTH, unsigned int AXIS_DATA_WIDTH, unsigned int DATA_WIDTH=32>
void memWriteGridSimpleV2(ap_uint<MEM_DATA_WIDTH>* mem_out,
		::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_in,
		SizeType gridSize,
		AccessRange& range)
{

	constexpr unsigned short data_vector_factor = MEM_DATA_WIDTH / DATA_WIDTH;
	constexpr int II_factor = MEM_DATA_WIDTH / AXIS_DATA_WIDTH;
	unsigned short ShiftBits = (unsigned short)LOG2(data_vector_factor);
	unsigned short DataShiftBits = (unsigned short)LOG2(DATA_WIDTH/8);
	unsigned short start_x = range.start[0] >> ShiftBits;
	unsigned short end_x = (range.end[0] + data_vector_factor - 1) >> ShiftBits;
	unsigned short grid_xblocks = gridSize[0] >> ShiftBits; //GridSize[0] has to be MEM_DATA_WIDTH aligned
	unsigned short num_xblocks = end_x - start_x;
	unsigned short x_tile_size = num_xblocks << ShiftBits;
	unsigned int x_tile_size_bytes = x_tile_size << DataShiftBits;

	if (range.dim < 3)
	{
		range.start[2] = 0;
		range.end[2] = 1;
	}
	else if (range.dim < 2)
	{
		range.start[1] = 0;
		range.end[1] = 1;
	}

#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| starting memWriteGrid. shiftbits: %d, data shift bits: %d, grid_size: (%d, %d, %d), grid_xblocks: %d, range: (%d, %d, %d) --> (%d, %d, %d)\n "
			"start_x_block: %d --> end_x_block: %d, x_tile_size:%d, x_tile_size_bytes:%d, num_xblocks:%d\n"
			, __func__, ShiftBits, DataShiftBits, gridSize[0], gridSize[1], gridSize[2], grid_xblocks, range.start[0], range.start[1], range.start[2],
			range.end[0], range.end[1], range.end[2], start_x, end_x, x_tile_size, x_tile_size_bytes, num_xblocks);
	printf("====================================================================================\n");
#endif

	unsigned short diff_y = range.end[1] - range.start[1];
	bool isContinous = (grid_xblocks == num_xblocks and (diff_y == gridSize[1] or not range.dim == 3)) or range.dim==1;

	if (isContinous)
	{
#ifdef DEBUG_LOG_PRINT
		printf("|HLS DEBUG_LOG|%s| continuous read\n", __func__);
#endif
		unsigned int offset = start_x + range.start[1] * grid_xblocks + range.start[2] * gridSize[1] * grid_xblocks;
		unsigned int size_y = range.end[1] - range.start[1];
		unsigned int size_z = range.end[2] - range.start[2];
		unsigned int  size_bytes = x_tile_size_bytes * size_y * size_z;

#ifdef DEBUG_LOG_PRINT
		printf("|HLS DEBUG_LOG|%s| init offset:%d, size_y:%d, size_z:%d, size_bytes:%d\n", __func__, offset, size_y, size_z, size_bytes);
#endif
		axis2memV2<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>((ap_uint<MEM_DATA_WIDTH>*)(mem_out + offset), strm_in, size_bytes);
	}
	else
	{
		for (unsigned short k = range.start[2]; k < range.end[2]; k++)
		{
			for (unsigned short j = range.start[1]; j < range.end[1]; j++)
			{
	// #pragma HLS PIPELINE II = II_factor
				unsigned int offset = start_x + j * grid_xblocks + k * gridSize[1] * grid_xblocks;
	#ifdef DEBUG_LOG_PRINT
				printf("|HLS DEBUG_LOG|%s| writing. offset:%d, j:%d, k:%d\n"
						, __func__, offset, j, k);
	#endif
				axis2memV2<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>((ap_uint<MEM_DATA_WIDTH>*)(mem_out + offset), strm_in, x_tile_size_bytes);
			}
		}
	}
#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| exiting.\n"
			, __func__);
#endif
}

/**
 * @brief memWriteGridTerminate writes to memory with range to terminate read only output stream
 *
 * @details terminate output axis stream which do not need to be written to a memory.
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port
 * @tparam AXIS_DATA_WIDTH : Data width of the AXIS-stream port
 * @tparam DATA_WIDTH : Data width of the base datatype
 *
 * @param strm_in : AXIS-stream input port
 * @param gridSize : gridSize of the original grid
 * @param range : Access range to be writen data
 */
template <unsigned int MEM_DATA_WIDTH, unsigned int AXIS_DATA_WIDTH, unsigned int DATA_WIDTH=32>
void memWriteGridTerminate(::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_in,
		SizeType gridSize,
		AccessRange& range)
{
	constexpr unsigned short data_vector_factor = MEM_DATA_WIDTH / DATA_WIDTH;
	constexpr unsigned int num_strm_pkts_per_beat = MEM_DATA_WIDTH / AXIS_DATA_WIDTH;
//	constexpr int II_factor = MEM_DATA_WIDTH / AXIS_DATA_WIDTH;
	unsigned short ShiftBits = (unsigned short)LOG2(data_vector_factor);
//	unsigned short DataShiftBits = (unsigned short)LOG2(DATA_WIDTH/8);
	unsigned short start_x = range.start[0] >> ShiftBits;
	unsigned short end_x = (range.end[0] + data_vector_factor - 1) >> ShiftBits;
	unsigned short grid_xblocks = gridSize[0] >> ShiftBits; //GridSize[0] has to be MEM_DATA_WIDTH aligned
	unsigned short num_xblocks = end_x - start_x;
	unsigned short num_xpkts = num_xblocks * num_strm_pkts_per_beat;
//	unsigned short x_tile_size = num_xblocks << ShiftBits;
//	unsigned int x_tile_size_bytes = x_tile_size << DataShiftBits;

	if (range.dim < 3)
	{
		range.start[2] = 0;
		range.end[2] = 1;
	}
	if (range.dim < 2)
	{
		range.start[1] = 0;
		range.end[1] = 1;
	}

	unsigned int num_pkts = num_xpkts * (range.end[1] - range.start[1]) * (range.end[2] - range.start[2]);

#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| Terminate memGridWrite with total pkts %d. num of x_pkts:%d, range dim: %d, range:(%d,%d,%d) -> (%d,%d,%d) \n"
			, __func__, num_pkts, num_xpkts, range.dim, range.start[0], range.start[1], range.start[2], range.end[0], range.end[1], range.end[2]);
#endif

	axisTerminate<AXIS_DATA_WIDTH>(strm_in, num_pkts);

#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| exiting.\n"
			, __func__);
#endif
}

//constexpr unsigned short log2(constexpr unsigned short n)
//{
//  return ( (n<2) ? 1 : 1+log2(n/2));
//}

template <unsigned int MEM_DATA_WIDTH, unsigned int AXIS_DATA_WIDTH, unsigned int DATA_WIDTH=32>
void memReadGrid_test(ap_uint<MEM_DATA_WIDTH>* mem_in,
		::hls::stream<ap_axiu<AXIS_DATA_WIDTH,0,0,0>>& strm_out, MemConfig& config)
{
// 	constexpr unsigned short data_vector_factor = MEM_DATA_WIDTH / DATA_WIDTH;
// 	constexpr unsigned short ShiftBits = log2(data_vector_factor);
// 	unsigned short DataShiftBits = (unsigned short)LOG2(DATA_WIDTH/8);
// 	unsigned short start_x = range.start[0] >> ShiftBits;
// 	unsigned short end_x = (range.end[0] + data_vector_factor - 1) >> ShiftBits;
// 	unsigned short grid_xblocks = gridSize[0] >> ShiftBits; //GridSize[0] has to be MEM_DATA_WIDTH aligned
// 	unsigned short num_xblocks = end_x - start_x;
// 	unsigned short x_tile_size = num_xblocks << ShiftBits;
// 	unsigned int x_tile_size_bytes = x_tile_size << DataShiftBits;

// 	if (range.dim < 3)
// 	{
// 		range.start[2] = 0;
// 		range.end[2] = 1;
// 	}
// 	if (range.dim < 2)
// 	{
// 		range.start[1] = 0;
// 		range.end[1] = 1;
// 	}

// #ifdef DEBUG_LOG_PRINT
// 	printf("|HLS DEBUG_LOG|%s| starting memReadGrid. shiftbits: %d, data shift bits: %d, grid_size: (%d, %d, %d), grid_xblocks: %d, range: (%d, %d, %d) --> (%d, %d, %d)\n "
// 			"start_x_block: %d --> end_x_block: %d, x_tile_size:%d, x_tile_size_bytes:%d, num_xblocks:%d\n"
// 			, __func__, ShiftBits, DataShiftBits, gridSize[0], gridSize[1], gridSize[2], grid_xblocks, range.start[0], range.start[1], range.start[2],
// 			range.end[0], range.end[1], range.end[2], start_x, end_x, x_tile_size, x_tile_size_bytes, num_xblocks);
// 	printf("===========================================================================================\n");
// #endif
// 	unsigned short diff_y = range.end[1] - range.start[1];
// 	bool isContinous = (grid_xblocks == num_xblocks and (diff_y == gridSize[1] or not range.dim == 3));

	if (config.isContinous)
	{
//#ifdef DEBUG_LOG_PRINT
		printf("|HLS DEBUG_LOG|%s| continuous read\n", __func__);
//#endif
// 		unsigned int offset = start_x + range.start[1] * grid_xblocks + range.start[2] * gridSize[1] * grid_xblocks;
// 		unsigned int size_y = range.end[1] - range.start[1];
// 		unsigned int size_z = range.end[2] - range.start[2];
// 		unsigned int  size_bytes = x_tile_size_bytes * size_y * size_z;

#ifdef DEBUG_LOG_PRINT
		printf("|HLS DEBUG_LOG|%s| init offset:%d, size_bytes:%d\n", __func__, config.start_offset, config.total_size_bytes);
#endif
		mem2axis<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>((ap_uint<MEM_DATA_WIDTH>* )(mem_in + config.start_offset), strm_out, config.total_size_bytes);
	}
	else
	{
		for (unsigned short k = config.start_z; k < config.end_z; k++)
		{
			for (unsigned short j = config.start_y; j < config.end_y; j++)
			{
				unsigned int offset = config.start_x + j * config.grid_xblocks + k * config.grid_size_y * config.grid_xblocks;
    #ifdef DEBUG_LOG_PRINT
				printf("|HLS DEBUG_LOG|%s| reading. offset:%d, j:%d, k:%d\n"
						, __func__,offset, j, k);
    #endif
				mem2axis<MEM_DATA_WIDTH, AXIS_DATA_WIDTH>((ap_uint<MEM_DATA_WIDTH>* )(mem_in + offset), strm_out, config.x_tile_bytes);
			}
		}
	}
#ifdef DEBUG_LOG_PRINT
	printf("|HLS DEBUG_LOG|%s| exiting.\n"
			, __func__);
#endif
}

}
}

#endif /* DOXYGEN_SHOULD_SKIP_THIS */
