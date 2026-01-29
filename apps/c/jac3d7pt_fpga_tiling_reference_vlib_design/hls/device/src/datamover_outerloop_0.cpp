// Auto-generated at 2026-01-27 16:58:13.707085 by ops-translator
#include <datamover_outerloop_0.hpp>

template <unsigned short BURSTLEN>
void configParser(::hls::stream<ap_uint<160>>& command,
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
        ap_uint<64> s3 = z * stride_z;
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
            }
        }
    }
    e_r.write(true);
}

template <int MEM_DATA_WIDTH, int LATENCY, int OUTSTANDING, int BURSTLEN>
void read3DTiled(
    // input
    ::hls::burst_maxi<ap_uint<MEM_DATA_WIDTH> >& data,
    ::hls::stream<ap_uint<160>>& s_command,
    // ouput
    ::hls::stream<ap_axiu<MEM_DATA_WIDTH, 0, 0, 0> >& s_data) {
#pragma HLS DATAFLOW
    hls::stream<ap_uint<64> > r_offset("r_offset");
#pragma HLS stream variable = r_offset depth = OUTSTANDING
    hls::stream<ap_uint<10> > r_burst("r_burst");
#pragma HLS stream variable = r_burst depth = OUTSTANDING
    hls::stream<bool> e_r("e_r");
#pragma HLS stream variable = e_r depth = OUTSTANDING

    configParser<BURSTLEN>(s_command, r_offset, r_burst, e_r);
    xf::data_mover::details::manualBurstRead<MEM_DATA_WIDTH, LATENCY, OUTSTANDING, BURSTLEN>(data, r_offset, r_burst, e_r, s_data);
}

template<int MEM_DATA_WIDTH, int LATENCY, int OUTSTANDING, int BURSTLEN>
void write3DTiled(
    // input
    ::hls::stream<ap_axiu<MEM_DATA_WIDTH, 0, 0, 0>>& s_data,
    ::hls::stream<ap_uint<160>>& s_command,
    // output
    ::hls::burst_maxi<ap_uint<MEM_DATA_WIDTH>>& data
)
{
#pragma HLS DATAFLOW
    hls::stream<ap_uint<64> > r_offset("r_offset");
#pragma HLS stream variable = r_offset depth = OUTSTANDING
    hls::stream<ap_uint<10> > r_burst("r_burst");
#pragma HLS stream variable = r_burst depth = OUTSTANDING
    hls::stream<bool> e_r("e_r");
#pragma HLS stream variable = e_r depth = OUTSTANDING
    configParser<BURSTLEN>(s_command, r_offset, r_burst, e_r);
    xf::data_mover::details::manualBurstWrite<MEM_DATA_WIDTH, LATENCY, OUTSTANDING, BURSTLEN>(r_offset, r_burst, e_r, s_data, data);
}

void commandGen3D(const ap_uint<64> offset, const ap_uint<16> stride_x, const ap_uint<16> size_x,
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
#ifdef DEBUG_LOG
    printf("|HLS DEBUG_LOG|%s| offset:%llu, stride_x:%u, size_x:%u, stride_y:%u, size_y:%u\n", __func__, (unsigned long long)offset, (unsigned int)stride_x, (unsigned int)size_x, (unsigned int)stride_y, (unsigned int)size_y);
#endif
}

