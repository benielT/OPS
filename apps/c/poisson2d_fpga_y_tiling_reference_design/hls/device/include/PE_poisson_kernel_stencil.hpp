// Auto-generated at 2026-11-26 15:29:07.524842 by ops-translator

#ifndef __LOOP_PE_POISSON_KERNEL_STENCIL__
#define __LOOP_PE_POISSON_KERNEL_STENCIL__
#include <ops_hls_kernel_support.h>

#include "../../common/include/common_config.hpp"
#ifndef __SYNTHESIS__
#include <vector>
#endif

static constexpr unsigned short read_num_points_poisson_kernel_stencil = 5;
static constexpr unsigned short read_stencil_size_poisson_kernel_stencil = 3;
static constexpr unsigned short read_stencil_dim_poisson_kernel_stencil = 2;

static constexpr unsigned short write_num_points_poisson_kernel_stencil = 1;
static constexpr unsigned short write_stencil_size_poisson_kernel_stencil = 1;
static constexpr unsigned short write_stencil_dim_poisson_kernel_stencil = 2;

static constexpr unsigned short vector_factor_poisson_kernel_stencil_0 =
    (vector_factor) * 1;
static constexpr unsigned short axis_data_width_poisson_kernel_stencil_0 =
    (vector_factor_poisson_kernel_stencil_0 * data_width);
typedef ap_uint<axis_data_width_poisson_kernel_stencil_0>
    widen_poisson_kernel_stencil_0_dt;
static constexpr unsigned short vector_factor_poisson_kernel_stencil_1 =
    (vector_factor) * 1;
static constexpr unsigned short axis_data_width_poisson_kernel_stencil_1 =
    (vector_factor_poisson_kernel_stencil_1 * data_width);
typedef ap_uint<axis_data_width_poisson_kernel_stencil_1>
    widen_poisson_kernel_stencil_1_dt;

typedef ::hls::stream<widen_poisson_kernel_stencil_0_dt>
    widen_stream_poisson_kernel_stencil_0_dt;
typedef ::hls::stream<widen_poisson_kernel_stencil_1_dt>
    widen_stream_poisson_kernel_stencil_1_dt;

/*
    ArgDat(id=0,
   loc=/home/x_thileeb/repos/ops-hls-batching-tiling-artifacts/OPS/apps/c/poisson2d_fpga_y_tiling_reference_design/poisson.cpp/361:21,
   access_type=AccessType.OPS_READ, opt=True, dat_id=0, global_dat_id=-1,
   stencil_id=S2D_00_P10_M10_0P1_0M1, is_read_only?=False) ArgDat(id=1,
   loc=/home/x_thileeb/repos/ops-hls-batching-tiling-artifacts/OPS/apps/c/poisson2d_fpga_y_tiling_reference_design/poisson.cpp/362:21,
   access_type=AccessType.OPS_WRITE, opt=True, dat_id=1, global_dat_id=-1,
   stencil_id=S2D_00, is_read_only?=False)
*/
inline void kernel_poisson_kernel_stencil_core(const stencil_type &reg_0_0,
                                               const stencil_type &reg_0_1,
                                               const stencil_type &reg_0_2,
                                               const stencil_type &reg_0_3,
                                               const stencil_type &reg_0_4,
                                               stencil_type &reg_1_0) {
#ifdef DEBUG_LOG
  printf("[KERNEL_INTERNAL_CORE]|%s| starting kernel core: "
         "kernel_poisson_kernel_stencil_core\n",
         __func__);
#endif

  float tmp1_1 = reg_0_1 + reg_0_3;
  float tmp1_2 = reg_0_4 + reg_0_0;
  float tmp1 = tmp1_1 + tmp1_2;
  float tmp2 = ldexpf(reg_0_2, -1);
  float tmp3 = ldexpf(tmp1, -3);
  reg_1_0 = tmp2 + tmp3;

#ifdef DEBUG_LOG
  printf("[KERNEL_INTERNAL_CORE]|%s| read_val - reg_0_0: %f \n", __func__,
         reg_0_0);
  printf("[KERNEL_INTERNAL_CORE]|%s| read_val - reg_0_1: %f \n", __func__,
         reg_0_1);
  printf("[KERNEL_INTERNAL_CORE]|%s| read_val - reg_0_2: %f \n", __func__,
         reg_0_2);
  printf("[KERNEL_INTERNAL_CORE]|%s| read_val - reg_0_3: %f \n", __func__,
         reg_0_3);
  printf("[KERNEL_INTERNAL_CORE]|%s| read_val - reg_0_4: %f \n", __func__,
         reg_0_4);
  printf("[KERNEL_INTERNAL_CORE]|%s| write_val - reg_1_0: %f \n", __func__,
         reg_1_0);
#endif

#ifdef DEBUG_LOG
  printf("[KERNEL_INTERNAL_CORE]|%s| exiting: "
         "kernel_poisson_kernel_stencil_core\n",
         __func__);
#endif
}

