// Auto-generated at 2025-09-06 21:18:49.758949 by ops-translator

#pragma once
#include <ops_hls_kernel_support.h>
#include "common_config.hpp"

static constexpr unsigned short read_num_points_jac3D_kernel_stencil = 7;
static constexpr unsigned short read_stencil_size_jac3D_kernel_stencil = 3;
static constexpr unsigned short read_stencil_dim_jac3D_kernel_stencil = 3;

static constexpr unsigned short write_num_points_jac3D_kernel_stencil = 1;
static constexpr unsigned short write_stencil_size_jac3D_kernel_stencil = 1;
static constexpr unsigned short write_stencil_dim_jac3D_kernel_stencil = 3;

static constexpr unsigned short vector_factor_jac3D_kernel_stencil_0 = vector_factor * 1;
static constexpr unsigned short axis_data_width_l_jac3D_kernel_stencil_0 = vector_factor_jac3D_kernel_stencil_0 * data_width;
typedef ap_uint<axis_data_width_l_jac3D_kernel_stencil_0> widen_jac3D_kernel_stencil_0_dt;
static constexpr unsigned short vector_factor_jac3D_kernel_stencil_1 = vector_factor * 1;
static constexpr unsigned short axis_data_width_l_jac3D_kernel_stencil_1 = vector_factor_jac3D_kernel_stencil_1 * data_width;
typedef ap_uint<axis_data_width_l_jac3D_kernel_stencil_1> widen_jac3D_kernel_stencil_1_dt;

typedef ::hls::stream<widen_jac3D_kernel_stencil_0_dt> widen_stream_jac3D_kernel_stencil_0_dt;
typedef ::hls::stream<widen_jac3D_kernel_stencil_1_dt> widen_stream_jac3D_kernel_stencil_1_dt;

/*
    ArgDat(id=0, loc=/home/x_thileeb/repos/ops-hls-batching-tiling-artifacts/codegen_apps/jacobian3d/u280_project/jac3D7pt.cpp/315:21, access_type=AccessType.OPS_READ, opt=True, dat_id=0, global_dat_id=-1, stencil_id=S3D_7PT)
    ArgDat(id=1, loc=/home/x_thileeb/repos/ops-hls-batching-tiling-artifacts/codegen_apps/jacobian3d/u280_project/jac3D7pt.cpp/316:21, access_type=AccessType.OPS_WRITE, opt=True, dat_id=1, global_dat_id=-1, stencil_id=S3D_00)
*/
inline void kernel_jac3D_kernel_stencil_core(
        const stencil_type& reg_0_0,
        const stencil_type& reg_0_1,
        const stencil_type& reg_0_2,
        const stencil_type& reg_0_3,
        const stencil_type& reg_0_4,
        const stencil_type& reg_0_5,
        const stencil_type& reg_0_6,
        stencil_type& reg_1_0
)
{
#ifdef DEBUG_LOG
    printf("[KERNEL_INTERNAL_CORE]|%s| starting kernel core: kernel_jac3D_kernel_stencil_core\n",__func__);
#endif

    float tmp1 = reg_0_6 * (0.02f);
    float tmp2 = reg_0_5 * (0.04f);
    float tmp3 = reg_0_2 * (0.05f);
    float tmp4 = reg_0_3 * (0.79f);
    float tmp5 = reg_0_4 * (0.06f);
    float tmp6 = reg_0_1 * (0.03f);
    float tmp7 = reg_0_0 * (0.01f);
    float tmp8 = tmp1 + tmp2;
    float tmp9 = tmp3 + tmp4;
    float tmp10 = tmp5 + tmp6;
    float tmp11 = tmp7 + tmp8;
    float tmp12 = tmp9 + tmp10;
    reg_1_0 = tmp11 + tmp12;

#ifdef DEBUG_LOG
    printf("[KERNEL_INTERNAL_CORE]|%s| read_val - reg_0_0: %f \n", __func__, reg_0_0);
    printf("[KERNEL_INTERNAL_CORE]|%s| read_val - reg_0_1: %f \n", __func__, reg_0_1);
    printf("[KERNEL_INTERNAL_CORE]|%s| read_val - reg_0_2: %f \n", __func__, reg_0_2);
    printf("[KERNEL_INTERNAL_CORE]|%s| read_val - reg_0_3: %f \n", __func__, reg_0_3);
    printf("[KERNEL_INTERNAL_CORE]|%s| read_val - reg_0_4: %f \n", __func__, reg_0_4);
    printf("[KERNEL_INTERNAL_CORE]|%s| read_val - reg_0_5: %f \n", __func__, reg_0_5);
    printf("[KERNEL_INTERNAL_CORE]|%s| read_val - reg_0_6: %f \n", __func__, reg_0_6);
    printf("[KERNEL_INTERNAL_CORE]|%s| write_val - reg_1_0: %f \n", __func__, reg_1_0);
#endif

#ifdef DEBUG_LOG
    printf("[KERNEL_INTERNAL_CORE]|%s| exiting: kernel_jac3D_kernel_stencil_core\n",__func__);
#endif
}