template <unsigned short MEM_DATA_WIDTH, unsigned short LATENCY, unsigned short BURSTLEN=32, unsigned short OUTSTANDING_READ=32, unsigned short IN_ITR=2>
static void stridedTileMem2streamV2(::hls::burst_maxi<ap_uint<MEM_DATA_WIDTH> >& mem_in, hls::stream<ap_axiu<MEM_DATA_WIDTH, 0, 0, 0> >& strm_out, const ops::hls::MemConfigTile& config)
{
// #pragma HLS INLINE off
    #ifdef DEBUG_LOG
        printf("|HLS DEBUG_LOG|%s| reading tile. tile_start:%d, tile_size:%d\n", __func__, config.start_offset, config.total_size_bytes);
    #endif

    const unsigned short z_diff = config.end_z - config.start_z;
    // const unsigned short tile_size_y_mul_z_diff = config.tile_size_y * z_diff;
    // const unsigned short last_tile_size_y_mul_z_diff = config.last_tile_size_y * z_diff;
    const unsigned short stride_y = config.grid_xblocks;
    const unsigned short stride_z = config.grid_xblocks * config.grid_size_y;
    // const unsigned short stride_x = 1;

    ::hls::stream<ap_uint<160>> command("command");
    #pragma HLS stream variable = command depth = OUTSTANDING_READ

    for (unsigned short tile_y = 0; tile_y < config.tile_count_y; tile_y++)
    {
        const unsigned short tile_size_y = tile_y == (config.tile_count_y -1) ? config.last_tile_size_y : config.tile_size_y;
        // const unsigned short realized_tile_size_y_mul_z_diff = tile_y == (config.tile_count_y -1) ? last_tile_size_y_mul_z_diff : tile_size_y_mul_z_diff;
        const ap_uint<64> tile_y_offset = tile_y * config.effective_tile_size_y * config.grid_xblocks;
        // const unsigned int abs_row_id_y_offset = config.tile_count_x * tile_y * tile_size_y_mul_z_diff;

        for (unsigned short tile_x = 0; tile_x < config.tile_count_x; tile_x++)
        {
#ifdef DEBUG_LOG
            printf("|HLS DEBUG_LOG|%s| Tile details tile_x:%d, tile_y:%d\n", __func__, tile_x, tile_y);
#endif
            // const unsigned int abs_row_id_x_offset = tile_x * realized_tile_size_y_mul_z_diff;
            const unsigned short tile_size_x = tile_x == (config.tile_count_x -1) ? config.last_tile_size_x : config.tile_size_x;
            const ap_uint<64> tile_x_offset = tile_x * config.effective_tile_size_x;

            const ap_uint<64> offset = config.start_offset + tile_x_offset + tile_y_offset;

    //         ap_uint<128> command;
    // //         ap_uint<64> offset = cmd_pkt.range(63,0);
    // // ap_uint<16> stride_x = cmd_pkt.range(79,64);
    // // ap_uint<16> size_x = cmd_pkt.range(95,80);
    // // ap_uint<16> stride_y = cmd_pkt.range(111,96);
    // // ap_uint<16> size_y = cmd_pkt.range(127,112);
    //         command.range(63,0) = offset;
    //         command.range(79,64) = 1;
    //         command.range(95,80) = (ap_uint<16>)tile_size_x;
    //         command.range(111,96) = (ap_uint<16>)stride_y;
    //         command.range(127,112) = (ap_uint<16>)tile_size_y;


            commandGen3D(offset, 1, (ap_uint<16>)tile_size_x, (ap_uint<16>)stride_y, (ap_uint<16>)tile_size_y, (ap_uint<16>)stride_z, (ap_uint<16>)z_diff, command);
            read3DTiled<MEM_DATA_WIDTH, LATENCY, OUTSTANDING_READ, BURSTLEN>(mem_in, command, strm_out);
        }
    }
}

