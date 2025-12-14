
#include "top.hpp"
#include "PE_jac3d_kernel_stencil.hpp"
// #define DEBUG_LOG

template <unsigned short MEM_DATA_WIDTH, unsigned short IN_ITR=2, unsigned short BURST_SIZE=32>
static void stridedTileMem2stream(ap_uint<MEM_DATA_WIDTH>* mem_in, hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_out, const ops::hls::MemConfigTile& config, unsigned short stride_start = 0)
{
    // #pragma HLS INLINE off
    #ifdef DEBUG_LOG
        printf("|HLS DEBUG_LOG|%s| reading tile. tile_start:%d, tile_size:%d, stride_start:%d\n", __func__, config.start_offset, config.total_size_bytes, stride_start);
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

                for (unsigned short j = stride_start; j < tile_size_y; j+=2)
                {
                    #pragma HLS PIPELINE
                    const unsigned short tile_size_x = tile_x == (config.tile_count_x -1) ? config.last_tile_size_x : config.tile_size_x;
                    
                    unsigned int offset_1 = config.start_offset + tile_x_offset;
                    unsigned int offset_2 = k_offset + tile_y_offset;
                    unsigned int offset_3 = offset_1 + offset_2;
                    unsigned int j_offset  = j * config.grid_xblocks;
                    unsigned int offset = offset_3 + j_offset;

                    #ifdef DEBUG_LOG
                        printf("|HLS DEBUG_LOG|%s| offset:%u tile_y:%u tile_x:%u k:%u j:%u tile_size_x:%u\n",
                               __func__,
                               (unsigned int)offset,
                               (unsigned int)tile_y,
                               (unsigned int)tile_x,
                               (unsigned int)k,
                               (unsigned int)j,
                               (unsigned int)tile_size_x);
                    #endif
                    ops::hls::mem2stream<MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>((ap_uint<MEM_DATA_WIDTH>* )(mem_in + offset), strm_out, tile_size_x);

                }
            }
        }
    }
}

/**
 * combine_steams
 *
 * Merge/interleave two input HLS streams (two memory banks) into a single output stream
 * according to a linearized row parity within a tiled 3D layout.
 *
 * Template parameters:
 *   MEM_DATA_WIDTH  - bit-width of each element carried on the input/output streams (ap_uint<...>).
 *   IN_ITR          - pipeline initiation-interval hint used by the inner loop (controls II pragma).
 *
 * Parameters:
 *   strm_in_b1      - input stream for bank 1.
 *   strm_in_b2      - input stream for bank 2.
 *   strm_out        - output stream receiving elements selected from bank 1 or bank 2.
 *   config          - tile layout and tiling metadata (start_z, end_z, tile sizes, last tile sizes,
 *                     tile counts, etc.). Used to compute absolute (linearized) row indices.
 *
 * High-level behavior:
 *   - Iterates over tiles in Y and X, over the local Z range (start_z .. end_z-1), and over
 *     intra-tile Y and X positions.
 *   - For each element position, computes an absolute linearized row index (abs_x_row_id).
 *   - If abs_x_row_id is even, reads one element from strm_in_b1 and writes it to strm_out.
 *     If odd, reads one element from strm_in_b2 and writes it to strm_out.
 *   - The read/write pairs are performed inside a pipelined inner loop (pragma uses IN_ITR).
 *
 * Absolute-index computation (conceptual):
 *   abs_x_row_id = j                              // local Y within tile
 *                + k * realized_tile_y_z_span     // local Z scaled by per-x-tile y*z span
 *                + tile_x * realized_tile_y_z_span
 *                + tile_count_x * tile_y * nominal_tile_y_z_span
 *
 *   Where:
 *     - j  : intra-tile Y coordinate
 *     - k  : intra-z index (0..z_diff-1)
 *     - tile_x/tile_y : tile coordinates
 *     - realized_tile_y_z_span :
 *           (tile_size_y or last_tile_size_y for this x tile) * z_diff
 *     - nominal_tile_y_z_span : nominal tile_size_y * z_diff (used when accumulating tile_y blocks)
 *
 * Skewed tiles and the z dimension not being tiled:
 *   - The implementation multiplies tile Y sizes by z_diff (end_z - start_z). This means the z
 *     dimension is treated as an inner expansion of the Y span rather than an independently tiled
 *     axis. When Z is not separately tiled (i.e., the entire z range is iterated inside each
 *     X/Y tile), tile extents in Y are effectively scaled by z_diff and the linearization
 *     (parity) boundary shifts accordingly.
 *   - "Skew" arises when tiles at the X edge use last_tile_size_y (different from the nominal
 *     tile_size_y). Because realized_tile_y_z_span depends on the per-X-tile Y size multiplied by
 *     z_diff, adjacent X tiles can have different Y*z spans. This changes the mapping from
 *     (x,y,z) to abs_x_row_id across tile boundaries and therefore changes which physical bank
 *     (strm_in_b1 or strm_in_b2) supplies a given element near tile edges.
 *
 * Assumptions and requirements:
 *   - The input streams must contain exactly the expected number of elements for the tiled region
 *     described by config; underflow or overflow on reads will block or be undefined in a hardware
 *     context.
 *   - config fields (tile sizes, last tile sizes, tile counts, start_z/end_z) must be consistent
 *     and produce non-zero extents (e.g., end_z > start_z).
 *   - Function relies on parity of the computed linear index to select banks; ensure the intended
 *     bank-mapping semantics match upstream/downstream memory layout.
 *
 * Notes:
 *   - The routine is intended for HLS synthesis; the inner-most loop is pipelined (II driven by
 *     template parameter IN_ITR) to help meet throughput targets.
 *   - The function name contains a typo ("steams" vs "streams"); documentation and callers should
 *     be kept consistent with the implemented identifier or corrected across the codebase.
 */
