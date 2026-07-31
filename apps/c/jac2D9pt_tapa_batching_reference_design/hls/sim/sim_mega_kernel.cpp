#include <sim_mega_kernel.hpp>


void sim_mega_kernel(const unsigned int num_beats,
    const unsigned int num_axis_trans,
    const unsigned int loopback_itr,
    const unsigned int outer_itr,
    const unsigned short stencilConfig_grid_size_0,
    const unsigned short stencilConfig_grid_size_1,
    const unsigned short stencilConfig_dim,
    const unsigned int stencilConfig_total_itr,
    const unsigned short stencilConfig_lower_limit_0,
    const unsigned short stencilConfig_lower_limit_1,
    const unsigned short stencilConfig_upper_limit_0,
    const unsigned short stencilConfig_upper_limit_1,
    const unsigned short stencilConfig_outer_loop_limit,
    const unsigned short stencilConfig_batch_size,
    // u
   ::tapa::mmap<::tapa::vec_t<float,mem_vector_factor>> arg0,
    // u2
   ::tapa::mmap<::tapa::vec_t<float,mem_vector_factor>> arg1) {

    ::tapa::stream<::tapa::vec_t<float,vector_factor>,4,4096> arg0_axis("arg0_axis_stream");
    ::tapa::stream<::tapa::vec_t<float,vector_factor>,4,4096> arg1_axis("arg1_axis_stream");
    ::tapa::stream<::tapa::vec_t<float,vector_factor>,2,4096> arg0_arg1_inter_PE_axis("arg0_arg1_inter_PE_axis_stream");

    ::tapa::task()
    .invoke(datamover_outerloop_0, num_beats, num_axis_trans, outer_itr, arg0, arg1, arg0_axis, arg1_axis)
    .invoke(kernel_outerloop_0, 0, outer_itr, stencilConfig_grid_size_0, stencilConfig_grid_size_1, stencilConfig_dim, stencilConfig_total_itr, stencilConfig_lower_limit_0, stencilConfig_lower_limit_1, stencilConfig_upper_limit_0, stencilConfig_upper_limit_1, stencilConfig_outer_loop_limit, stencilConfig_batch_size, arg0_axis, arg0_arg1_inter_PE_axis)
    .invoke(kernel_outerloop_0, 1, outer_itr, stencilConfig_grid_size_0, stencilConfig_grid_size_1, stencilConfig_dim, stencilConfig_total_itr, stencilConfig_lower_limit_0, stencilConfig_lower_limit_1, stencilConfig_upper_limit_0, stencilConfig_upper_limit_1, stencilConfig_outer_loop_limit, stencilConfig_batch_size, arg0_arg1_inter_PE_axis, arg1_axis);

    // ::tapa::task()
    //     .invoke(datamover_outerloop_0, num_beats, num_axis_trans, outer_itr, arg0, arg1, arg0_axis, arg1_axis)
    //     .invoke(kernel_outerloop_0, 0, outer_itr, stencilConfig_grid_size_0, stencilConfig_grid_size_1, stencilConfig_dim, stencilConfig_total_itr, stencilConfig_lower_limit_0, stencilConfig_lower_limit_1, stencilConfig_upper_limit_0, stencilConfig_upper_limit_1, stencilConfig_outer_loop_limit, stencilConfig_batch_size, arg0_axis, arg1_axis);
}