template <unsigned short MEM_DATA_WIDTH, unsigned short LATENCY, unsigned short BURSTLEN=32, unsigned short OUTSTANDING_READ=32, unsigned short IN_ITR=2>
static void stridedTileStream2memV2(::hls::burst_maxi<ap_uint<MEM_DATA_WIDTH> >& mem_out, hls::stream<ap_axiu<MEM_DATA_WIDTH, 0, 0, 0> >& strm_in, const ops::hls::MemConfigTile& config)
{
#ifdef DEBUG_LOG
    printf("|HLS DEBUG_LOG|%s| writing tile. tile_start:%d, tile_size:%d\n", __func__, config.start_offset, config.total_size_bytes);
#endif

    const unsigned short z_diff = config.end_z - config.start_z;
    // const unsigned short tile_size_y_mul_z_diff = config.tile_size_y * z_diff;
    // const unsigned short last_tile_size_y_mul_z_diff = config.last_tile_size_y * z_diff;
    const unsigned short stride_y = config.grid_xblocks;
    const unsigned short stride_z = config.grid_xblocks * config.grid_size_y;
    // const unsigned short stride_x = 1;

    ::hls::stream<ap_uint<160>> command("command");
    #pragma HLS stream variable = command depth = OUTSTANDING_READ

    for (unsigned short tile_y = 0; tile_y < config.tile_count_y; tile_y++)
    {
        const unsigned short tile_size_y = tile_y == (config.tile_count_y -1) ? config.last_tile_size_y : config.tile_size_y;
        // const unsigned short realized_tile_size_y_mul_z_diff = tile_y == (config.tile_count_y -1) ? last_tile_size_y_mul_z_diff : tile_size_y_mul_z_diff;
        const ap_uint<64> tile_y_offset = tile_y * config.effective_tile_size_y * config.grid_xblocks;
        // const unsigned int abs_row_id_y_offset = config.tile_count_x * tile_y * tile_size_y_mul_z_diff;

        for (unsigned short tile_x = 0; tile_x < config.tile_count_x; tile_x++)
        {
            // const unsigned int abs_row_id_x_offset = tile_x * realized_tile_size_y_mul_z_diff;
            const unsigned short tile_size_x = tile_x == (config.tile_count_x -1) ? config.last_tile_size_x : config.tile_size_x;
            const ap_uint<64> tile_x_offset = tile_x * config.effective_tile_size_x;

            const ap_uint<64> offset = config.start_offset + tile_x_offset + tile_y_offset;

    //         ap_uint<128> command;
    // //         ap_uint<64> offset = cmd_pkt.range(63,0);
    // // ap_uint<16> stride_x = cmd_pkt.range(79,64);
    // // ap_uint<16> size_x = cmd_pkt.range(95,80);
    // // ap_uint<16> stride_y = cmd_pkt.range(111,96);
    // // ap_uint<16> size_y = cmd_pkt.range(127,112);
    //         command.range(63,0) = offset;
    //         command.range(79,64) = 1;
    //         command.range(95,80) = (ap_uint<16>)tile_size_x;
    //         command.range(111,96) = (ap_uint<16>)stride_y;
    //         command.range(127,112) = (ap_uint<16>)tile_size_y;


            commandGen3D(offset, 1, (ap_uint<16>)tile_size_x, (ap_uint<16>)stride_y, (ap_uint<16>)tile_size_y, (ap_uint<16>)stride_z, (ap_uint<16>)z_diff, command);
            write3DTiled<MEM_DATA_WIDTH, LATENCY, OUTSTANDING_READ, BURSTLEN>(strm_in, command, mem_out);
        }
    }
}

static void datamover_outerloop_0_dataflow_region_read(
        const unsigned int num_pkts,
        const ops::hls::MemConfig& memconfig,
        ap_uint<mem_data_width>* arg0,
        hls::stream<ap_axiu<axis_data_width, 0, 0, 0>>& arg0_axis_out)
{
#pragma HLS DATAFLOW
    static ::hls::stream<ap_uint<mem_data_width>> arg0_read_mem_strm;
    #pragma HLS STREAM variable = arg0_read_mem_strm
    static ::hls::stream<ap_uint<axis_data_width>> arg0_read_reduced_mem_strm;
    #pragma HLS STREAM variable = arg0_read_reduced_mem_strm
    ops::hls::mem2stream<mem_data_width, 32, 2>(arg0, arg0_read_mem_strm, memconfig.total_xblocks);
    
    ops::hls::stream2streamStepdown<mem_data_width, axis_data_width>(arg0_read_mem_strm, arg0_read_reduced_mem_strm, memconfig.total_xblocks);
    ops::hls::stream2axis<axis_data_width>(arg0_read_reduced_mem_strm, arg0_axis_out, num_pkts);
}

