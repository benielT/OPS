
#include "top.hpp"

//template <unsigned int MEM_DATA_WIDTH, unsigned int STREAM_DATA_WIDTH, unsigned int BURST_SIZE>
void dut(hls::stream<ap_uint<MEM_DATA_WIDTH>> in_stream[NUM_STREAMS],
		hls::stream<ap_uint<MEM_DATA_WIDTH_OUT>> out_stream[NUM_STREAMS])
{
#pragma HLS TOP

	ops::hls::stream2interleave<MEM_DATA_WIDTH, DATA_WIDTH, NUM_STREAMS, OVERLAP_SIZE>(in_stream, out_stream, NUM_PKTS);
}