template <unsigned short MEM_DATA_WIDTH,  unsigned short IN_ITR=2>
static void combineSteams(
    hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in_b1,
    hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in_b2,
    hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_out,
    const ops::hls::MemConfigTile& config)
{
    // #pragma HLS INLINE off
    const unsigned short z_diff = config.end_z - config.start_z;
    const unsigned short tile_size_y_mul_z_diff = config.tile_size_y * z_diff;
    const unsigned short last_tile_size_y_mul_z_diff = config.last_tile_size_y * z_diff;

    for (unsigned short tile_y = 0; tile_y < config.tile_count_y; tile_y++)
    {
        const unsigned short tile_size_y = tile_y == (config.tile_count_y -1) ? config.last_tile_size_y : config.tile_size_y;
        // const unsigned short realized_tile_size_y_mul_z_diff = tile_y == (config.tile_count_y -1) ? last_tile_size_y_mul_z_diff : tile_size_y_mul_z_diff;
        // const unsigned int tile_y_offset = tile_y * config.effective_tile_size_y * config.grid_xblocks;

        for (unsigned short tile_x = 0; tile_x < config.tile_count_x; tile_x++)
        {
            // const unsigned int abs_row_id_x_offset = tile_x * realized_tile_size_y_mul_z_diff;
            // const unsigned int tile_x_offset = tile_x * config.effective_tile_size_x;

            for (unsigned short k = 0; k < z_diff; k++)
            {
                // const unsigned int abs_row_id_y_offset_k = k * realized_tile_size_y_mul_z_diff;

                for (unsigned short j = 0; j < tile_size_y; j++)
                {
                    const unsigned short tile_size_x = tile_x == (config.tile_count_x -1) ? config.last_tile_size_x : config.tile_size_x;
                    
                    // unsigned int abs_x_row_id = j + abs_row_id_y_offset_k + abs_row_id_x_offset + abs_row_id_y_offset;
                    unsigned int local_tile_row_id = j;

                    for (unsigned short i = 0; i < tile_size_x; i++)
                    {
#pragma HLS PIPELINE II=IN_ITR
                        if (local_tile_row_id % 2 == 0)
                        {
                            #ifdef DEBUG_LOG
                                printf("|HLS DEBUG_LOG|%s| reading from bank 1. tile_y:%u tile_x:%u k:%u j:%u i:%u local_tile_row_id:%u\n",
                                       __func__,
                                       (unsigned int)tile_y,
                                       (unsigned int)tile_x,
                                       (unsigned int)k,
                                       (unsigned int)j,
                                       (unsigned int)i,
                                       (unsigned int)local_tile_row_id);
                            #endif
                            ap_uint<MEM_DATA_WIDTH> data_b1 = strm_in_b1.read();
                            #ifdef DEBUG_LOG
                                #ifndef __SYNTHESIS__
                                    printf("   |HLS DEBUG_LOG||%s| read data_b1 val=(", __func__);
                                    ops::hls::DataConv tmp;
                                    for (unsigned n = 0; n < MEM_DATA_WIDTH; n+=32)
                                    {
                                        tmp.i = data_b1.range(n + 32 - 1, n);
                                        printf("%f,", tmp.f);
                                    }
                                    printf(")\n");
                                #endif
                            #endif
                            strm_out.write(data_b1);
                        }
                        else
                        {
                            #ifdef DEBUG_LOG
                                printf("|HLS DEBUG_LOG|%s| reading from bank 2. tile_y:%u tile_x:%u k:%u j:%u i:%u local_tile_row_id:%u\n",
                                        __func__,
                                        (unsigned int)tile_y,
                                        (unsigned int)tile_x,
                                        (unsigned int)k,
                                        (unsigned int)j,
                                        (unsigned int)i,
                                        (unsigned int)local_tile_row_id);
                            #endif
                            ap_uint<MEM_DATA_WIDTH> data_b2 = strm_in_b2.read();
                            #ifdef DEBUG_LOG
                                #ifndef __SYNTHESIS__
                                    printf("   |HLS DEBUG_LOG||%s| read data_b2 val=(", __func__);
                                    ops::hls::DataConv tmp;
                                    for (unsigned n = 0; n < MEM_DATA_WIDTH; n+=32)
                                    {
                                        tmp.i = data_b2.range(n + 32 - 1, n);
                                        printf("%f,", tmp.f);
                                    }
                                    printf(")\n");
                                #endif
                            #endif
                            strm_out.write(data_b2);
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief 	mem2streamTiled reads from a memory location in tiled manner to an hls stream.
 *  		This is optimized to read from AXI4 with burst and to utilize width maximum througput
 *          by utilizing two banks to read from.
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port and the hls stream port
 * @tparam IN_ITR: II of the mem read
 * @tparam BURST_SIZE : Burst length of the AXI4 (max beats < 256)
 * 
 *
 * @param mem_in : input memory port
 * @param stream_out : output hls-stream
 * @param config : MemconfigTile to guide reading
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short IN_ITR=2, unsigned short BURST_SIZE=32>
void mem2streamTiled(ap_uint<MEM_DATA_WIDTH>* mem_in_b1,
                ap_uint<MEM_DATA_WIDTH>* mem_in_b2,
				::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_out,
				const ops::hls::MemConfigTile& config)
{
#ifdef DEBUG_LOG
	printf("|HLS DEBUG_LOG|%s| starting\n", __func__);
#endif
	if (config.isContinous)
	{
#ifdef DEBUG_LOG
	printf("|HLS DEBUG_LOG|%s| continuous read\n", __func__);
#endif

#ifdef DEBUG_LOG
	printf("|HLS DEBUG_LOG|%s| init offset:%d, size_bytes:%d\n", __func__, config.start_offset, config.total_size_bytes);
#endif
	ops::hls::mem2stream<MEM_DATA_WIDTH>((ap_uint<MEM_DATA_WIDTH>* )(mem_in_b1 + config.start_offset), strm_out, config.total_xblocks);
	}
	else
	{
        static hls::stream<ap_uint<MEM_DATA_WIDTH>> strm_in_b1;
        #pragma HLS STREAM variable = strm_in_b1
        static hls::stream<ap_uint<MEM_DATA_WIDTH>> strm_in_b2;
        #pragma HLS STREAM variable = strm_in_b2
        #pragma HLS DATAFLOW
        stridedTileMem2stream<MEM_DATA_WIDTH, IN_ITR, BURST_SIZE>(mem_in_b1, strm_in_b1, config, 0);
        stridedTileMem2stream<MEM_DATA_WIDTH, IN_ITR, BURST_SIZE>(mem_in_b2, strm_in_b2, config, 1);
        combineSteams<MEM_DATA_WIDTH, IN_ITR>(strm_in_b1, strm_in_b2, strm_out, config);
	}
#ifdef DEBUG_LOG
	printf("|HLS DEBUG_LOG|%s| exiting.\n"
			, __func__);
#endif
}

/**
 * @brief 	splitStream splits a single input stream into two output streams
 *  		according to a linearized row parity within a tiled 3D layout.
 *
 * @tparam MEM_DATA_WIDTH : Data width of the hls stream
 * @tparam IN_ITR: II of the stream operations
 *
 * @param strm_in : input stream
 * @param strm_out_b1 : output stream for bank 1
 * @param strm_out_b2 : output stream for bank 2
 * @param config : MemConfigTile to guide splitting
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short IN_ITR=2>
static void splitStream(
    hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in,
    hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_out_b1,
    hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_out_b2,
    const ops::hls::MemConfigTile& config)
{
    // #pragma HLS INLINE off
    const unsigned short z_diff = config.end_z - config.start_z;
    const unsigned short tile_size_y_mul_z_diff = config.tile_size_y * z_diff;
    const unsigned short last_tile_size_y_mul_z_diff = config.last_tile_size_y * z_diff;

    for (unsigned short tile_y = 0; tile_y < config.tile_count_y; tile_y++)
    {
        const unsigned short tile_size_y = tile_y == (config.tile_count_y -1) ? config.last_tile_size_y : config.tile_size_y;
        // const unsigned short realized_tile_size_y_mul_z_diff = tile_y == (config.tile_count_y -1) ? last_tile_size_y_mul_z_diff : tile_size_y_mul_z_diff;

        for (unsigned short tile_x = 0; tile_x < config.tile_count_x; tile_x++)
        {
            // const unsigned int abs_row_id_x_offset = tile_x * realized_tile_size_y_mul_z_diff;

            for (unsigned short k = 0; k < z_diff; k++)
            {
                // const unsigned int abs_row_id_y_offset_k = k * realized_tile_size_y_mul_z_diff;

                for (unsigned short j = 0; j < tile_size_y; j++)
                {
                    const unsigned short tile_size_x = tile_x == (config.tile_count_x -1) ? config.last_tile_size_x : config.tile_size_x;
                    
                    unsigned int local_tile_row_id = j;// + abs_row_id_y_offset_k;
                    for (unsigned short i = 0; i < tile_size_x; i++)
                    {
#pragma HLS PIPELINE II=IN_ITR
                        ap_uint<MEM_DATA_WIDTH> data = strm_in.read();
                        #ifdef DEBUG_LOG
                            #ifndef __SYNTHESIS__
                                printf("   |HLS DEBUG_LOG||%s| read data val=(", __func__);
                                ops::hls::DataConv tmp;
                                for (unsigned n = 0; n < MEM_DATA_WIDTH; n+=32)
                                {
                                    tmp.i = data.range(n + 32 - 1, n);
                                    printf("%f,", tmp.f);
                                }
                                printf(")\n");
                            #endif
                        #endif
                        if (local_tile_row_id % 2 == 0)
                        {
                            #ifdef DEBUG_LOG
                                printf("|HLS DEBUG_LOG|%s| writing to bank 1. tile_y:%u tile_x:%u k:%u j:%u i:%u local_tile_row_id:%u\n",
                                       __func__,
                                       (unsigned int)tile_y,
                                       (unsigned int)tile_x,
                                       (unsigned int)k,
                                       (unsigned int)j,
                                       (unsigned int)i,
                                       (unsigned int)local_tile_row_id);
                                printf("|HLS DEBUG_LOG|%s| writing data to bank 1\n", __func__);
                            #endif
                            strm_out_b1.write(data);
                        }
                        else
                        {
                            #ifdef DEBUG_LOG
                                printf("|HLS DEBUG_LOG|%s| writing to bank 2. tile_y:%u tile_x:%u k:%u j:%u i:%u local_tile_row_id:%u\n",
                                       __func__,
                                       (unsigned int)tile_y,
                                       (unsigned int)tile_x,
                                       (unsigned int)k,
                                       (unsigned int)j,
                                       (unsigned int)i,
                                       (unsigned int)local_tile_row_id);
                                printf("|HLS DEBUG_LOG|%s| writing data to bank 2\n", __func__);
                            #endif
                            strm_out_b2.write(data);
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief 	stridedTileStream2mem writes from an hls stream to memory in tiled manner with stride.
 *  		This is optimized to write to AXI4 with burst and to utilize maximum throughput
 *          by writing to two banks.
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port and the hls stream port
 * @tparam IN_ITR: II of the mem write
 * @tparam BURST_SIZE : Burst length of the AXI4 (max beats < 256)
 *
 * @param strm_in : input hls-stream
 * @param mem_out : output memory port
 * @param config : MemConfigTile to guide writing
 * @param stride_start : stride offset for starting write position (0 or 1)
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short IN_ITR=2, unsigned short BURST_SIZE=32>
static void stridedTileStream2mem(hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in, ap_uint<MEM_DATA_WIDTH>* mem_out, const ops::hls::MemConfigTile& config, unsigned short stride_start = 0)
{
    // #pragma HLS INLINE off
    #ifdef DEBUG_LOG
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

        for (unsigned short tile_x = 0; tile_x < config.tile_count_x; tile_x++)
        {
            const unsigned int abs_row_id_x_offset = tile_x * realized_tile_size_y_mul_z_diff;
            const unsigned int tile_x_offset = tile_x * config.effective_tile_size_x; 

            for (unsigned short k = 0; k < z_diff; k++)
            {
                const unsigned int k_offset = k * config.grid_xblocks * config.grid_size_y;

                for (unsigned short j = stride_start; j < tile_size_y; j+=2)
                {
                    #pragma HLS PIPELINE II=IN_ITR
                    const unsigned short tile_size_x = tile_x == (config.tile_count_x -1) ? config.last_tile_size_x : config.tile_size_x;
                    
                    unsigned int offset_1 = config.start_offset + tile_x_offset;
                    unsigned int offset_2 = k_offset + tile_y_offset;
                    unsigned int offset_3 = offset_1 + offset_2;
                    unsigned int j_offset  = j * config.grid_xblocks;
                    unsigned int offset = offset_3 + j_offset;
                    #ifdef DEBUG_LOG
                        printf("|HLS DEBUG_LOG|%s| offset:%u tile_y:%u tile_x:%u k:%u j:%u tile_size_x:%u\n",
                               __func__,
                               (unsigned int)offset,
                               (unsigned int)tile_y,
                               (unsigned int)tile_x,
                               (unsigned int)k,
                               (unsigned int)j,
                               (unsigned int)tile_size_x);
                    #endif
                    ops::hls::stream2mem<MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>((ap_uint<MEM_DATA_WIDTH>* )(mem_out + offset), strm_in, tile_size_x);
                }
            }
        }
    }
}

/**
 * @brief 	stream2memTiled writes from a stream to memory in tiled manner.
 *  		This is optimized to write to AXI4 with burst and to utilize maximum throughput
 *          by utilizing two banks to write to.
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port and the hls stream port
 * @tparam IN_ITR: II of the mem write
 * @tparam BURST_SIZE : Burst length of the AXI4 (max beats < 256)
 *
 * @param stream_in : input hls-stream
 * @param mem_out_b1 : output memory port for bank 1
 * @param mem_out_b2 : output memory port for bank 2
 * @param config : MemConfigTile to guide writing
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short IN_ITR=2, unsigned short BURST_SIZE=32>
void stream2memTiled(::hls::stream<ap_uint<MEM_DATA_WIDTH>>& strm_in,
                ap_uint<MEM_DATA_WIDTH>* mem_out_b1,
                ap_uint<MEM_DATA_WIDTH>* mem_out_b2,
				const ops::hls::MemConfigTile& config)
{
#ifdef DEBUG_LOG
	printf("|HLS DEBUG_LOG|%s| starting\n", __func__);
#endif
	if (config.isContinous)
	{
#ifdef DEBUG_LOG
		printf("|HLS DEBUG_LOG|%s| continuous write\n", __func__);
#endif

#ifdef DEBUG_LOG
		printf("|HLS DEBUG_LOG|%s| init offset:%d, size_bytes:%d\n", __func__, config.start_offset, config.total_size_bytes);
#endif
		ops::hls::stream2mem<MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>((ap_uint<MEM_DATA_WIDTH>* )(mem_out_b1 + config.start_offset), strm_in, config.total_xblocks);
	}
	else
	{
        static hls::stream<ap_uint<MEM_DATA_WIDTH>> strm_out_b1;
        #pragma HLS STREAM variable = strm_out_b1
        static hls::stream<ap_uint<MEM_DATA_WIDTH>> strm_out_b2;
        #pragma HLS STREAM variable = strm_out_b2
        #pragma HLS DATAFLOW
        splitStream<MEM_DATA_WIDTH, IN_ITR>(strm_in, strm_out_b1, strm_out_b2, config);
        stridedTileStream2mem<MEM_DATA_WIDTH, IN_ITR, BURST_SIZE>(strm_out_b1, mem_out_b1,  config, 0);
        stridedTileStream2mem<MEM_DATA_WIDTH, IN_ITR, BURST_SIZE>(strm_out_b2, mem_out_b2,  config, 1);
        // ops::hls::stream2mem<MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>(mem_out_b1, strm_out_b1, config.total_xblocks);
        // ops::hls::stream2mem<MEM_DATA_WIDTH, BURST_SIZE, IN_ITR>(mem_out_b2, strm_out_b2, config.total_xblocks);
	}
#ifdef DEBUG_LOG
	printf("|HLS DEBUG_LOG|%s| exiting.\n"
			, __func__);
#endif
}

static void joint_PE_outerloop_0(const unsigned short PEId, const ops::hls::StencilConfigCoreTiled& stencilConfig, 
        ::hls::stream<ap_uint<axis_data_width>>& arg0_hls_stream_in, 
        ::hls::stream<ap_uint<axis_data_width>>& arg1_hls_stream_out
)
{
    // ::hls::stream<ap_uint<axis_data_width>> node2_1_to_node3_0;
    // #pragma HLS STREAM variable = node2_1_to_node3_0 depth = 10

    kernel_jac3D_kernel_stencil_PE(PEId, stencilConfig,
            arg0_hls_stream_in,
            arg1_hls_stream_out
    );
}

static void kernel_outerloop_0_dataflow_region_cascaded(const ops::hls::StencilConfigCoreTiled& stencilConfig,
    ::hls::stream<ap_uint<axis_data_width>> arg0_arg1_streams[iter_par_factor + 1]
)
{
#pragma HLS INLINE 

    for (int i = 0; i < iter_par_factor; i++)
    {
#pragma HLS UNROLL factor=iter_par_factor
        joint_PE_outerloop_0(i, stencilConfig,
                arg0_arg1_streams[i],
                arg0_arg1_streams[i+1]
        );
    }
}

/**
 * @brief 	readWriteTiledDataflow orchestrates the dataflow for tiled read and write operations.
 *
 * @tparam MEM_DATA_WIDTH : Data width of the AXI4 port and the hls stream port
 * @tparam IN_ITR: II of the mem operations
 * @tparam BURST_SIZE : Burst length of the AXI4 (max beats < 256)
 *
 * @param mem_in_b1 : input memory bank 1
 * @param mem_in_b2 : input memory bank 2
 * @param mem_out_b1 : output memory bank 1
 * @param mem_out_b2 : output memory bank 2
 * @param config : MemConfigTile to guide operations
 */
template <unsigned short MEM_DATA_WIDTH, unsigned short IN_ITR=2, unsigned short BURST_SIZE=32>
static void readWriteTiledDataflow(
    ap_uint<MEM_DATA_WIDTH>* mem_in_b1,
    ap_uint<MEM_DATA_WIDTH>* mem_in_b2,
    ap_uint<MEM_DATA_WIDTH>* mem_out_b1,
    ap_uint<MEM_DATA_WIDTH>* mem_out_b2,
    const ops::hls::MemConfigTile& config,
    const ops::hls::StencilConfigCoreTiled& stencilConfig)
{
    #pragma HLS DATAFLOW

    // Internal stream connecting read and write paths
    static hls::stream<ap_uint<MEM_DATA_WIDTH>> hls_streams[iter_par_factor + 1];
    #pragma HLS STREAM variable = hls_streams depth = 10

    // Read from memory banks into stream
    mem2streamTiled<MEM_DATA_WIDTH, IN_ITR, BURST_SIZE>(mem_in_b1, mem_in_b2, hls_streams[0], config);

    kernel_outerloop_0_dataflow_region_cascaded(stencilConfig, hls_streams);
    // Write from stream to memory banks
    stream2memTiled<MEM_DATA_WIDTH, IN_ITR, BURST_SIZE>(hls_streams[iter_par_factor], mem_out_b1, mem_out_b2, config);
}

void dut(ap_uint<AXI_M_WIDTH>* mem_in_b1,
        ap_uint<AXI_M_WIDTH>* mem_in_b2,
		ap_uint<AXI_M_WIDTH>* mem_out_b1,
        ap_uint<AXI_M_WIDTH>* mem_out_b2,
        const unsigned short dim,
        const unsigned short grid_x_size,
        const unsigned short grid_y_size,
        const unsigned short grid_z_size,
        const unsigned short range_start_0,
        const unsigned short range_end_0,
        const unsigned short range_start_1,
        const unsigned short range_end_1,
        const unsigned short range_start_2,
        const unsigned short range_end_2,
        const unsigned short tile_size_x,
        const unsigned short tile_size_y,
        const unsigned short overlap_size_x,
        const unsigned short overlap_size_y,
        const unsigned short effective_tile_size_x,
        const unsigned short effective_tile_size_y,
        const unsigned short last_tile_size_x,
        const unsigned short last_tile_size_y,
        const unsigned short tile_count_x,
        const unsigned short tile_count_y)
{
#pragma HLS TOP
    #pragma HLS INTERFACE mode=m_axi bundle=gmem0 depth=8192 max_read_burst_length=64 max_write_burst_length=64 \
            num_read_outstanding=8 num_write_outstanding=8 \
            port=mem_in_b1 offset=slave
    #pragma HLS INTERFACE s_axilite port = mem_in_b1 bundle = control

    #pragma HLS INTERFACE mode=m_axi bundle=gmem1 depth=8192 max_read_burst_length=64 max_write_burst_length=64 \
            num_read_outstanding=8 num_write_outstanding=8 \
            port=mem_in_b2 offset=slave
    #pragma HLS INTERFACE s_axilite port = mem_in_b2 bundle = control

    #pragma HLS INTERFACE mode=m_axi bundle=gmem2 depth=8192 max_read_burst_length=64 max_write_burst_length=64 \
            num_read_outstanding=8 num_write_outstanding=8 \
            port=mem_out_b1 offset=slave
    #pragma HLS INTERFACE s_axilite port = mem_out_b1 bundle = control
 
    #pragma HLS INTERFACE mode=m_axi bundle=gmem3 depth=8192 max_read_burst_length=64 max_write_burst_length=64 \
            num_read_outstanding=8 num_write_outstanding=8 \
            port=mem_out_b2 offset=slave
    #pragma HLS INTERFACE s_axilite port = mem_out_b2 bundle = control

    #pragma HLS INTERFACE s_axilite port = dim bundle = control
    #pragma HLS INTERFACE s_axilite port = grid_x_size bundle = control
    #pragma HLS INTERFACE s_axilite port = grid_y_size bundle = control
    #pragma HLS INTERFACE s_axilite port = grid_z_size bundle = control
    #pragma HLS INTERFACE s_axilite port = range_start_0 bundle = control
    #pragma HLS INTERFACE s_axilite port = range_end_0 bundle = control
    #pragma HLS INTERFACE s_axilite port = range_start_1 bundle = control
    #pragma HLS INTERFACE s_axilite port = range_end_1 bundle = control
    #pragma HLS INTERFACE s_axilite port = range_start_2 bundle = control
    #pragma HLS INTERFACE s_axilite port = range_end_2 bundle = control
    #pragma HLS INTERFACE s_axilite port = tile_size_x bundle = control
    #pragma HLS INTERFACE s_axilite port = tile_size_y bundle = control
    #pragma HLS INTERFACE s_axilite port = overlap_size_x bundle = control
    #pragma HLS INTERFACE s_axilite port = overlap_size_y bundle = control
    #pragma HLS INTERFACE s_axilite port = effective_tile_size_x bundle = control
    #pragma HLS INTERFACE s_axilite port = effective_tile_size_y bundle = control
    #pragma HLS INTERFACE s_axilite port = last_tile_size_x bundle = control
    #pragma HLS INTERFACE s_axilite port = last_tile_size_y bundle = control
    #pragma HLS INTERFACE s_axilite port = tile_count_x bundle = control
    #pragma HLS INTERFACE s_axilite port = tile_count_y bundle = control
    #pragma HLS INTERFACE s_axilite port = return bundle = control

    // printf("|HLS DEBUG_LOG|%s| started.\n", __func__);
    ops::hls::SizeType gridSize = {grid_x_size, grid_y_size, grid_z_size};
    ops::hls::AccessRange range = {{range_start_0, range_start_1, range_start_2},
                                   {range_end_0, range_end_1, range_end_2},
                                   dim};
    ops::hls::SizeType2d tileSize = {tile_size_x, tile_size_y};
    ops::hls::SizeType2d overlapSize = {overlap_size_x, overlap_size_y};
    ops::hls::SizeType2d effectiveTileSize = {effective_tile_size_x, effective_tile_size_y};
    ops::hls::SizeType2d lastTileSize = {last_tile_size_x, last_tile_size_y};
    ops::hls::SizeType2d tileCount = {tile_count_x, tile_count_y};
    ops::hls::MemConfigTile memconfig;
    ops::hls::StencilConfigCoreTiled stencilConfig;
    // printf("|HLS DEBUG_LOG|%s| generating memconfig and stencilConfig\n", __func__);
    ops::hls::genMemConfigTileV2<AXI_M_WIDTH, 32>(gridSize, range, tileSize, tileCount, overlapSize, effectiveTileSize, lastTileSize, memconfig);   
    // SizeType grid_size; //{xblocks, y, z, ...}
    // unsigned short dim;
    // unsigned short tiling_dim; //number of tiled dimensions
    // unsigned short outer_loop_limit;
    // SizeType2d tile_size; //{xblocks, y}
    // SizeType2d last_tile_size; //{xblocks, y}
    // // SizeType2d tile_overlap_size; //{xblocks, y}
    // // SizeType2d effective_tile_size; //{xblocks, y}
    // SizeType2d tile_count; //{xblocks, y}   
    // printf("|HLS DEBUG_LOG|%s| generated memconfig and stencilConfig\n", __func__);
    stencilConfig.grid_size[0] = grid_x_size;
        stencilConfig.grid_size[1] = grid_y_size;
        stencilConfig.grid_size[2] = grid_z_size;
        stencilConfig.dim = dim;
        stencilConfig.tiling_dim = 2; //fixed to 2D tiling
        stencilConfig.outer_loop_limit = grid_z_size + 1; //fixed to outer loop on z
        stencilConfig.tile_size[0] = tile_size_x;
        stencilConfig.tile_size[1] = tile_size_y;
        stencilConfig.last_tile_size[0] = last_tile_size_x;
        stencilConfig.last_tile_size[1] = last_tile_size_y;
        stencilConfig.tile_count[0] = tile_count_x;
        stencilConfig.tile_count[1] = tile_count_y;
    #ifdef DEBUG_LOG
        // print generated memconfig
        printf("|HLS memconfig| start_offset:%u total_size_bytes:%u total_xblocks:%u isContinous:%d\n",
            (unsigned int)memconfig.start_offset,
            (unsigned int)memconfig.total_size_bytes,
            (unsigned int)memconfig.total_xblocks,
            (int)memconfig.isContinous);
        printf("|HLS memconfig| tile_count_x:%u tile_size_x:%u last_tile_size_x:%u effective_tile_size_x:%u\n",
            (unsigned int)memconfig.tile_count_x,
            (unsigned int)memconfig.tile_size_x,
            (unsigned int)memconfig.last_tile_size_x,
            (unsigned int)memconfig.effective_tile_size_x);
        printf("|HLS memconfig| tile_count_y:%u tile_size_y:%u last_tile_size_y:%u effective_tile_size_y:%u grid_xblocks:%u grid_size_y:%u\n",
            (unsigned int)memconfig.tile_count_y,
            (unsigned int)memconfig.tile_size_y,
            (unsigned int)memconfig.last_tile_size_y,
            (unsigned int)memconfig.effective_tile_size_y,
            (unsigned int)memconfig.grid_xblocks,
            (unsigned int)memconfig.grid_size_y);
        printf("|HLS memconfig| start_z:%u end_z:%u z_diff:%u\n",
            (unsigned int)memconfig.start_z,
            (unsigned int)memconfig.end_z,
            (unsigned int)(memconfig.end_z - memconfig.start_z));
        // print generated stencilConfig
        printf("|HLS stencilConfig| grid_size:%u %u %u dim:%u tiling_dim:%u outer_loop_limit:%u\n",
            (unsigned int)stencilConfig.grid_size[0],
            (unsigned int)stencilConfig.grid_size[1],
            (unsigned int)stencilConfig.grid_size[2],
            (unsigned int)stencilConfig.dim,
            (unsigned int)stencilConfig.tiling_dim,
            (unsigned int)stencilConfig.outer_loop_limit);
        printf("|HLS stencilConfig| tile_size:%u %u last_tile_size:%u %u tile_count:%u %u\n",
            (unsigned int)stencilConfig.tile_size[0],
            (unsigned int)stencilConfig.tile_size[1],
            (unsigned int)stencilConfig.last_tile_size[0],
            (unsigned int)stencilConfig.last_tile_size[1],
            (unsigned int)stencilConfig.tile_count[0],
            (unsigned int)stencilConfig.tile_count[1]);
        // Call the dataflow function for tiled read and write operations
    #endif

    readWriteTiledDataflow<AXI_M_WIDTH, 1, 32>(mem_in_b1, mem_in_b2, mem_out_b1, mem_out_b2, memconfig, stencilConfig);
}