static void datamover_outerloop_0_dataflow_region_write(
        const unsigned int num_pkts,
        const ops::hls::MemConfig& memconfig,
        ap_uint<mem_data_width>* arg1,
        hls::stream<ap_axiu<axis_data_width, 0, 0, 0>>& arg1_axis_in)
{
    static ::hls::stream<ap_uint<mem_data_width>> arg1_write_mem_strm;
    #pragma HLS STREAM variable = arg1_write_mem_strm
    static ::hls::stream<ap_uint<axis_data_width>> arg1_write_reduced_mem_strm;
    #pragma HLS STREAM variable = arg1_write_reduced_mem_strm

#pragma HLS DATAFLOW
    ops::hls::axis2stream<axis_data_width>(arg1_axis_in, arg1_write_reduced_mem_strm, num_pkts);
    
    ops::hls::stream2streamStepup<axis_data_width, mem_data_width>(arg1_write_reduced_mem_strm, arg1_write_mem_strm, memconfig.total_xblocks);
    ops::hls::stream2mem<mem_data_width, 32, 2>(arg1, arg1_write_mem_strm, memconfig.total_xblocks);
}

static void datamover_outerloop_0_dataflow_read_write_dataflow_region(
        const unsigned int num_pkts,
        const ops::hls::MemConfigTile& memconfig,
		::hls::burst_maxi<ap_uint<mem_data_width> >& arg0,
		::hls::burst_maxi<ap_uint<mem_data_width> >& arg1,
		hls::stream <ap_axiu<mem_data_width,0,0,0>>& arg0_axis_out,
		hls::stream <ap_axiu<mem_data_width,0,0,0>>& arg1_axis_in
)    
{
//    static ::hls::stream<ap_uint<mem_data_width>> arg0_read_mem_strm;
//    #pragma HLS STREAM variable = arg0_read_mem_strm
//    static ::hls::stream<ap_uint<axis_data_width>> arg0_read_reduced_mem_strm;
//    #pragma HLS STREAM variable = arg0_read_reduced_mem_strm
//    static ::hls::stream<ap_uint<axis_data_width>> arg1_write_reduced_mem_strm;
//    #pragma HLS STREAM variable = arg1_write_reduced_mem_strm
//    static ::hls::stream<ap_uint<mem_data_width>> arg1_write_mem_strm;
//    #pragma HLS STREAM variable = arg1_write_mem_strm

#pragma HLS DATAFLOW
        //ops::hls::mem2streamTiled<mem_data_width, 64, 2>(arg0_b1, arg0_b2, arg0_read_mem_strm, memconfig);
        //ops::hls::stridedTileMem2stream<mem_data_width,  64, 2>(arg0_b1, arg0_read_mem_strm_b1, memconfig, 0);
        //ops::hls::stridedTileMem2stream<mem_data_width,  64, 2>(arg0_b2, arg0_read_mem_strm_b2, memconfig, 1);
        //ops::hls::combineSteams<mem_data_width, 2>(arg0_read_mem_strm_b1, arg0_read_mem_strm_b2, arg0_read_mem_strm, memconfig);
//        ops::hls::tileMem2stream<mem_data_width, 64, 2>(arg0, arg0_read_mem_strm, memconfig);
//        ops::hls::stream2streamStepdown<mem_data_width, axis_data_width>(arg0_read_mem_strm, arg0_read_reduced_mem_strm, memconfig.total_xblocks);
//        ops::hls::stream2axis<axis_data_width>(arg0_read_reduced_mem_strm, arg0_axis_out, num_pkts);
//        ops::hls::axis2stream<axis_data_width>(arg1_axis_in, arg1_write_reduced_mem_strm, num_pkts);
//
//        ops::hls::stream2streamStepup<axis_data_width, mem_data_width>(arg1_write_reduced_mem_strm, arg1_write_mem_strm, memconfig.total_xblocks);
        // ops::hls::stream2memTiled<mem_data_width, 64, 2>(arg1_b1, arg1_b2, arg1_write_mem_strm, memconfig);
        // ops::hls::splitStream<mem_data_width, 2>(arg1_write_mem_strm, arg1_write_mem_strm_b1, arg1_write_mem_strm_b2, memconfig);
        // ops::hls::stridedTileStream2mem<mem_data_width, 64, 2>(arg1_write_mem_strm_b1, arg1_b1, memconfig, 0);
        // ops::hls::stridedTileStream2mem<mem_data_width, 64, 2>(arg1_write_mem_strm_b2, arg1_b2, memconfig, 1);
//           ops::hls::tileStream2mem<mem_data_width, 64, 2>(arg1, arg1_write_mem_strm, memconfig);
    stridedTileMem2streamV2<mem_data_width, 32, 40, 32, 2>(arg1, arg0_axis_out, memconfig);
    stridedTileStream2memV2<mem_data_width, 32, 40, 32, 2>(arg1, arg1_axis_in, memconfig);
}

