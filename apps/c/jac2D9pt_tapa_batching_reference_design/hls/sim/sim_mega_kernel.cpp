#include <sim_mega_kernel.hpp>


void sim_mega_kernel(const unsigned int num_beats,
    const unsigned int num_axis_trans,
    const unsigned int loopback_itr,
    const unsigned int outer_iter,
    const unsigned int stencilConfig_total_itr,
    // u
   ::tapa::mmap<::tapa::vec_t<float,mem_vector_factor>> arg0,
    // u2
   ::tapa::mmap<::tapa::vec_t<float,mem_vector_factor>> arg1) {

    ::tapa::stream<::tapa::vec_t<float,vector_factor>,4,4096> arg0_axis("arg0_axis_stream");
    ::tapa::stream<::tapa::vec_t<float,vector_factor>,4,4096> arg1_axis("arg1_axis_stream");

    ::tapa::task()
    .invoke(datamover_outerloop_0, num_beats, num_axis_trans, outer_iter, arg0, arg1, arg0_axis, arg1_axis)
    .invoke(kernel_outerloop_0, outer_iter, stencilConfig_total_itr, arg0_axis, arg1_axis);
}