class Stencil_poisson_kernel_stencil
    : public ops::hls::StencilCoreV2<
          stencil_type, read_num_points_poisson_kernel_stencil, vector_factor,
          ops::hls::CoefTypes::CONST_COEF,
          read_stencil_size_poisson_kernel_stencil,
          read_stencil_dim_poisson_kernel_stencil, true> {
  using ops::hls::StencilCoreV2<
      stencil_type, read_num_points_poisson_kernel_stencil, vector_factor,
      ops::hls::CoefTypes::CONST_COEF, read_stencil_size_poisson_kernel_stencil,
      read_stencil_dim_poisson_kernel_stencil, true>::m_stencilConfig;

public:
  void stencilRun(widen_stream_poisson_kernel_stencil_0_dt &arg0_rd_buffer,
                  widen_stream_poisson_kernel_stencil_1_dt &arg1_wr_buffer) {
    ::ops::hls::StencilConfigCoreSingleTile stencilConfig = m_stencilConfig;
#pragma HLS ARRAY_PARTITION variable = stencilConfig.tile_size dim = 1 complete
// #pragma HLS ARRAY_PARTITION variable = stencilConfig.is_tiled dim = 1
// complete
#pragma HLS ARRAY_PARTITION variable = stencilConfig.is_first dim = 1 complete
#pragma HLS ARRAY_PARTITION variable = stencilConfig.is_last dim = 1 complete
    // #pragma HLS ARRAY_PARTITION variable = stencilConfig.last_tile_size dim =
    // 1 complete #pragma HLS ARRAY_PARTITION variable =
    // stencilConfig.tile_count dim = 1 complete #pragma HLS ARRAY_PARTITION
    // variable = stencilConfig.grid_size dim = 1 complete
    //  const unsigned short peId = m_PEId;

    const unsigned short span_x = 2;
    const unsigned short half_span_x = 1;
    const unsigned short true_tile_size_x = m_stencilConfig.tile_size[0]
                                            << shift_bits;
    const bool is_first_x = m_stencilConfig.is_first[0];
    const bool is_last_x = m_stencilConfig.is_last[0];

    // const unsigned short adj_half_span_x = m_stencilConfig.tile_count[0] ==
    // 1? half_span_x : half_span_x * (m_PEId + 1);
    const unsigned short adj_half_span_x_lower =
        is_first_x ? half_span_x : half_span_x * (m_PEId + 1);
    const unsigned short adj_half_span_x_upper =
        is_last_x ? half_span_x : half_span_x * (m_PEId + 1);

    // static limits are set outside the tile loops
    const unsigned short lower_limit_x = adj_half_span_x_lower;
    // const unsigned short min_limit_x_const =
    // m_stencilConfig.last_tile_upper_limit_x < true_tile_size_x ?
    // m_stencilConfig.last_tile_upper_limit_x : true_tile_size_x;
    // //(m_stencilConfig.last_tile_upper_limit_x - adj_half_span_x) <
    // adj_half_span_x ? adj_half_span_x :
    // (m_stencilConfig.last_tile_upper_limit_x - adj_half_span_x); const
    // unsigned short upper_limit_x = (min_limit_x_const -
    // adj_half_span_x_upper) < adj_half_span_x_upper ? adj_half_span_x_upper :
    // min_limit_x_const - adj_half_span_x_upper;
    const unsigned short upper_limit_x =
        m_stencilConfig.is_last[0] ? m_stencilConfig.last_tile_upper_limit_x - 1
                                   : true_tile_size_x - adj_half_span_x_upper;
    const unsigned short lower_limit_y = half_span_x;
    const unsigned short upper_limit_y =
        m_stencilConfig.outer_loop_limit - span_x;
    // const unsigned short lower_limit_x = m_stencilConfig.lower_limit[0];
    // const unsigned short lower_limit_y = m_stencilConfig.lower_limit[1];
    // const unsigned short lower_limit_z = m_stencilConfig.lower_limit[2];
    // const unsigned short upper_limit_x = m_stencilConfig.upper_limit[0];
    // const unsigned short upper_limit_y = m_stencilConfig.upper_limit[1];
    // const unsigned short upper_limit_z = m_stencilConfig.upper_limit[2];

    // read_origin_wide_diff_x: 0, read_origin_wide_diff: (0,1,0)
    //  *** counters definitions ****
    short i = -1;
    short j = -1;

    //  *** stencil description and read & write point definitions  ****
    /*
    WindowBufferDiscriptor(widen_stencil=Stencil(id=-1, dim=2,
    stencil_ptr='read_stencil_widen', num_points=5, points=[Point(x=1, y=0,
    z=0), Point(x=0, y=1, z=0), Point(x=1, y=1, z=0), Point(x=2, y=1, z=0),
    Point(x=1, y=2, z=0)], base_point=Point(x=1, y=1, z=0),
    stencil_size=Point(x=3, y=3, z=1), d_m=Point(x=-0.75, y=1, z=0),
    d_p=Point(x=1.0, y=1, z=0), row_discriptors=[StencilRowDiscriptor(row_id=(0,
    0), base_point=Point(x=1, y=1, z=0), row_points=[Point(x=1, y=0, z=0)]),
    StencilRowDiscriptor(row_id=(1, 0), base_point=Point(x=1, y=1, z=0),
    row_points=[Point(x=0, y=1, z=0), Point(x=1, y=1, z=0), Point(x=2, y=1,
    z=0)]), StencilRowDiscriptor(row_id=(2, 0), base_point=Point(x=1, y=1, z=0),
    row_points=[Point(x=1, y=2, z=0)])], stride=[], read_origin_diff=Point(x=0,
    y=1, z=0)), window_buffers=[WindowBuffer(name='buf_r0_1_p0',
    buffer_type=<BufferType.LINE_BUFF: 0>, read_point=Point(x=0, y=1, z=0),
    write_point=Point(x=1, y=0, z=0), is_read_write_aligned=False),
    WindowBuffer(name='buf_r1_2_p0', buffer_type=<BufferType.LINE_BUFF: 0>,
    read_point=Point(x=1, y=2, z=0), write_point=Point(x=2, y=1, z=0),
    is_read_write_aligned=False)], chains=[(0, WindowBuffer(name='buf_r0_1_p0',
    buffer_type=<BufferType.LINE_BUFF: 0>, read_point=Point(x=0, y=1, z=0),
    write_point=Point(x=1, y=0, z=0), is_read_write_aligned=False)), (1, 2), (2,
    3), (3, WindowBuffer(name='buf_r1_2_p0', buffer_type=<BufferType.LINE_BUFF:
    0>, read_point=Point(x=1, y=2, z=0), write_point=Point(x=2, y=1, z=0),
    is_read_write_aligned=False)), (WindowBuffer(name='buf_r0_1_p0',
    buffer_type=<BufferType.LINE_BUFF: 0>, read_point=Point(x=0, y=1, z=0),
    write_point=Point(x=1, y=0, z=0), is_read_write_aligned=False), 1), (4,
    'read_val'), (WindowBuffer(name='buf_r1_2_p0',
    buffer_type=<BufferType.LINE_BUFF: 0>, read_point=Point(x=1, y=2, z=0),
    write_point=Point(x=2, y=1, z=0), is_read_write_aligned=False), 4)],
    point_to_widen_map={Point(x=1, y=0, z=0): Point(x=1, y=0, z=0), Point(x=0,
    y=1, z=0): Point(x=0, y=1, z=0), Point(x=1, y=1, z=0): Point(x=1, y=1, z=0),
    Point(x=2, y=1, z=0): Point(x=2, y=1, z=0), Point(x=1, y=2, z=0): Point(x=1,
    y=2, z=0)})
    */
    // Stencil: S2D_00_P10_M10_0P1_0M1
    //      |- point: (1,0,0)
    //      |- point: (0,1,0)
    //      |- point: (1,1,0)
    //      |- point: (2,1,0)
    //      `- point: (1,2,0)
    // read point: (0,1,0), write point: (1,0,0)
    unsigned short S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_rd = 0;
    unsigned short S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_wr = 1;
    // read point: (1,2,0), write point: (2,1,0)
    unsigned short S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_rd = 0;
    unsigned short S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_wr = 1;

    //  *** iteration limit definition ****
    unsigned int iter_limit =
        m_stencilConfig.outer_loop_limit * m_stencilConfig.tile_size[0];

    //  *** data read write boundary definitions ****
    unsigned int S2D_00_P10_M10_0P1_0M1_read_lb_itr = 0;
    unsigned int S2D_00_P10_M10_0P1_0M1_read_ub_itr =
        m_stencilConfig.tile_size[1] * m_stencilConfig.tile_size[0];

    /*
        unsigned int read_lb_itr = 0;
        unsigned int read_ub_itr = m_stencilConfig.grid_size[1] *
       m_stencilConfig.grid_size[0];
    */
    //  *** Read & write widen temporaries ****
    // arg0(u)
    widen_poisson_kernel_stencil_0_dt arg0_read_val = 0;
    // arg1(u2)
    widen_poisson_kernel_stencil_1_dt arg1_update_val;
    // 2
    //  *** widen stencil values holder & window buffers ****

    // arg0(u)
#ifdef __SYNTHESIS__
    widen_poisson_kernel_stencil_0_dt
        arg0_widenStencilValues[read_num_points_poisson_kernel_stencil];
#pragma HLS ARRAY_PARTITION variable = arg0_widenStencilValues dim = 1 complete
#else
    std::vector<widen_poisson_kernel_stencil_0_dt> arg0_widenStencilValues_vec(
        read_num_points_poisson_kernel_stencil);
    widen_poisson_kernel_stencil_0_dt *arg0_widenStencilValues =
        arg0_widenStencilValues_vec.data();
#endif
    // Max grid_size: (8192, 300)

#ifndef __SYNTHESIS__
    // tile_size_x = 8192, half_span_x: 0, vector_factor: 8, interleave_div: 1
    std::vector<widen_poisson_kernel_stencil_0_dt> arg0_buf_r0_1_p0_vec(1024);
    widen_poisson_kernel_stencil_0_dt *arg0_buf_r0_1_p0 =
        arg0_buf_r0_1_p0_vec.data();
    // tile_size_x = 8192, half_span_x: 0, vector_factor: 8, interleave_div: 1
    std::vector<widen_poisson_kernel_stencil_0_dt> arg0_buf_r1_2_p0_vec(1024);
    widen_poisson_kernel_stencil_0_dt *arg0_buf_r1_2_p0 =
        arg0_buf_r1_2_p0_vec.data();
#else
    // tile_size_x = 8192, half_span_x: 0, vector_factor: 8, interleave_div: 1
    widen_poisson_kernel_stencil_0_dt arg0_buf_r0_1_p0[1024];
#pragma HLS BIND_STORAGE variable = arg0_buf_r0_1_p0 type = ram_s2p impl =     \
    URAM latency = 2
    // curr SIS_id = 0
    // next SIS_id = 1
    // tile_size_x = 8192, half_span_x: 0, vector_factor: 8, interleave_div: 1
    widen_poisson_kernel_stencil_0_dt arg0_buf_r1_2_p0[1024];
#pragma HLS BIND_STORAGE variable = arg0_buf_r1_2_p0 type = ram_s2p impl =     \
    BRAM latency = 2
    // curr SIS_id = 1
    // next SIS_id = 0
#endif
    stencil_type arg0_rowArr_0_0[vector_factor + span_x];
#pragma HLS ARRAY_PARTITION variable = arg0_rowArr_0_0 dim = 1 complete
    stencil_type arg0_rowArr_1_0[vector_factor + span_x];
#pragma HLS ARRAY_PARTITION variable = arg0_rowArr_1_0 dim = 1 complete
    stencil_type arg0_rowArr_2_0[vector_factor + span_x];
#pragma HLS ARRAY_PARTITION variable = arg0_rowArr_2_0 dim = 1 complete

    const short cond_x_val = m_stencilConfig.tile_size[0] - 1;
    const short cond_y_val = m_stencilConfig.outer_loop_limit - 1;

#ifdef DEBUG_LOG
    printf("[DEBUG][INTERNAL][%s:%d] initial values, "
           "tile_size_x(%d), grid_size_y(%d), "
           "iter_limit(%d), m_stencilConfig.outer_loop_limit:%d \n",
           __func__, m_PEId, m_stencilConfig.tile_size[0],
           m_stencilConfig.tile_size[1],

           iter_limit, m_stencilConfig.outer_loop_limit);
#endif

    for (unsigned int itr = 0; itr < iter_limit; itr++) {
#pragma HLS PIPELINE II = 1
      unsigned int reg_itr = register_it(itr);

    spc_temp_blocking_read: {
      bool cond_x_terminate = (i == cond_x_val ? true : false);
      bool cond_y_terminate = (j == cond_y_val ? true : false);

#ifdef DEBUG_LOG
      printf("[DEBUG][INTERNAL][poisson_kernel_stencil_PE_%d] loop params "
             "before update i(%d), "
             "j(%d), "
             "S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_rd: %d, "
             "S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_wr: %d, "
             "S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_rd: %d, "
             "S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_wr: %d, "
             "S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_rd_cont_buf_len: %d, "
             "S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_rd_cont_buf_len: %d, "
             "reg_itr(%d)\n",
             m_PEId, i, j, S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_rd,
             S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_wr,
             S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_rd,
             S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_wr,
             (m_stencilConfig.tile_size[0] - 1),
             (m_stencilConfig.tile_size[0] - 1), reg_itr);
#endif
      if (cond_x_terminate)
        i = 0;
      else
        i++;
      if (cond_x_terminate && cond_y_terminate)
        j = 0;
      else if (cond_x_terminate)
        j++;

      bool S2D_00_P10_M10_0P1_0M1_read_cond =
          (reg_itr < S2D_00_P10_M10_0P1_0M1_read_ub_itr) and
          (reg_itr >= S2D_00_P10_M10_0P1_0M1_read_lb_itr);
      /*    bool read_cond  =  (reg_itr < read_ub_itr) and (reg_itr >=
       * read_lb_itr); */

      if (S2D_00_P10_M10_0P1_0M1_read_cond) {
        arg0_read_val = arg0_rd_buffer.read();
      }

      /*if (read_cond)
      {
          arg0_read_val = arg0_rd_buffer.read();
      }*/

      arg0_widenStencilValues[0] =
          arg0_buf_r0_1_p0[S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_wr];
      arg0_widenStencilValues[1] = arg0_widenStencilValues[2];
      arg0_widenStencilValues[2] = arg0_widenStencilValues[3];
      arg0_widenStencilValues[3] =
          arg0_buf_r1_2_p0[S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_wr];
      arg0_buf_r0_1_p0[S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_rd] =
          arg0_widenStencilValues[1];
      arg0_widenStencilValues[4] = arg0_read_val;
      arg0_buf_r1_2_p0[S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_rd] =
          arg0_widenStencilValues[4];

      bool cond_end_of_line_buff_S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_rd =
          S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_rd >= m_stencilConfig.line_diff;
      bool cond_end_of_line_buff_S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_wr =
          S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_wr >= m_stencilConfig.line_diff;

      if (cond_end_of_line_buff_S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_rd)
        S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_rd = 0;
      else
        S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_rd++;

      if (cond_end_of_line_buff_S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_wr)
        S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_wr = 0;
      else
        S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_wr++;

      bool cond_end_of_line_buff_S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_rd =
          S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_rd >= m_stencilConfig.line_diff;
      bool cond_end_of_line_buff_S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_wr =
          S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_wr >= m_stencilConfig.line_diff;

      if (cond_end_of_line_buff_S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_rd)
        S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_rd = 0;
      else
        S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_rd++;

      if (cond_end_of_line_buff_S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_wr)
        S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_wr = 0;
      else
        S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_wr++;

#ifdef DEBUG_LOG
      printf("[DEBUG][INTERNAL][poisson_kernel_stencil_PE_%d] loop params "
             "after update i: %d, "
             "j: %d, "
             "S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_rd(%d), "
             "S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_wr(%d), "
             "S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_rd(%d), "
             "S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_wr(%d), "
             "(m_stencilConfig.tile_size[0] - 1): %d, "
             "(m_stencilConfig.tile_size[0] - 1): %d, "
             "reg_itr: %d\n",
             m_PEId, i, j, S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_rd,
             S2D_00_P10_M10_0P1_0M1_buf_r0_1_p0_wr,
             S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_rd,
             S2D_00_P10_M10_0P1_0M1_buf_r1_2_p0_wr,
             (m_stencilConfig.tile_size[0] - 1),
             (m_stencilConfig.tile_size[0] - 1), reg_itr);

      printf("[DEBUG][INTERNAL][poisson_kernel_stencil_PE_%d] "
             "--------------------------------------------------------\n\n",
             m_PEId);

      printf(
          "[DEBUG][INTERNAL][poisson_kernel_stencil_PE_%d] read values arg0: (",
          m_PEId);
      for (int ri = 0; ri < vector_factor; ri++) {
        ops::hls::DataConv tmpConverter;
        tmpConverter.i = arg0_read_val.range((ri + 1) * s_datatype_size - 1,
                                             ri * s_datatype_size);
        printf("%f ", tmpConverter.f);
      }
      printf(")\n");
#endif
    }

    vec2arr:
      for (unsigned short x = 0; x < vector_factor; x++) {
#pragma HLS UNROLL factor = vector_factor
        ops::hls::DataConv arg0_tmpConverter_0_0;
        arg0_tmpConverter_0_0.i = arg0_widenStencilValues[0].range(
            s_datatype_size * (x + 1) - 1, x * s_datatype_size);
        arg0_rowArr_0_0[x + half_span_x] = arg0_tmpConverter_0_0.f;
        ops::hls::DataConv arg0_tmpConverter_1_0;
        arg0_tmpConverter_1_0.i = arg0_widenStencilValues[2].range(
            s_datatype_size * (x + 1) - 1, x * s_datatype_size);
        arg0_rowArr_1_0[x + half_span_x] = arg0_tmpConverter_1_0.f;
        ops::hls::DataConv arg0_tmpConverter_2_0;
        arg0_tmpConverter_2_0.i = arg0_widenStencilValues[4].range(
            s_datatype_size * (x + 1) - 1, x * s_datatype_size);
        arg0_rowArr_2_0[x + half_span_x] = arg0_tmpConverter_2_0.f;
      }
    vec2arr_rest: {
      // diff = -1
      //  access_idx = 0
      ops::hls::DataConv arg0_tmpConverter_0_1_0_7;
      arg0_tmpConverter_0_1_0_7.i = arg0_widenStencilValues[1].range(
          s_datatype_size * (7 + 1) - 1, s_datatype_size * 7);
      arg0_rowArr_1_0[0] = arg0_tmpConverter_0_1_0_7.f;
      // diff = 1
      //  access_idx = 9
      ops::hls::DataConv arg0_tmpConverter_2_1_0_0;
      arg0_tmpConverter_2_1_0_0.i = arg0_widenStencilValues[3].range(
          s_datatype_size * (0 + 1) - 1, s_datatype_size * 0);
      arg0_rowArr_1_0[9] = arg0_tmpConverter_2_1_0_0.f;
    }
    process:
      for (unsigned short x = 0; x < vector_factor; x++) {
#pragma HLS UNROLL factor = vector_factor
        short index = (i << shift_bits) + x;
        bool neg_cond =
            register_it((index < lower_limit_x) || (index >= upper_limit_x) ||
                        (j < lower_limit_y) || (j >= upper_limit_y));

#ifdef DEBUG_LOG
        printf("[DEBUG][INTERNAL][poisson_kernel_stencil_PE_%d] index=(%d, "
               "%d), lowerbound=(%d, %d), upperbound=(%d, %d), neg_cond=%d\n",
               m_PEId, index, j, lower_limit_x, lower_limit_y, upper_limit_x,
               upper_limit_y, neg_cond);
#endif

        stencil_type arg1_result;

        kernel_poisson_kernel_stencil_core(
            arg0_rowArr_0_0[x + 1], arg0_rowArr_1_0[x + 0],
            arg0_rowArr_1_0[x + 1], arg0_rowArr_1_0[x + 2],
            arg0_rowArr_2_0[x + 1], arg1_result);

        ops::hls::DataConv arg1_tmpConvWrite;

        if (not neg_cond) {
          arg1_tmpConvWrite.f = arg1_result;
        } else {

          // [1, 0]
          // dat_id: 1, dat: u2, swap_id: 0, swap_dat: u
          arg1_tmpConvWrite.f = arg0_rowArr_1_0[x + 1];
        }
        // *** rw convertions ***

        arg1_update_val.range(s_datatype_size * (x + 1) - 1,
                              x * s_datatype_size) = arg1_tmpConvWrite.i;
      }
    write: {
      bool cond_write = (j >= 0);

      if (cond_write) {
#ifdef DEBUG_LOG
        printf("[DEBUG][INTERNAL][poisson_kernel_stencil_PE_%d] "
               "--------------------------------------------------------\n\n",
               m_PEId);

        printf("[DEBUG][INTERNAL][poisson_kernel_stencil_PE_%d] wirte values "
               "arg1: (",
               m_PEId);
        for (int ri = 0; ri < vector_factor; ri++) {
          ops::hls::DataConv tmpConverter;
          tmpConverter.i = arg1_update_val.range((ri + 1) * s_datatype_size - 1,
                                                 ri * s_datatype_size);
          printf("%f ", tmpConverter.f);
        }
        printf(")\n");
#endif
        arg1_wr_buffer << arg1_update_val;
      }
    }
    }
  }
};