class Stencil_jac3D_kernel_stencil : public ops::hls::StencilCoreV2<stencil_type, read_num_points_jac3D_kernel_stencil, vector_factor, ops::hls::CoefTypes::CONST_COEF,
        read_stencil_size_jac3D_kernel_stencil, read_stencil_dim_jac3D_kernel_stencil>
{
    using ops::hls::StencilCoreV2<stencil_type, read_num_points_jac3D_kernel_stencil, vector_factor, ops::hls::CoefTypes::CONST_COEF,
            read_stencil_size_jac3D_kernel_stencil, read_stencil_dim_jac3D_kernel_stencil>::m_stencilConfig;
public:

    void stencilRun(
            widen_stream_jac3D_kernel_stencil_0_dt& arg0_rd_buffer,
            widen_stream_jac3D_kernel_stencil_1_dt& arg1_wr_buffer
        )
    {
        const unsigned short span_x = 2;
        const unsigned short half_span_x = 1;

        ::ops::hls::StencilConfigCore stencilConfig = m_stencilConfig;

        for (unsigned bat = 0; bat < stencilConfig.batch_size; bat++)
        {
        //read_origin_wide_diff_x: 0, read_origin_wide_diff: (0,0,1)

        //  *** counters definitions ****
            short i = -1;
            short j = 0; 
            short k = -1;
            unsigned short plane_diff = stencilConfig.grid_size[0] * stencilConfig.grid_size[1] - 1;

        //  *** stencil description and read & write point definitions  ****
            /*
            WindowBufferDiscriptor(widen_stencil=Stencil(id=-1, dim=3, stencil_ptr='read_stencil_widen', num_points=7, points=[Point(x=1, y=1, z=0), Point(x=1, y=0, z=1), Point(x=0, y=1, z=1), Point(x=1, y=1, z=1), Point(x=2, y=1, z=1), Point(x=1, y=2, z=1), Point(x=1, y=1, z=2)], base_point=Point(x=1, y=1, z=1), stencil_size=Point(x=3, y=3, z=3), d_m=Point(x=-0.75, y=1, z=1), d_p=Point(x=1.0, y=1, z=1), row_discriptors=[StencilRowDiscriptor(row_id=(1, 0), base_point=Point(x=1, y=1, z=1), row_points=[Point(x=1, y=1, z=0)]), StencilRowDiscriptor(row_id=(0, 1), base_point=Point(x=1, y=1, z=1), row_points=[Point(x=1, y=0, z=1)]), StencilRowDiscriptor(row_id=(1, 1), base_point=Point(x=1, y=1, z=1), row_points=[Point(x=0, y=1, z=1), Point(x=1, y=1, z=1), Point(x=2, y=1, z=1)]), StencilRowDiscriptor(row_id=(2, 1), base_point=Point(x=1, y=1, z=1), row_points=[Point(x=1, y=2, z=1)]), StencilRowDiscriptor(row_id=(1, 2), base_point=Point(x=1, y=1, z=1), row_points=[Point(x=1, y=1, z=2)])], stride=[], read_origin_diff=Point(x=0, y=0, z=1)), window_buffers=[WindowBuffer(name='buf_p0_1', buffer_type=<BufferType.PLANE_BUFF: 1>, read_point=Point(x=1, y=0, z=1), write_point=Point(x=1, y=1, z=0), is_read_write_aligned=False), WindowBuffer(name='buf_r0_1_p1', buffer_type=<BufferType.LINE_BUFF: 0>, read_point=Point(x=0, y=1, z=1), write_point=Point(x=1, y=0, z=1), is_read_write_aligned=False), WindowBuffer(name='buf_r1_2_p1', buffer_type=<BufferType.LINE_BUFF: 0>, read_point=Point(x=1, y=2, z=1), write_point=Point(x=2, y=1, z=1), is_read_write_aligned=False), WindowBuffer(name='buf_p1_2', buffer_type=<BufferType.PLANE_BUFF: 1>, read_point=Point(x=1, y=1, z=2), write_point=Point(x=1, y=2, z=1), is_read_write_aligned=False)], chains=[(0, WindowBuffer(name='buf_p0_1', buffer_type=<BufferType.PLANE_BUFF: 1>, read_point=Point(x=1, y=0, z=1), write_point=Point(x=1, y=1, z=0), is_read_write_aligned=False)), (1, WindowBuffer(name='buf_r0_1_p1', buffer_type=<BufferType.LINE_BUFF: 0>, read_point=Point(x=0, y=1, z=1), write_point=Point(x=1, y=0, z=1), is_read_write_aligned=False)), (WindowBuffer(name='buf_p0_1', buffer_type=<BufferType.PLANE_BUFF: 1>, read_point=Point(x=1, y=0, z=1), write_point=Point(x=1, y=1, z=0), is_read_write_aligned=False), 1), (2, 3), (3, 4), (4, WindowBuffer(name='buf_r1_2_p1', buffer_type=<BufferType.LINE_BUFF: 0>, read_point=Point(x=1, y=2, z=1), write_point=Point(x=2, y=1, z=1), is_read_write_aligned=False)), (WindowBuffer(name='buf_r0_1_p1', buffer_type=<BufferType.LINE_BUFF: 0>, read_point=Point(x=0, y=1, z=1), write_point=Point(x=1, y=0, z=1), is_read_write_aligned=False), 2), (5, WindowBuffer(name='buf_p1_2', buffer_type=<BufferType.PLANE_BUFF: 1>, read_point=Point(x=1, y=1, z=2), write_point=Point(x=1, y=2, z=1), is_read_write_aligned=False)), (WindowBuffer(name='buf_r1_2_p1', buffer_type=<BufferType.LINE_BUFF: 0>, read_point=Point(x=1, y=2, z=1), write_point=Point(x=2, y=1, z=1), is_read_write_aligned=False), 5), (6, 'read_val'), (WindowBuffer(name='buf_p1_2', buffer_type=<BufferType.PLANE_BUFF: 1>, read_point=Point(x=1, y=1, z=2), write_point=Point(x=1, y=2, z=1), is_read_write_aligned=False), 6)], point_to_widen_map={Point(x=1, y=1, z=0): Point(x=1, y=1, z=0), Point(x=1, y=0, z=1): Point(x=1, y=0, z=1), Point(x=0, y=1, z=1): Point(x=0, y=1, z=1), Point(x=1, y=1, z=1): Point(x=1, y=1, z=1), Point(x=2, y=1, z=1): Point(x=2, y=1, z=1), Point(x=1, y=2, z=1): Point(x=1, y=2, z=1), Point(x=1, y=1, z=2): Point(x=1, y=1, z=2)})
            */
            // Stencil: S3D_7PT
            //      |- point: (1,1,0)
            //      |- point: (1,0,1)
            //      |- point: (0,1,1)
            //      |- point: (1,1,1)
            //      |- point: (2,1,1)
            //      |- point: (1,2,1)
            //      `- point: (1,1,2)
            unsigned short S3D_7PT_buf_p0_1_rd;

            if (-1 * stencilConfig.grid_size[0] + 0 > 0)
                S3D_7PT_buf_p0_1_rd = -1 * stencilConfig.grid_size[0] + 0;
            else
                S3D_7PT_buf_p0_1_rd = 0;

            unsigned short S3D_7PT_buf_p0_1_wr;

            if (1 * stencilConfig.grid_size[0] + 0 > 0)
                S3D_7PT_buf_p0_1_wr = 1 * stencilConfig.grid_size[0] + 0;
            else
                S3D_7PT_buf_p0_1_wr = 0;
            // read point: (0,1,1), write point: (1,0,1)
            unsigned short S3D_7PT_buf_r0_1_p1_rd = 0;
            unsigned short S3D_7PT_buf_r0_1_p1_wr = 1;
            // read point: (1,2,1), write point: (2,1,1)
            unsigned short S3D_7PT_buf_r1_2_p1_rd = 0;
            unsigned short S3D_7PT_buf_r1_2_p1_wr = 1;
            unsigned short S3D_7PT_buf_p1_2_rd;

            if (-1 * stencilConfig.grid_size[0] + 0 > 0)
                S3D_7PT_buf_p1_2_rd = -1 * stencilConfig.grid_size[0] + 0;
            else
                S3D_7PT_buf_p1_2_rd = 0;

            unsigned short S3D_7PT_buf_p1_2_wr;

            if (1 * stencilConfig.grid_size[0] + 0 > 0)
                S3D_7PT_buf_p1_2_wr = 1 * stencilConfig.grid_size[0] + 0;
            else
                S3D_7PT_buf_p1_2_wr = 0;

            #pragma HLS ARRAY_PARTITION variable = stencilConfig.lower_limit dim = 1 complete
            #pragma HLS ARRAY_PARTITION variable = stencilConfig.upper_limit dim = 1 complete

        //  *** iteration limit definition ****
            unsigned int iter_limit = stencilConfig.outer_loop_limit * 
                    stencilConfig.grid_size[1] * stencilConfig.grid_size[0] ;

        //  *** data read write boundary definitions ****
            unsigned int S3D_7PT_read_lb_itr = 0 * stencilConfig.grid_size[0] * stencilConfig.grid_size[1];
            unsigned int S3D_7PT_read_ub_itr = (0 + stencilConfig.grid_size[2]) * stencilConfig.grid_size[1] * stencilConfig.grid_size[0];

        /*
            unsigned int read_lb_itr = 0;
            unsigned int read_ub_itr = stencilConfig.grid_size[2] * stencilConfig.grid_size[1] * stencilConfig.grid_size[0];
        */
        //  *** Read & write widen temporaries ****
        // arg0(u)
            widen_jac3D_kernel_stencil_0_dt arg0_read_val = 0;
        // arg1(u2)
            widen_jac3D_kernel_stencil_1_dt arg1_update_val;
        // 2
        //  *** widen stencil values holder & window buffers ****

            // arg0(u)
            widen_jac3D_kernel_stencil_0_dt arg0_widenStencilValues[read_num_points_jac3D_kernel_stencil];
            #pragma HLS ARRAY_PARTITION variable = arg0_widenStencilValues dim = 1 complete

            widen_jac3D_kernel_stencil_0_dt arg0_buf_p0_1[max_depth];
            #pragma HLS BIND_STORAGE variable = arg0_buf_p0_1 type = ram_s2p impl = URAM latency=2
                    // curr SIS_id = 0
                    // next SIS_id = 1
            widen_jac3D_kernel_stencil_0_dt arg0_buf_r0_1_p1[line_buff_3d_depth];
            #pragma HLS BIND_STORAGE variable = arg0_buf_r0_1_p1 type = ram_s2p impl = BRAM latency=2
                    // curr SIS_id = 1
                    // next SIS_id = 0
            widen_jac3D_kernel_stencil_0_dt arg0_buf_r1_2_p1[line_buff_3d_depth];
            #pragma HLS BIND_STORAGE variable = arg0_buf_r1_2_p1 type = ram_s2p impl = URAM latency=2
                    // curr SIS_id = 0
                    // next SIS_id = 1
            widen_jac3D_kernel_stencil_0_dt arg0_buf_p1_2[max_depth];
            #pragma HLS BIND_STORAGE variable = arg0_buf_p1_2 type = ram_s2p impl = BRAM latency=2
                    // curr SIS_id = 1
                    // next SIS_id = 0

            stencil_type arg0_rowArr_1_0[vector_factor + span_x];
            #pragma HLS ARRAY_PARTITION variable = arg0_rowArr_1_0 dim=1 complete
            stencil_type arg0_rowArr_0_1[vector_factor + span_x];
            #pragma HLS ARRAY_PARTITION variable = arg0_rowArr_0_1 dim=1 complete
            stencil_type arg0_rowArr_1_1[vector_factor + span_x];
            #pragma HLS ARRAY_PARTITION variable = arg0_rowArr_1_1 dim=1 complete
            stencil_type arg0_rowArr_2_1[vector_factor + span_x];
            #pragma HLS ARRAY_PARTITION variable = arg0_rowArr_2_1 dim=1 complete
            stencil_type arg0_rowArr_1_2[vector_factor + span_x];
            #pragma HLS ARRAY_PARTITION variable = arg0_rowArr_1_2 dim=1 complete

            const short cond_x_val = stencilConfig.grid_size[0] - 1; 
            const short cond_y_val = stencilConfig.grid_size[1] - 1;
            const short cond_z_val = stencilConfig.outer_loop_limit - 1;

            for (unsigned int itr = 0; itr < iter_limit; itr++)
            {
            #pragma HLS PIPELINE II=1
                unsigned int reg_itr = register_it(itr);

                spc_temp_blocking_read:
                {
                    bool cond_x_terminate = (i == cond_x_val ? true : false); 
                    bool cond_y_terminate = (j == cond_y_val ? true : false);
                    bool cond_z_terminate = (k == cond_z_val ? true : false);

    #ifdef DEBUG_LOG
                    printf("[DEBUG][INTERNAL][jac3D_kernel_stencil_PE_%d] loop params before update i(%d), "\
                        "j(%d), "\
                        "k(%d), "\
                        "S3D_7PT_buf_p0_1_rd: %d, "\
                        "S3D_7PT_buf_p0_1_wr: %d, "\
                        "S3D_7PT_buf_r0_1_p1_rd: %d, "\
                        "S3D_7PT_buf_r0_1_p1_wr: %d, "\
                        "S3D_7PT_buf_r1_2_p1_rd: %d, "\
                        "S3D_7PT_buf_r1_2_p1_wr: %d, "\
                        "S3D_7PT_buf_p1_2_rd: %d, "\
                        "S3D_7PT_buf_p1_2_wr: %d, "\
                        "reg_itr(%d)\n", m_PEId, i, 
                        j,
                        k,
                        S3D_7PT_buf_p0_1_rd, 
                        S3D_7PT_buf_p0_1_wr, 
                        S3D_7PT_buf_r0_1_p1_rd, 
                        S3D_7PT_buf_r0_1_p1_wr, 
                        S3D_7PT_buf_r1_2_p1_rd, 
                        S3D_7PT_buf_r1_2_p1_wr, 
                        S3D_7PT_buf_p1_2_rd, 
                        S3D_7PT_buf_p1_2_wr, 
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
                    if (cond_x_terminate && cond_y_terminate && cond_z_terminate)
                        k = 0;
                    else if (cond_x_terminate && cond_y_terminate)
                        k++;

                    bool S3D_7PT_read_cond = (reg_itr < S3D_7PT_read_ub_itr) and (reg_itr >= S3D_7PT_read_lb_itr);
                /*    bool read_cond  =  (reg_itr < read_ub_itr) and (reg_itr >= read_lb_itr); */

                    if (S3D_7PT_read_cond)
                    {
                        arg0_read_val = arg0_rd_buffer.read();
                    }

                    /*if (read_cond)
                    {

                        arg0_read_val = arg0_rd_buffer.read();
                    }*/

                    arg0_widenStencilValues[0] = arg0_buf_p0_1[S3D_7PT_buf_p0_1_wr];                
                    arg0_widenStencilValues[1] = arg0_buf_r0_1_p1[S3D_7PT_buf_r0_1_p1_wr];                
                    arg0_buf_p0_1[S3D_7PT_buf_p0_1_rd] = arg0_widenStencilValues[1];                
                    arg0_widenStencilValues[2] = arg0_widenStencilValues[3];                
                    arg0_widenStencilValues[3] = arg0_widenStencilValues[4];                
                    arg0_widenStencilValues[4] = arg0_buf_r1_2_p1[S3D_7PT_buf_r1_2_p1_wr];                
                    arg0_buf_r0_1_p1[S3D_7PT_buf_r0_1_p1_rd] = arg0_widenStencilValues[2];                
                    arg0_widenStencilValues[5] = arg0_buf_p1_2[S3D_7PT_buf_p1_2_wr];                
                    arg0_buf_r1_2_p1[S3D_7PT_buf_r1_2_p1_rd] = arg0_widenStencilValues[5];                
                    arg0_widenStencilValues[6] = arg0_read_val;                
                    arg0_buf_p1_2[S3D_7PT_buf_p1_2_rd] = arg0_widenStencilValues[6];

                    bool cond_end_of_line_buff_S3D_7PT_buf_p0_1_rd = S3D_7PT_buf_p0_1_rd >= (plane_diff);
                    bool cond_end_of_line_buff_S3D_7PT_buf_p0_1_wr = S3D_7PT_buf_p0_1_wr >= (plane_diff);

                    if (cond_end_of_line_buff_S3D_7PT_buf_p0_1_rd)
                        S3D_7PT_buf_p0_1_rd = 0;
                    else
                        S3D_7PT_buf_p0_1_rd++;

                    if (cond_end_of_line_buff_S3D_7PT_buf_p0_1_wr)
                        S3D_7PT_buf_p0_1_wr = 0;
                    else
                        S3D_7PT_buf_p0_1_wr++;
                    bool cond_end_of_line_buff_S3D_7PT_buf_r0_1_p1_rd = S3D_7PT_buf_r0_1_p1_rd >= (stencilConfig.grid_size[0] - 1);
                    bool cond_end_of_line_buff_S3D_7PT_buf_r0_1_p1_wr = S3D_7PT_buf_r0_1_p1_wr >= (stencilConfig.grid_size[0] - 1);

                    if (cond_end_of_line_buff_S3D_7PT_buf_r0_1_p1_rd)
                        S3D_7PT_buf_r0_1_p1_rd = 0;
                    else
                        S3D_7PT_buf_r0_1_p1_rd++;

                    if (cond_end_of_line_buff_S3D_7PT_buf_r0_1_p1_wr)
                        S3D_7PT_buf_r0_1_p1_wr = 0;
                    else
                        S3D_7PT_buf_r0_1_p1_wr++;
                    bool cond_end_of_line_buff_S3D_7PT_buf_r1_2_p1_rd = S3D_7PT_buf_r1_2_p1_rd >= (stencilConfig.grid_size[0] - 1);
                    bool cond_end_of_line_buff_S3D_7PT_buf_r1_2_p1_wr = S3D_7PT_buf_r1_2_p1_wr >= (stencilConfig.grid_size[0] - 1);

                    if (cond_end_of_line_buff_S3D_7PT_buf_r1_2_p1_rd)
                        S3D_7PT_buf_r1_2_p1_rd = 0;
                    else
                        S3D_7PT_buf_r1_2_p1_rd++;

                    if (cond_end_of_line_buff_S3D_7PT_buf_r1_2_p1_wr)
                        S3D_7PT_buf_r1_2_p1_wr = 0;
                    else
                        S3D_7PT_buf_r1_2_p1_wr++;
                    bool cond_end_of_line_buff_S3D_7PT_buf_p1_2_rd = S3D_7PT_buf_p1_2_rd >= (plane_diff);
                    bool cond_end_of_line_buff_S3D_7PT_buf_p1_2_wr = S3D_7PT_buf_p1_2_wr >= (plane_diff);

                    if (cond_end_of_line_buff_S3D_7PT_buf_p1_2_rd)
                        S3D_7PT_buf_p1_2_rd = 0;
                    else
                        S3D_7PT_buf_p1_2_rd++;

                    if (cond_end_of_line_buff_S3D_7PT_buf_p1_2_wr)
                        S3D_7PT_buf_p1_2_wr = 0;
                    else
                        S3D_7PT_buf_p1_2_wr++;

    #ifdef DEBUG_LOG
                    printf("[DEBUG][INTERNAL][jac3D_kernel_stencil_PE_%d] loop params after update i(%d), "\
                                    "j(%d), "\
                                    "k(%d), "\
                                    "S3D_7PT_buf_p0_1_rd(%d), "\
                                    "S3D_7PT_buf_p0_1_wr(%d), "\
                                    "S3D_7PT_buf_r0_1_p1_rd(%d), "\
                                    "S3D_7PT_buf_r0_1_p1_wr(%d), "\
                                    "S3D_7PT_buf_r1_2_p1_rd(%d), "\
                                    "S3D_7PT_buf_r1_2_p1_wr(%d), "\
                                    "S3D_7PT_buf_p1_2_rd(%d), "\
                                    "S3D_7PT_buf_p1_2_wr(%d), "\
                                    "reg_itr(%d)\n", m_PEId, i, 
                                    j,
                                    k,
                                    S3D_7PT_buf_p0_1_rd, 
                                    S3D_7PT_buf_p0_1_wr, 
                                    S3D_7PT_buf_r0_1_p1_rd, 
                                    S3D_7PT_buf_r0_1_p1_wr, 
                                    S3D_7PT_buf_r1_2_p1_rd, 
                                    S3D_7PT_buf_r1_2_p1_wr, 
                                    S3D_7PT_buf_p1_2_rd, 
                                    S3D_7PT_buf_p1_2_wr, 
                                    reg_itr);

                    printf("[DEBUG][INTERNAL][jac3D_kernel_stencil_PE_%d] --------------------------------------------------------\n\n", m_PEId);

                    printf("[DEBUG][INTERNAL][jac3D_kernel_stencil_PE_%d] read values arg0: (", m_PEId);
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
                    ops::hls::DataConv arg0_tmpConverter_1_0;
                    arg0_tmpConverter_1_0.i = arg0_widenStencilValues[0].range(s_datatype_size * (x + 1) - 1, x * s_datatype_size);
                    arg0_rowArr_1_0[x + half_span_x] = arg0_tmpConverter_1_0.f; 
                    ops::hls::DataConv arg0_tmpConverter_0_1;
                    arg0_tmpConverter_0_1.i = arg0_widenStencilValues[1].range(s_datatype_size * (x + 1) - 1, x * s_datatype_size);
                    arg0_rowArr_0_1[x + half_span_x] = arg0_tmpConverter_0_1.f; 
                    ops::hls::DataConv arg0_tmpConverter_1_1;
                    arg0_tmpConverter_1_1.i = arg0_widenStencilValues[3].range(s_datatype_size * (x + 1) - 1, x * s_datatype_size);
                    arg0_rowArr_1_1[x + half_span_x] = arg0_tmpConverter_1_1.f; 
                    ops::hls::DataConv arg0_tmpConverter_2_1;
                    arg0_tmpConverter_2_1.i = arg0_widenStencilValues[5].range(s_datatype_size * (x + 1) - 1, x * s_datatype_size);
                    arg0_rowArr_2_1[x + half_span_x] = arg0_tmpConverter_2_1.f; 
                    ops::hls::DataConv arg0_tmpConverter_1_2;
                    arg0_tmpConverter_1_2.i = arg0_widenStencilValues[6].range(s_datatype_size * (x + 1) - 1, x * s_datatype_size);
                    arg0_rowArr_1_2[x + half_span_x] = arg0_tmpConverter_1_2.f; 

                }
                vec2arr_rest:
                {
                    //diff = -1
                    // access_idx = 0
                    ops::hls::DataConv arg0_tmpConverter_0_1_1_7;
                    arg0_tmpConverter_0_1_1_7.i = arg0_widenStencilValues[2].range(s_datatype_size * (7 + 1) - 1, s_datatype_size * 7);
                    arg0_rowArr_1_1[0] = arg0_tmpConverter_0_1_1_7.f;
                    //diff = 1
                    // access_idx = 9
                    ops::hls::DataConv arg0_tmpConverter_2_1_1_0;
                    arg0_tmpConverter_2_1_1_0.i = arg0_widenStencilValues[4].range(s_datatype_size * (0 + 1) - 1, s_datatype_size * 0);
                    arg0_rowArr_1_1[9] = arg0_tmpConverter_2_1_1_0.f;
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
                            || (k < stencilConfig.lower_limit[2]) 
                            || (k >= stencilConfig.upper_limit[2])
                    );

    #ifdef DEBUG_LOG
                    printf("[DEBUG][INTERNAL][jac3D_kernel_stencil_PE_%d] index=(%d, %d, %d), lowerbound=(%d, %d, %d), upperbound=(%d, %d, %d), neg_cond=%d\n", m_PEId, index, j, k,
                                stencilConfig.lower_limit[0], stencilConfig.lower_limit[1], stencilConfig.lower_limit[2], 
                                stencilConfig.upper_limit[0], stencilConfig.upper_limit[1], stencilConfig.upper_limit[2], neg_cond);

    #endif

                    stencil_type arg1_result;

                    kernel_jac3D_kernel_stencil_core(
                            arg0_rowArr_1_0[x + 1],
                            arg0_rowArr_0_1[x + 1],
                            arg0_rowArr_1_1[x + 0],
                            arg0_rowArr_1_1[x + 1],
                            arg0_rowArr_1_1[x + 2],
                            arg0_rowArr_2_1[x + 1],
                            arg0_rowArr_1_2[x + 1],
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
                        arg1_tmpConvWrite.f = arg0_rowArr_1_1[x + 1];
                    }
                // *** rw convertions ***

                    arg1_update_val.range(s_datatype_size * (x + 1) - 1, x * s_datatype_size) = arg1_tmpConvWrite.i;

                }

                write:
                {
                    bool cond_write = (k >= 0);

                    if (cond_write)
                    {
    #ifdef DEBUG_LOG
                        printf("[DEBUG][INTERNAL][jac3D_kernel_stencil_PE_%d] --------------------------------------------------------\n\n", m_PEId);

                        printf("[DEBUG][INTERNAL][jac3D_kernel_stencil_PE_%d] wirte values arg1: (", m_PEId);
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
    } 
};

void kernel_jac3D_kernel_stencil_PE(const short& PEId, const ops::hls::StencilConfigCore& stencilConfig,
            //u
    widen_stream_jac3D_kernel_stencil_0_dt& arg0_rd_buffer,
            //u2
    widen_stream_jac3D_kernel_stencil_1_dt& arg1_wr_buffer        
)
{
    Stencil_jac3D_kernel_stencil stencil;

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