static void datamover_outerloop_0_dataflow_read_write(
		const unsigned int iter,
		const unsigned int num_pkts,
		const ops::hls::MemConfigTile& memconfig,
		::hls::burst_maxi<ap_uint<mem_data_width> >& arg0,
		::hls::burst_maxi<ap_uint<mem_data_width> >& arg1,
		hls::stream <ap_axiu<mem_data_width,0,0,0>>& arg0_axis_out,
		hls::stream <ap_axiu<mem_data_width,0,0,0>>& arg1_axis_in
)    
{
    for (unsigned int i = 0; i < iter/2; i++)
    {
    //#pragma HLS PIPELINE REWIND
    #ifdef DEBUG_LOG
        printf("[KERNEL_DEBUG]|%s| Calling datamover. i:%d\n", __func__, i);
    #endif
        datamover_outerloop_0_dataflow_read_write_dataflow_region(
                num_pkts,
                memconfig,
                arg0,
                arg1,
                arg0_axis_out,
                arg1_axis_in
        );
        datamover_outerloop_0_dataflow_read_write_dataflow_region(
                num_pkts,
                memconfig,
                arg1,
                arg0,
                arg0_axis_out,
                arg1_axis_in
);
    }
}

static void datamover_outerloop_0_loopback_dataflow_region(
        const unsigned int num_pkts
,
        hls::stream<ap_axiu<axis_data_width, 0, 0, 0>>& arg0_axis_out,
        hls::stream<ap_axiu<axis_data_width, 0, 0, 0>>& arg1_axis_in)
{
    static ::hls::stream<ap_uint<axis_data_width>> arg0_mem_strm;
    #pragma HLS STREAM variable = arg0_mem_strm
#pragma HLS DATAFLOW
    ops::hls::axis2stream<axis_data_width>(arg1_axis_in, arg0_mem_strm, num_pkts);
    ops::hls::stream2axis<axis_data_width>(arg0_mem_strm, arg0_axis_out, num_pkts);
}

static void datamover_outerloop_0_loopback(
        const unsigned int iter,
        const unsigned int num_pkts
,
        hls::stream<ap_axiu<axis_data_width, 0, 0, 0>>& arg0_axis_out,
        hls::stream<ap_axiu<axis_data_width, 0, 0, 0>>& arg1_axis_in)
{
    for (unsigned int i = 0; i < iter; i++)
    {
     //   pragma HLS PIPELINE REWIND
    #ifdef DEBUG_LOG
        printf("[KERNEL_DEBUG]|%s| Calling loopback. i:%d\n", __func__, i);
    #endif
        datamover_outerloop_0_loopback_dataflow_region(num_pkts
,
        arg0_axis_out,
        arg1_axis_in);
    } 
}
extern "C" void datamover_outerloop_0(
        const unsigned short range_start_0,
        const unsigned short range_end_0,
        const unsigned short range_start_1,
        const unsigned short range_end_1,
        const unsigned short range_start_2,
        const unsigned short range_end_2,
        const unsigned short gridSize_0,
        const unsigned short gridSize_1,
        const unsigned short gridSize_2,
        const unsigned int outer_itr,
        const unsigned short tile_size_x,
        const unsigned short tile_size_y,
        const unsigned short overlap_size_x,
        const unsigned short overlap_size_y,
        const unsigned short effective_tile_size_x,
        const unsigned short effective_tile_size_y,
        const unsigned short last_tile_size_x,
        const unsigned short last_tile_size_y,
        const unsigned short tile_count_x,
        const unsigned short tile_count_y,
        const unsigned int total_xblocks,
	//u-b1
		::hls::burst_maxi<ap_uint<mem_data_width> > arg0,
	//u2-b1
		::hls::burst_maxi<ap_uint<mem_data_width> > arg1,
    //u
        hls::stream <ap_axiu<mem_data_width,0,0,0>>& arg0_axis_out,
    //u2
        hls::stream <ap_axiu<mem_data_width,0,0,0>>& arg1_axis_in
    )

