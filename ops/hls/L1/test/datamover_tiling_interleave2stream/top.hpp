#include "../../../include/device/ops_hls_kernel_support.h"

// --------------------------------------------------------
// Configuration Parameters (Overridable via compiler flags)
// --------------------------------------------------------

#ifndef CFG_MEM_DATA_WIDTH
#define CFG_MEM_DATA_WIDTH 256
#endif

#ifndef CFG_DATA_WIDTH
#define CFG_DATA_WIDTH 32
#endif

#ifndef CFG_OVERLAP_SIZE
#define CFG_OVERLAP_SIZE 1
#endif

#ifndef CFG_NUM_STREAMS
#define CFG_NUM_STREAMS 4
#endif

#ifndef CFG_NUM_PKTS
#define CFG_NUM_PKTS 10
#endif

constexpr unsigned short MEM_DATA_WIDTH = CFG_MEM_DATA_WIDTH;
constexpr unsigned short DATA_WIDTH     = CFG_DATA_WIDTH;
constexpr unsigned short OVERLAP_SIZE   = CFG_OVERLAP_SIZE;
constexpr unsigned short NUM_STREAMS    = CFG_NUM_STREAMS;
constexpr unsigned int   NUM_PKTS       = CFG_NUM_PKTS;
constexpr unsigned short REALISED_OVERLAP = DATA_WIDTH * OVERLAP_SIZE;
constexpr unsigned short MEM_DATA_WIDTH_IN = MEM_DATA_WIDTH + REALISED_OVERLAP;

//template <unsigned int MEM_DATA_WIDTH, unsigned int STREAM_DATA_WIDTH, unsigned int BURST_SIZE=32>
void dut(hls::stream<ap_uint<MEM_DATA_WIDTH_IN>> in_stream[NUM_STREAMS],
		hls::stream<ap_uint<MEM_DATA_WIDTH>> out_stream[NUM_STREAMS]);