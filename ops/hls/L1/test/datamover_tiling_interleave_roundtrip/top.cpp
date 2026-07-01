
#include "top.hpp"

//template <unsigned int MEM_DATA_WIDTH, unsigned int STREAM_DATA_WIDTH, unsigned int BURST_SIZE>
void dut(hls::stream<ap_uint<MEM_DATA_WIDTH>> in_stream[NUM_STREAMS],
		hls::stream<ap_uint<MEM_DATA_WIDTH>> out_stream[NUM_STREAMS])
{
#pragma HLS TOP

	constexpr unsigned short REALISED_OVERLAP = DATA_WIDTH * OVERLAP_SIZE;
	constexpr unsigned short MEM_DATA_WIDTH_OUT = MEM_DATA_WIDTH + REALISED_OVERLAP;

	hls::stream<ap_uint<MEM_DATA_WIDTH_OUT>> mid_stream[NUM_STREAMS];

	ops::hls::stream2interleave<MEM_DATA_WIDTH, DATA_WIDTH, NUM_STREAMS, OVERLAP_SIZE>(in_stream, mid_stream, NUM_PKTS);
	ops::hls::interleave2stream<MEM_DATA_WIDTH, DATA_WIDTH, NUM_STREAMS, OVERLAP_SIZE>(mid_stream, out_stream, NUM_PKTS);
}