{
    #pragma HLS INTERFACE s_axilite port = range_start_0 bundle = control
    #pragma HLS INTERFACE s_axilite port = range_end_0 bundle = control
    #pragma HLS INTERFACE s_axilite port = range_start_1 bundle = control
    #pragma HLS INTERFACE s_axilite port = range_end_1 bundle = control
    #pragma HLS INTERFACE s_axilite port = range_start_2 bundle = control
    #pragma HLS INTERFACE s_axilite port = range_end_2 bundle = control
 
    #pragma HLS INTERFACE s_axilite port = gridSize_0 bundle = control
    #pragma HLS INTERFACE s_axilite port = gridSize_1 bundle = control
    #pragma HLS INTERFACE s_axilite port = gridSize_2 bundle = control
    #pragma HLS INTERFACE s_axilite port = outer_itr bundle = control
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
    #pragma HLS INTERFACE s_axilite port = total_xblocks bundle = control
 
    #pragma HLS INTERFACE mode=m_axi bundle=gmem0 depth=4096 max_read_burst_length=64 max_write_burst_length=64 \
            num_read_outstanding=4 num_write_outstanding=4 \
            port=arg0 offset=slave
    #pragma HLS INTERFACE s_axilite port = arg0 bundle = control
 
    #pragma HLS INTERFACE mode=m_axi bundle=gmem1 depth=4096 max_read_burst_length=64 max_write_burst_length=64 \
            num_read_outstanding=4 num_write_outstanding=4 \
            port=arg1 offset=slave
    #pragma HLS INTERFACE s_axilite port = arg1 bundle = control
    #pragma HLS INTERFACE mode=axis port=arg0_axis_out register
    #pragma HLS INTERFACE mode=axis port=arg1_axis_in register

    #pragma HLS INTERFACE mode=s_axilite port=return bundle = control
    #pragma HLS INTERFACE mode=ap_ctrl_chain port=return

    ops::hls::AccessRange range;
    range.start[0] = range_start_0;
    range.end[0] = range_end_0;
    range.start[1] = range_start_1;
    range.end[1] = range_end_1;
    range.start[2] = range_start_2;
    range.end[2] = range_end_2;
    range.dim = 3;

    ops::hls::SizeType read_gridSize = { gridSize_0, gridSize_1, gridSize_2 };
    ops::hls::SizeType2d tile_size = {tile_size_x, tile_size_y};
    ops::hls::SizeType2d overlap_size = {overlap_size_x, overlap_size_y};
    ops::hls::SizeType2d effective_tile_size = {effective_tile_size_x, effective_tile_size_y};
    ops::hls::SizeType2d last_tile_size = {last_tile_size_x, last_tile_size_y};
    ops::hls::SizeType2d tile_count = {tile_count_x, tile_count_y};

#ifdef DEBUG_LOG
    printf("[KERNEL_DEBUG]|%s| starting datamover TOP range:(%d,%d,%d) ---> (%d,%d,%d)\n", __func__,
            range.start[0], range.start[1], range.start[2], range.end[0], range.end[1], range.end[2]);
    printf("[KERNEL_DEBUG]|%s| read_gridSize: (%d, %d, %d), \n", __func__,
            read_gridSize[0], read_gridSize[1], read_gridSize[2]);
#endif 

    ops::hls::MemConfigTile config;

    ops::hls::genMemConfigTileV2<mem_data_width, data_width>(read_gridSize, range, tile_size, tile_count, overlap_size, effective_tile_size, last_tile_size, total_xblocks, config);   
    const unsigned int num_beats = config.total_xblocks;
    const unsigned int num_pkts = num_of_pkts_per_beat * num_beats;

#ifdef DEBUG_LOG
    printf("[KERNEL_DEBUG]|%s| REALIZED numbers: num_beats: %d, num_pkts: %d,\n", __func__,
         num_beats, num_pkts);
#endif 
        datamover_outerloop_0_dataflow_read_write(
                outer_itr,
                num_pkts,
                config,
                arg0,
                arg1,
            arg0_axis_out,
            arg1_axis_in
            );
}



