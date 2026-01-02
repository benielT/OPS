// Auto-generated at 2025-09-06 21:18:49.925309 by ops-translator
#include <datamover_outerloop_0.hpp>
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
        const ops::hls::MemConfig& memconfig,
        ap_uint<mem_data_width>* arg0,
        ap_uint<mem_data_width>* arg1,
        hls::stream <ap_axiu<axis_data_width,0,0,0>>& arg0_axis_out,
        hls::stream <ap_axiu<axis_data_width,0,0,0>>& arg1_axis_in
)    
{
    static ::hls::stream<ap_uint<mem_data_width>> arg0_read_mem_strm;
    #pragma HLS STREAM variable = arg0_read_mem_strm
    static ::hls::stream<ap_uint<axis_data_width>> arg0_read_reduced_mem_strm;
    #pragma HLS STREAM variable = arg0_read_reduced_mem_strm
    static ::hls::stream<ap_uint<mem_data_width>> arg1_write_mem_strm;
    #pragma HLS STREAM variable = arg1_write_mem_strm
    static ::hls::stream<ap_uint<axis_data_width>> arg1_write_reduced_mem_strm;
    #pragma HLS STREAM variable = arg1_write_reduced_mem_strm

#pragma HLS DATAFLOW
        ops::hls::mem2stream<mem_data_width, 32, 2>(arg0, arg0_read_mem_strm, memconfig.total_xblocks);
        ops::hls::stream2streamStepdown<mem_data_width, axis_data_width>(arg0_read_mem_strm, arg0_read_reduced_mem_strm, memconfig.total_xblocks);
        ops::hls::stream2axis<axis_data_width>(arg0_read_reduced_mem_strm, arg0_axis_out, num_pkts);
        ops::hls::axis2stream<axis_data_width>(arg1_axis_in, arg1_write_reduced_mem_strm, num_pkts);
    
        ops::hls::stream2streamStepup<axis_data_width, mem_data_width>(arg1_write_reduced_mem_strm, arg1_write_mem_strm, memconfig.total_xblocks);
        ops::hls::stream2mem<mem_data_width, 32, 2>(arg1, arg1_write_mem_strm, memconfig.total_xblocks);
}

static void datamover_outerloop_0_dataflow_read_write(
        const unsigned int iter,
        const unsigned int num_pkts,
        const ops::hls::MemConfig& memconfig,
        ap_uint<mem_data_width>* arg0,
        ap_uint<mem_data_width>* arg1,
        hls::stream <ap_axiu<axis_data_width,0,0,0>>& arg0_axis_out,
        hls::stream <ap_axiu<axis_data_width,0,0,0>>& arg1_axis_in
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
        const bool is_loopback,
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
        const unsigned short batch_size,
    //u
        ap_uint<mem_data_width>* arg0,
    //u2
        ap_uint<mem_data_width>* arg1,
    //u
        hls::stream <ap_axiu<axis_data_width,0,0,0>>& arg0_axis_out,
    //u2
        hls::stream <ap_axiu<axis_data_width,0,0,0>>& arg1_axis_in
    )

{
    #pragma HLS INTERFACE s_axilite port = is_loopback bundle = control
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
    #pragma HLS INTERFACE s_axilite port = batch_size bundle = control

 
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
    unsigned int loopback_itr = outer_itr - 1 >= 0 ? outer_itr - 1 : 0;

#ifdef DEBUG_LOG
    printf("[KERNEL_DEBUG]|%s| starting datamover TOP range:(%d,%d,%d) ---> (%d,%d,%d)\n", __func__,
            range.start[0], range.start[1], range.start[2], range.end[0], range.end[1], range.end[2]);
    printf("[KERNEL_DEBUG]|%s| read_gridSize: (%d, %d, %d), \n", __func__,
            read_gridSize[0], read_gridSize[1], read_gridSize[2]);
#endif 

    constexpr unsigned int num_of_pkts_per_bytes = mem_data_width / axis_data_width;
    ops::hls::MemConfig config;
    ops::hls::genMemConfig<mem_data_width, axis_data_width, data_width>(read_gridSize, range, config, batch_size);
    const unsigned int num_beats = config.total_xblocks;
    const unsigned int num_pkts = num_of_pkts_per_bytes * num_beats;

#ifdef DEBUG_LOG
    printf("[KERNEL_DEBUG]|%s| REALIZED numbers: batch_size: %d, num_beats: %d, num_pkts: %d,\n", __func__,
            batch_size, num_beats, num_pkts);
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


