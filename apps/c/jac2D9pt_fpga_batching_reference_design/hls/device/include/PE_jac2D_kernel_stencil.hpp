// Auto-generated at 2025-06-04 12:59:58.408505 by ops-translator

#pragma once
#include <ops_hls_kernel_support.h>
#include "common_config.hpp"

static constexpr unsigned short read_num_points_jac2D_kernel_stencil = 9;
static constexpr unsigned short read_stencil_size_jac2D_kernel_stencil = 3;
static constexpr unsigned short read_stencil_dim_jac2D_kernel_stencil = 2;

static constexpr unsigned short write_num_points_jac2D_kernel_stencil = 1;
static constexpr unsigned short write_stencil_size_jac2D_kernel_stencil = 1;
static constexpr unsigned short write_stencil_dim_jac2D_kernel_stencil = 2;

static constexpr unsigned short vector_factor_jac2D_kernel_stencil_0 = vector_factor * 1;
static constexpr unsigned short axis_data_width_l_jac2D_kernel_stencil_0 = vector_factor_jac2D_kernel_stencil_0 * data_width;
typedef ap_uint<axis_data_width_l_jac2D_kernel_stencil_0> widen_jac2D_kernel_stencil_0_dt;
static constexpr unsigned short vector_factor_jac2D_kernel_stencil_1 = vector_factor * 1;
static constexpr unsigned short axis_data_width_l_jac2D_kernel_stencil_1 = vector_factor_jac2D_kernel_stencil_1 * data_width;
typedef ap_uint<axis_data_width_l_jac2D_kernel_stencil_1> widen_jac2D_kernel_stencil_1_dt;

typedef ::hls::stream<widen_jac2D_kernel_stencil_0_dt> widen_stream_jac2D_kernel_stencil_0_dt;
typedef ::hls::stream<widen_jac2D_kernel_stencil_1_dt> widen_stream_jac2D_kernel_stencil_1_dt;

/*
    ArgDat(id=0, loc=/home/x_thileeb/repos/OPS_mine/apps/c/jac2D9pt_fpga_experimental/jac2D9pt.cpp/266:21, access_type=AccessType.OPS_READ, opt=True, dat_id=0, global_dat_id=-1, stencil_id=S2D_9PT)
    ArgDat(id=1, loc=/home/x_thileeb/repos/OPS_mine/apps/c/jac2D9pt_fpga_experimental/jac2D9pt.cpp/267:21, access_type=AccessType.OPS_WRITE, opt=True, dat_id=1, global_dat_id=-1, stencil_id=S2D_00)
*/
inline void kernel_jac2D_kernel_stencil_core(
        const stencil_type& reg_0_0,
        const stencil_type& reg_0_1,
        const stencil_type& reg_0_2,
        const stencil_type& reg_0_3,
        const stencil_type& reg_0_4,
        const stencil_type& reg_0_5,
        const stencil_type& reg_0_6,
        const stencil_type& reg_0_7,
        const stencil_type& reg_0_8,
        stencil_type& reg_1_0
)
{
#ifdef DEBUG_LOG
    printf("[KERNEL_INTERNAL_CORE]|%s| starting kernel core: kernel_jac2D_kernel_stencil_core\n",__func__);
#endif

    float tmp1 = reg_0_0 * (-0.07f);
    float tmp2 = reg_0_3 * (-0.08f);
    float tmp3 = reg_0_6 * (-0.01f);
    float tmp4 = reg_0_1 * (-0.06f);
    float tmp5 = reg_0_4 * (0.36f);
    float tmp6 = reg_0_7 * (-0.02f);
    float tmp7 = reg_0_2 * (-0.05f);
    float tmp8 = reg_0_5 * (-0.04f);
    float tmp9 = reg_0_8 * (-0.03f);
    float tmp10 = tmp1 + tmp2;
    float tmp11 = tmp3 + tmp4;
    float tmp12 = tmp5 + tmp6;
    float tmp13 = tmp7 + tmp8;
    float tmp14 = tmp11 + tmp10;
    float tmp15 = tmp12 + tmp13;
    float tmp16 = tmp14 + tmp15;
    reg_1_0 = tmp9 + tmp16;

#ifdef DEBUG_LOG
    printf("[KERNEL_INTERNAL_CORE]|%s| read_val - reg_0_0: %f \n", __func__, reg_0_0);
    printf("[KERNEL_INTERNAL_CORE]|%s| read_val - reg_0_1: %f \n", __func__, reg_0_1);
    printf("[KERNEL_INTERNAL_CORE]|%s| read_val - reg_0_2: %f \n", __func__, reg_0_2);
    printf("[KERNEL_INTERNAL_CORE]|%s| read_val - reg_0_3: %f \n", __func__, reg_0_3);
    printf("[KERNEL_INTERNAL_CORE]|%s| read_val - reg_0_4: %f \n", __func__, reg_0_4);
    printf("[KERNEL_INTERNAL_CORE]|%s| read_val - reg_0_5: %f \n", __func__, reg_0_5);
    printf("[KERNEL_INTERNAL_CORE]|%s| read_val - reg_0_6: %f \n", __func__, reg_0_6);
    printf("[KERNEL_INTERNAL_CORE]|%s| read_val - reg_0_7: %f \n", __func__, reg_0_7);
    printf("[KERNEL_INTERNAL_CORE]|%s| read_val - reg_0_8: %f \n", __func__, reg_0_8);
    printf("[KERNEL_INTERNAL_CORE]|%s| write_val - reg_1_0: %f \n", __func__, reg_1_0);
#endif

#ifdef DEBUG_LOG
    printf("[KERNEL_INTERNAL_CORE]|%s| exiting: kernel_jac2D_kernel_stencil_core\n",__func__);
#endif
}