// extern "C" void stream_step_down(
//     const unsigned int total_xblocks,
//     const unsigned int num_pkts,
//     const unsigned int outer_itr,
//      ::hls::stream <ap_axiu<mem_data_width,0,0,0>>& strm_in,
//      ::hls::stream <ap_axiu<axis_data_width,0,0,0>>& strm_out
// ) {
//     #pragma HLS INTERFACE mode=s_axilite port=total_xblocks bundle = control
//     #pragma HLS INTERFACE mode=s_axilite port=num_pkts bundle = control
//     #pragma HLS INTERFACE mode=s_axilite port=outer_itr bundle = control
//     #pragma HLS INTERFACE mode=axis port=strm_in register
//     #pragma HLS INTERFACE mode=axis port=strm_out register

//     #pragma HLS INTERFACE mode=s_axilite port=return bundle = control
//     #pragma HLS INTERFACE mode=ap_ctrl_chain port=return

//     ::hls::stream<ap_uint<mem_data_width>> read_mem_strm;
//     #pragma HLS STREAM variable = read_mem_strm depth = 32
//     ::hls::stream<ap_uint<axis_data_width>> read_reduced_mem_strm;
//     #pragma HLS STREAM variable = read_reduced_mem_strm depth = 64
// #ifdef DEBUG_LOG
//     printf("[KERNEL_DEBUG]|%s| starting", __func__);
// #endif
//     for (unsigned int itr = 0; itr < outer_itr; itr++) {
//         #pragma HLS DATAFLOW
//         ops::hls::axis2stream<mem_data_width>(strm_in, read_mem_strm, total_xblocks);
//         ops::hls::stream2streamStepdown<mem_data_width, axis_data_width>(read_mem_strm, read_reduced_mem_strm,total_xblocks);
//         ops::hls::stream2axis<axis_data_width>(read_reduced_mem_strm, strm_out, num_pkts);
//     }
// }

// extern "C" void stream_step_up(
//     const unsigned int total_xblocks,
//     const unsigned int num_pkts,
//     const unsigned int outer_itr,
//      ::hls::stream <ap_axiu<axis_data_width,0,0,0>>& strm_in,
//      ::hls::stream <ap_axiu<mem_data_width,0,0,0>>& strm_out
// ) {
//     #pragma HLS INTERFACE mode=s_axilite port=total_xblocks bundle = control
//     #pragma HLS INTERFACE mode=s_axilite port=num_pkts bundle = control
//     #pragma HLS INTERFACE mode=s_axilite port=outer_itr bundle = control
//     #pragma HLS INTERFACE mode=axis port=strm_out register
//     #pragma HLS INTERFACE mode=axis port=strm_in register

//     #pragma HLS INTERFACE mode=s_axilite port=return bundle = control
//     #pragma HLS INTERFACE mode=ap_ctrl_chain port=return

//     ::hls::stream<ap_uint<axis_data_width>> write_reduced_mem_strm;
//     #pragma HLS STREAM variable = write_reduced_mem_strm depth = 64
//     ::hls::stream<ap_uint<mem_data_width>> write_mem_strm;
//     #pragma HLS STREAM variable = write_mem_strm depth = 32

// #ifdef DEBUG_LOG
//     printf("[KERNEL_DEBUG]|%s| starting", __func__);
// #endif
//     for (unsigned int itr = 0; itr < outer_itr; itr++) {
//         #pragma HLS DATAFLOW
//         ops::hls::axis2stream<axis_data_width>(strm_in, write_reduced_mem_strm, num_pkts);
//         ops::hls::stream2streamStepup<axis_data_width, mem_data_width>(write_reduced_mem_strm, write_mem_strm, total_xblocks);
//         ops::hls::stream2axis<mem_data_width>(write_mem_strm, strm_out, total_xblocks);
//     }
// }