static void kernel_poisson_kernel_stencil_PE(
    const unsigned short &PEId_offset, const unsigned short &PEId_i,
    const ops::hls::StencilConfigCoreSingleTile &stencilConfig,
    // u
    widen_stream_poisson_kernel_stencil_0_dt &arg0_rd_buffer,
    // u2
    widen_stream_poisson_kernel_stencil_1_dt &arg1_wr_buffer) {

  Stencil_poisson_kernel_stencil stencil;

  short PEId = PEId_offset + PEId_i;

#ifdef DEBUG_LOG
  printf(
      "[KERNEL_DEBUG][%d]|%s| stencil config gridSize: %d (xblocks), %d, %d\n",
      PEId, __func__, stencilConfig.tile_size[0], stencilConfig.tile_size[1],
      stencilConfig.tile_size[2]);
#endif

  stencil.setConfig(PEId, stencilConfig);

#ifdef DEBUG_LOG
  printf("[KERNEL_DEBUG][%d]|%s| starting stencil kernel PE\n", PEId, __func__);
#endif

  stencil.stencilRun(arg0_rd_buffer, arg1_wr_buffer

  );

#ifdef DEBUG_LOG
  printf("[KERNEL_DEBUG][%d]|%s| Ending stencil kernel PE\n", PEId, __func__);
#endif
}

#endif // __LOOP_PE_POISSON_KERNEL_STENCIL__