class Stencil_jac2D_kernel_stencil : public ops::hls::StencilCoreV2<stencil_type, read_num_points_jac2D_kernel_stencil, vector_factor, ops::hls::CoefTypes::CONST_COEF,
        read_stencil_size_jac2D_kernel_stencil, read_stencil_dim_jac2D_kernel_stencil>
{
    using ops::hls::StencilCoreV2<stencil_type, read_num_points_jac2D_kernel_stencil, vector_factor, ops::hls::CoefTypes::CONST_COEF,
            read_stencil_size_jac2D_kernel_stencil, read_stencil_dim_jac2D_kernel_stencil>::m_stencilConfig;
public:

    void stencilRun(
            widen_stream_jac2D_kernel_stencil_0_dt& arg0_rd_buffer,
            widen_stream_jac2D_kernel_stencil_1_dt& arg1_wr_buffer
        )
    {
        const unsigned short span_x = 2;
        const unsigned short half_span_x = 1;

        ::ops::hls::StencilConfigCore stencilConfig = m_stencilConfig;
    //read_origin_wide_diff_x: 1, read_origin_wide_diff: (1,1,0)

    //  *** counters definitions ****
        short i = -2;
        short j = -1; 

    //  *** stencil description and read & write point definitions  ****
        /*
        WindowBufferDiscriptor(widen_stencil=Stencil(id=-1, dim=2, stencil_ptr='read_stencil_widen', num_points=9, points=[Point(x=0, y=0, z=0), Point(x=1, y=0, z=0), Point(x=2, y=0, z=0), Point(x=0, y=1, z=0), Point(x=1, y=1, z=0), Point(x=2, y=1, z=0), Point(x=0, y=2, z=0), Point(x=1, y=2, z=0), Point(x=2, y=2, z=0)], base_point=Point(x=1, y=1, z=0), stencil_size=Point(x=3, y=3, z=1), d_m=Point(x=-0.75, y=1, z=0), d_p=Point(x=1.0, y=1, z=0), row_discriptors=[StencilRowDiscriptor(row_id=(0, 0), base_point=Point(x=1, y=1, z=0), row_points=[Point(x=0, y=0, z=0), Point(x=1, y=0, z=0), Point(x=2, y=0, z=0)]), StencilRowDiscriptor(row_id=(1, 0), base_point=Point(x=1, y=1, z=0), row_points=[Point(x=0, y=1, z=0), Point(x=1, y=1, z=0), Point(x=2, y=1, z=0)]), StencilRowDiscriptor(row_id=(2, 0), base_point=Point(x=1, y=1, z=0), row_points=[Point(x=0, y=2, z=0), Point(x=1, y=2, z=0), Point(x=2, y=2, z=0)])], stride=[], read_origin_diff=Point(x=1, y=1, z=0)), window_buffers=[WindowBuffer(name='buf_r0_1_p0', buffer_type=<BufferType.LINE_BUFF: 0>, read_point=Point(x=0, y=1, z=0), write_point=Point(x=2, y=0, z=0), is_read_write_aligned=False), WindowBuffer(name='buf_r1_2_p0', buffer_type=<BufferType.LINE_BUFF: 0>, read_point=Point(x=0, y=2, z=0), write_point=Point(x=2, y=1, z=0), is_read_write_aligned=False)], chains=[(0, 1), (1, 2), (2, WindowBuffer(name='buf_r0_1_p0', buffer_type=<BufferType.LINE_BUFF: 0>, read_point=Point(x=0, y=1, z=0), write_point=Point(x=2, y=0, z=0), is_read_write_aligned=False)), (3, 4), (4, 5), (5, WindowBuffer(name='buf_r1_2_p0', buffer_type=<BufferType.LINE_BUFF: 0>, read_point=Point(x=0, y=2, z=0), write_point=Point(x=2, y=1, z=0), is_read_write_aligned=False)), (WindowBuffer(name='buf_r0_1_p0', buffer_type=<BufferType.LINE_BUFF: 0>, read_point=Point(x=0, y=1, z=0), write_point=Point(x=2, y=0, z=0), is_read_write_aligned=False), 3), (6, 7), (7, 8), (8, 'read_val'), (WindowBuffer(name='buf_r1_2_p0', buffer_type=<BufferType.LINE_BUFF: 0>, read_point=Point(x=0, y=2, z=0), write_point=Point(x=2, y=1, z=0), is_read_write_aligned=False), 6)], point_to_widen_map={Point(x=0, y=0, z=0): Point(x=0, y=0, z=0), Point(x=1, y=0, z=0): Point(x=1, y=0, z=0), Point(x=2, y=0, z=0): Point(x=2, y=0, z=0), Point(x=0, y=1, z=0): Point(x=0, y=1, z=0), Point(x=1, y=1, z=0): Point(x=1, y=1, z=0), Point(x=2, y=1, z=0): Point(x=2, y=1, z=0), Point(x=0, y=2, z=0): Point(x=0, y=2, z=0), Point(x=1, y=2, z=0): Point(x=1, y=2, z=0), Point(x=2, y=2, z=0): Point(x=2, y=2, z=0)})
        */
        // Stencil: S2D_9PT
        //      |- point: (0,0,0)
        //      |- point: (1,0,0)
        //      |- point: (2,0,0)
        //      |- point: (0,1,0)
        //      |- point: (1,1,0)
        //      |- point: (2,1,0)
        //      |- point: (0,2,0)
        //      |- point: (1,2,0)
        //      `- point: (2,2,0)
        // read point: (0,1,0), write point: (2,0,0)
        unsigned short S2D_9PT_buf_r0_1_p0_rd = 0;
        unsigned short S2D_9PT_buf_r0_1_p0_wr = 2;
        // read point: (0,2,0), write point: (2,1,0)
        unsigned short S2D_9PT_buf_r1_2_p0_rd = 0;
        unsigned short S2D_9PT_buf_r1_2_p0_wr = 2;

        #pragma HLS ARRAY_PARTITION variable = stencilConfig.lower_limit dim = 1 complete
        #pragma HLS ARRAY_PARTITION variable = stencilConfig.upper_limit dim = 1 complete

    //  *** iteration limit definition ****
        unsigned int iter_limit = stencilConfig.outer_loop_limit * stencilConfig.grid_size[0] + 1;

    //  *** data read write boundary definitions ****
    /*        unsigned int arg0_read_lb_itr = 0 * stencilConfig.grid_size[0];
        unsigned int arg0_read_ub_itr = (0 + stencilConfig.grid_size[1]) * stencilConfig.grid_size[0];
*/

        unsigned int read_lb_itr = 0;
        unsigned int read_ub_itr = stencilConfig.grid_size[1] * stencilConfig.grid_size[0];

    //  *** Read & write widen temporaries ****
    // arg0(u)
        widen_jac2D_kernel_stencil_0_dt arg0_read_val = 0;
    // arg1(u2)
        widen_jac2D_kernel_stencil_1_dt arg1_update_val;
    // 2
    //  *** widen stencil values holder & window buffers ****

        // arg0(u)
        widen_jac2D_kernel_stencil_0_dt arg0_widenStencilValues[read_num_points_jac2D_kernel_stencil];
        #pragma HLS ARRAY_PARTITION variable = arg0_widenStencilValues dim = 1 complete

        widen_jac2D_kernel_stencil_0_dt arg0_buf_r0_1_p0[max_depth];
        #pragma HLS BIND_STORAGE variable = arg0_buf_r0_1_p0 type = ram_s2p impl = URAM latency=2
                // curr SIS_id = 0
                // next SIS_id = 1
        widen_jac2D_kernel_stencil_0_dt arg0_buf_r1_2_p0[max_depth];
        #pragma HLS BIND_STORAGE variable = arg0_buf_r1_2_p0 type = ram_s2p impl = BRAM latency=2
                // curr SIS_id = 1
                // next SIS_id = 0

        stencil_type arg0_rowArr_0_0[vector_factor + span_x];
        #pragma HLS ARRAY_PARTITION variable = arg0_rowArr_0_0 dim=1 complete
        stencil_type arg0_rowArr_1_0[vector_factor + span_x];
        #pragma HLS ARRAY_PARTITION variable = arg0_rowArr_1_0 dim=1 complete
        stencil_type arg0_rowArr_2_0[vector_factor + span_x];
        #pragma HLS ARRAY_PARTITION variable = arg0_rowArr_2_0 dim=1 complete

        const short cond_x_val = stencilConfig.grid_size[0] - 1; 
        const short cond_y_val = stencilConfig.outer_loop_limit - 1;

        for (unsigned int itr = 0; itr < iter_limit; itr++)
        {
        #pragma HLS PIPELINE II=1
            unsigned int reg_itr = register_it(itr);

            spc_temp_blocking_read:
            {
                bool cond_x_terminate = (i == cond_x_val ? true : false); 
                bool cond_y_terminate = (j == cond_y_val ? true : false);

#ifdef DEBUG_LOG
                printf("[DEBUG][INTERNAL][jac2D_kernel_stencil_PE_%d] loop params before update i(%d), "\
                    "j(%d), "\
                    "S2D_9PT_buf_r0_1_p0_rd: %d, "\
                    "S2D_9PT_buf_r0_1_p0_wr: %d, "\
                    "S2D_9PT_buf_r1_2_p0_rd: %d, "\
                    "S2D_9PT_buf_r1_2_p0_wr: %d, "\
                    "reg_itr(%d)\n", m_PEId, i, 
                    j,
                    S2D_9PT_buf_r0_1_p0_rd, 
                    S2D_9PT_buf_r0_1_p0_wr, 
                    S2D_9PT_buf_r1_2_p0_rd, 
                    S2D_9PT_buf_r1_2_p0_wr, 
                    reg_itr);
#endif
                if (cond_x_terminate)
                    i = 0;
                else
                    i++;
                if (cond_x_terminate && cond_y_terminate)
                    j = 0;
                else if  (cond_x_terminate)
                    j++;

    /*                bool arg0_read_cond = (reg_itr < arg0_read_ub_itr) and (reg_itr >= arg0_read_lb_itr);
*/
                bool read_cond  =  (reg_itr < read_ub_itr) and (reg_itr >= read_lb_itr);

                if (read_cond)
                {

                    arg0_read_val = arg0_rd_buffer.read();
                }

                arg0_widenStencilValues[0] = arg0_widenStencilValues[1];                
                arg0_widenStencilValues[1] = arg0_widenStencilValues[2];                
                arg0_widenStencilValues[2] = arg0_buf_r0_1_p0[S2D_9PT_buf_r0_1_p0_wr];                
                arg0_widenStencilValues[3] = arg0_widenStencilValues[4];                
                arg0_widenStencilValues[4] = arg0_widenStencilValues[5];                
                arg0_widenStencilValues[5] = arg0_buf_r1_2_p0[S2D_9PT_buf_r1_2_p0_wr];                
                arg0_buf_r0_1_p0[S2D_9PT_buf_r0_1_p0_rd] = arg0_widenStencilValues[3];                
                arg0_widenStencilValues[6] = arg0_widenStencilValues[7];                
                arg0_widenStencilValues[7] = arg0_widenStencilValues[8];                
                arg0_widenStencilValues[8] = arg0_read_val;                
                arg0_buf_r1_2_p0[S2D_9PT_buf_r1_2_p0_rd] = arg0_widenStencilValues[6];

                bool cond_end_of_line_buff_S2D_9PT_buf_r0_1_p0_rd = S2D_9PT_buf_r0_1_p0_rd >= (stencilConfig.grid_size[0] - 1);
                bool cond_end_of_line_buff_S2D_9PT_buf_r0_1_p0_wr = S2D_9PT_buf_r0_1_p0_wr >= (stencilConfig.grid_size[0] - 1);

                if (cond_end_of_line_buff_S2D_9PT_buf_r0_1_p0_rd)
                    S2D_9PT_buf_r0_1_p0_rd = 0;
                else
                    S2D_9PT_buf_r0_1_p0_rd++;

                if (cond_end_of_line_buff_S2D_9PT_buf_r0_1_p0_wr)
                    S2D_9PT_buf_r0_1_p0_wr = 0;
                else
                    S2D_9PT_buf_r0_1_p0_wr++;
                bool cond_end_of_line_buff_S2D_9PT_buf_r1_2_p0_rd = S2D_9PT_buf_r1_2_p0_rd >= (stencilConfig.grid_size[0] - 1);
                bool cond_end_of_line_buff_S2D_9PT_buf_r1_2_p0_wr = S2D_9PT_buf_r1_2_p0_wr >= (stencilConfig.grid_size[0] - 1);

                if (cond_end_of_line_buff_S2D_9PT_buf_r1_2_p0_rd)
                    S2D_9PT_buf_r1_2_p0_rd = 0;
                else
                    S2D_9PT_buf_r1_2_p0_rd++;

                if (cond_end_of_line_buff_S2D_9PT_buf_r1_2_p0_wr)
                    S2D_9PT_buf_r1_2_p0_wr = 0;
                else
                    S2D_9PT_buf_r1_2_p0_wr++;

#ifdef DEBUG_LOG
                printf("[DEBUG][INTERNAL][jac2D_kernel_stencil_PE_%d] loop params after update i(%d), "\
                                "j(%d), "\
                                "S2D_9PT_buf_r0_1_p0_rd(%d), "\
                                "S2D_9PT_buf_r0_1_p0_wr(%d), "\
                                "S2D_9PT_buf_r1_2_p0_rd(%d), "\
                                "S2D_9PT_buf_r1_2_p0_wr(%d), "\
                                "reg_itr(%d)\n", m_PEId, i, 
                                j,
                                S2D_9PT_buf_r0_1_p0_rd, 
                                S2D_9PT_buf_r0_1_p0_wr, 
                                S2D_9PT_buf_r1_2_p0_rd, 
                                S2D_9PT_buf_r1_2_p0_wr, 
                                reg_itr);

                printf("[DEBUG][INTERNAL][jac2D_kernel_stencil_PE_%d] --------------------------------------------------------\n\n", m_PEId);

                printf("[DEBUG][INTERNAL][jac2D_kernel_stencil_PE_%d] read values arg0: (", m_PEId);
                for (int ri = 0; ri < vector_factor; ri++)
                {
                    ops::hls::DataConv tmpConverter;
                    tmpConverter.i = arg0_read_val.range((ri + 1)*s_datatype_size - 1, ri * s_datatype_size);

                    printf("%f ", tmpConverter.f);
                }
                printf(")\n");
#endif      
            }

            vec2arr: for (unsigned short x = 0; x < vector_factor; x++)
            {
#pragma HLS UNROLL factor=vector_factor
                ops::hls::DataConv arg0_tmpConverter_0_0;
                arg0_tmpConverter_0_0.i = arg0_widenStencilValues[1].range(s_datatype_size * (x + 1) - 1, x * s_datatype_size);
                arg0_rowArr_0_0[x + half_span_x] = arg0_tmpConverter_0_0.f; 
                ops::hls::DataConv arg0_tmpConverter_1_0;
                arg0_tmpConverter_1_0.i = arg0_widenStencilValues[4].range(s_datatype_size * (x + 1) - 1, x * s_datatype_size);
                arg0_rowArr_1_0[x + half_span_x] = arg0_tmpConverter_1_0.f; 
                ops::hls::DataConv arg0_tmpConverter_2_0;
                arg0_tmpConverter_2_0.i = arg0_widenStencilValues[7].range(s_datatype_size * (x + 1) - 1, x * s_datatype_size);
                arg0_rowArr_2_0[x + half_span_x] = arg0_tmpConverter_2_0.f; 

            }
            vec2arr_rest:
            {
                //diff = -1
                // access_idx = 0
                ops::hls::DataConv arg0_tmpConverter_0_0_0_7;
                arg0_tmpConverter_0_0_0_7.i = arg0_widenStencilValues[0].range(s_datatype_size * (7 + 1) - 1, s_datatype_size * 7);
                arg0_rowArr_0_0[0] = arg0_tmpConverter_0_0_0_7.f;
                //diff = 1
                // access_idx = 9
                ops::hls::DataConv arg0_tmpConverter_2_0_0_0;
                arg0_tmpConverter_2_0_0_0.i = arg0_widenStencilValues[2].range(s_datatype_size * (0 + 1) - 1, s_datatype_size * 0);
                arg0_rowArr_0_0[9] = arg0_tmpConverter_2_0_0_0.f;
                //diff = -1
                // access_idx = 0
                ops::hls::DataConv arg0_tmpConverter_0_1_0_7;
                arg0_tmpConverter_0_1_0_7.i = arg0_widenStencilValues[3].range(s_datatype_size * (7 + 1) - 1, s_datatype_size * 7);
                arg0_rowArr_1_0[0] = arg0_tmpConverter_0_1_0_7.f;
                //diff = 1
                // access_idx = 9
                ops::hls::DataConv arg0_tmpConverter_2_1_0_0;
                arg0_tmpConverter_2_1_0_0.i = arg0_widenStencilValues[5].range(s_datatype_size * (0 + 1) - 1, s_datatype_size * 0);
                arg0_rowArr_1_0[9] = arg0_tmpConverter_2_1_0_0.f;
                //diff = -1
                // access_idx = 0
                ops::hls::DataConv arg0_tmpConverter_0_2_0_7;
                arg0_tmpConverter_0_2_0_7.i = arg0_widenStencilValues[6].range(s_datatype_size * (7 + 1) - 1, s_datatype_size * 7);
                arg0_rowArr_2_0[0] = arg0_tmpConverter_0_2_0_7.f;
                //diff = 1
                // access_idx = 9
                ops::hls::DataConv arg0_tmpConverter_2_2_0_0;
                arg0_tmpConverter_2_2_0_0.i = arg0_widenStencilValues[8].range(s_datatype_size * (0 + 1) - 1, s_datatype_size * 0);
                arg0_rowArr_2_0[9] = arg0_tmpConverter_2_2_0_0.f;
            }

            process: for (unsigned short x = 0; x < vector_factor; x++)
            {
#pragma HLS UNROLL factor=vector_factor
                short index = (i << shift_bits) + x;
                bool neg_cond = register_it(             
                        (index < stencilConfig.lower_limit[0]) 
                        || (index >= stencilConfig.upper_limit[0])
                        || (j < stencilConfig.lower_limit[1]) 
                        || (j >= stencilConfig.upper_limit[1])
                );

#ifdef DEBUG_LOG
                printf("[DEBUG][INTERNAL][jac2D_kernel_stencil_PE_%d] index=(%d, %d), lowerbound=(%d, %d), upperbound=(%d, %d), neg_cond=%d\n", m_PEId, index, j,
                            stencilConfig.lower_limit[0], stencilConfig.lower_limit[1], stencilConfig.upper_limit[0], stencilConfig.upper_limit[1], neg_cond);
#endif

                stencil_type arg1_result;

                kernel_jac2D_kernel_stencil_core(
                        arg0_rowArr_0_0[x + 0],
                        arg0_rowArr_0_0[x + 1],
                        arg0_rowArr_0_0[x + 2],
                        arg0_rowArr_1_0[x + 0],
                        arg0_rowArr_1_0[x + 1],
                        arg0_rowArr_1_0[x + 2],
                        arg0_rowArr_2_0[x + 0],
                        arg0_rowArr_2_0[x + 1],
                        arg0_rowArr_2_0[x + 2],
                        arg1_result
                );

                ops::hls::DataConv arg1_tmpConvWrite;

                if (not neg_cond)
                {
                    arg1_tmpConvWrite.f = arg1_result;
                }
                else
                {

    // [1, 0]
            //dat_id: 1, dat: u2, swap_id: 0, swap_dat: u
                    arg1_tmpConvWrite.f = arg0_rowArr_1_0[x + 1];
                }
            // *** rw convertions ***

                arg1_update_val.range(s_datatype_size * (x + 1) - 1, x * s_datatype_size) = arg1_tmpConvWrite.i;

            }

            write:
            {
                bool cond_write = (j >= 0);

                if (cond_write)
                {
#ifdef DEBUG_LOG
                    printf("[DEBUG][INTERNAL][jac2D_kernel_stencil_PE_%d] --------------------------------------------------------\n\n", m_PEId);

                    printf("[DEBUG][INTERNAL][jac2D_kernel_stencil_PE_%d] wirte values arg1: (", m_PEId);
                    for (int ri = 0; ri < vector_factor; ri++)
                    {
                        ops::hls::DataConv tmpConverter;
                        tmpConverter.i = arg1_update_val.range((ri + 1)*s_datatype_size - 1, ri * s_datatype_size);
                        printf("%f ", tmpConverter.f);
                    }
                    printf(")\n");
#endif
                    arg1_wr_buffer <<  arg1_update_val;
                }
            }
        }
    } 
};

void kernel_jac2D_kernel_stencil_PE(const short& PEId, const ops::hls::StencilConfigCore& stencilConfig,
            //u
    widen_stream_jac2D_kernel_stencil_0_dt& arg0_rd_buffer,
            //u2
    widen_stream_jac2D_kernel_stencil_1_dt& arg1_wr_buffer        
)
{
    Stencil_jac2D_kernel_stencil stencil;

#ifdef DEBUG_LOG
    printf("[KERNEL_DEBUG]|%s| stencil config gridSize: %d (xblocks), %d, %d\n", __func__, stencilConfig.grid_size[0], stencilConfig.grid_size[1], stencilConfig.grid_size[2]);
#endif
    stencil.setConfig(PEId, stencilConfig);

#ifdef DEBUG_LOG
    printf("[KERNEL_DEBUG]|%s| starting stencil kernel PE\n", __func__);
#endif

    stencil.stencilRun(
            arg0_rd_buffer,
            arg1_wr_buffer

);

#ifdef DEBUG_LOG
    printf("[KERNEL_DEBUG]|%s| Ending stencil kernel PE\n", __func__);
#endif
} 
