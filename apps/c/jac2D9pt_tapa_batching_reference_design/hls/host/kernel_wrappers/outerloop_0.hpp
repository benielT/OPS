// Auto-generated at 2026-07-06 21:11:33.478567 by ops-translator

#pragma once 
#if !defined(TAPA_SW_EMU) && !defined(TAPA_HW_EMU)
#include <ops_hls_rt_support.h>
#else 
#include <ops_tapa_rt_support.h>
#endif
#include "../../common/include/common_config.hpp"

#if defined(TAPA_SW_EMU)
#include "../../sim/sim_mega_kernel.hpp"
#include <gflags/gflags.h>
#include <tapa.h>

#endif

#if !defined(TAPA_SW_EMU) && !defined(TAPA_HW_EMU)
class KernelWrapper_outerloop_0 : public ops::hls::Kernel
{
public:
    KernelWrapper_outerloop_0():
            Kernel("isl0"),
            m_kernelName("kernel_outerloop_0"),
            m_datamoverName("datamover_outerloop_0")
    {
        cl_int err;
        OCL_CHECK(err, m_kernel_0 = cl::Kernel(m_fpga->getProgram(), m_kernelName.c_str(), &err));
        OCL_CHECK(err, m_kernel_1 = cl::Kernel(m_fpga->getProgram(), m_kernelName.c_str(), &err));
        OCL_CHECK(err, m_datamover = cl::Kernel(m_fpga->getProgram(), m_datamoverName.c_str(), &err));    
    }

    void run(ops::hls::AccessRange& range, unsigned int outer_iter,
            ops::hls::Grid<float>& arg0,
            ops::hls::Grid<float>& arg1
    )
    {
        cl_int err;
    
        ops::hls::SizeType read_stencil_d_m = { 1, 1, 0 };
        ops::hls::SizeType read_stencil_d_p = { 1, 1, 0 };
        ops::hls::SizeType write_stencil_d_m = {0,0,0};
        ops::hls::SizeType write_stencil_d_p = {0,0,0};
        auto read_stencilConfig = getStencilConfig(arg0.originalProperty, range, vector_factor, mem_vector_factor, read_stencil_d_m, read_stencil_d_p);
        ops::hls::AccessRange read_range;
        getAdjustedRange(arg0.originalProperty, range, read_range, read_stencil_d_m, read_stencil_d_p);
        ops::hls::MemConfig memconfig;
        ops::hls::genMemConfig<mem_data_width, axis_data_width, data_width>(arg0.originalProperty.grid_size, read_range, memconfig);
        const unsigned int num_beats = memconfig.total_xblocks * arg0.originalProperty.batch_size;
        const unsigned int num_of_pkts_per_beat = mem_data_width / axis_data_width;
        const unsigned int num_of_pkts = num_beats * num_of_pkts_per_beat;
#ifdef OPS_TILING
        ops::hls::SizeType original_gridSize_copy = {arg0.originalProperty.grid_size[0], arg0.originalProperty.grid_size[1], arg0.originalProperty.grid_size[2]};
        ops::hls::AccessRange read_range_copy;
        read_range_copy.start[0] = read_range.start[0];
        read_range_copy.start[1] = read_range.start[1];
        read_range_copy.start[2] = read_range.start[2];
        read_range_copy.end[0] = read_range.end[0];
        read_range_copy.end[1] = read_range.end[1];
        read_range_copy.end[2] = read_range.end[2];
        read_range_copy.dim = read_range.dim;
        ops::hls::SizeType2d tile_size = {m_fpga->getOPSTileSizeX(), 1};
        ops::hls::SizeType2d overlap_size = {get_overlap_size<2, 1, 1, mem_vector_factor>(), 
0};
        ops::hls::SizeType2d tile_count;
        ops::hls::SizeType2d effective_tile_size;
        ops::hls::SizeType2d last_tile_size;
        unsigned int total_xblocks_widen;
        unsigned short last_tile_upper_limit_x;
    #ifdef DEBUG_LOG
        std::cout << "[DEBUG][KERNEL_WRAPPER]|" << __func__ << "| TILE PARAMETERS BEFORE TILE METADATA GENERATION" << std::endl;
        std::cout << "[DEBUG][KERNEL_WRAPPER]|" << __func__ << "| original_gridSize_copy: (" 
                  << original_gridSize_copy[0] << ", " << original_gridSize_copy[1] << ", " << original_gridSize_copy[2] << ")\n";
        std::cout << "[DEBUG][KERNEL_WRAPPER]|" << __func__ << "| read_range_copy: ( start: (" 
                  << read_range_copy.start[0] << ", " << read_range_copy.start[1] << ", " << read_range_copy.start[2] << "), end: ("
                  << read_range_copy.end[0] << ", " << read_range_copy.end[1] << ", " << read_range_copy.end[2] << "), dim: " << read_range_copy.dim << ")\n";
        std::cout << "[DEBUG][KERNEL_WRAPPER]|" << __func__ << "| tile_size: (" 
                  << tile_size[0] << ", " << tile_size[1] << ")\n";
        std::cout << "[DEBUG][KERNEL_WRAPPER]|" << __func__ << "| overlap_size: (" 
                  << overlap_size[0] << ", " << overlap_size[1] << ")\n";
    #endif
        ops::hls::genTileMetadata<mem_data_width, data_width, 1>(original_gridSize_copy, read_range_copy, tile_size, overlap_size,
                effective_tile_size, last_tile_size, tile_count, last_tile_upper_limit_x, total_xblocks_widen);
        // kernel tile sizes need to be adusted according to the axis_data_width
        ops::hls::SizeType2d kernel_tile_size = {tile_size[0] * num_of_pkts_per_beat, tile_size[1]};
        ops::hls::SizeType2d kernel_last_tile_size = {last_tile_size[0] * num_of_pkts_per_beat, last_tile_size[1]};
    #ifdef DEBUG_LOG
        std::cout << "[DEBUG][KERNEL_WRAPPER]|" << __func__ << "| TILE PARAMETERS AFTER TILE METADATA GENERATION" << std::endl;
        std::cout << "[DEBUG][KERNEL_WRAPPER]|" << __func__ << "| effective_tile_size: (" 
                  << effective_tile_size[0] << ", " << effective_tile_size[1] << ")\n";
        std::cout << "[DEBUG][KERNEL_WRAPPER]|" << __func__ << "| last_tile_size: (" 
                  << last_tile_size[0] << ", " << last_tile_size[1] << ")\n";
        std::cout << "[DEBUG][KERNEL_WRAPPER]|" << __func__ << "| tile_count: (" 
                  << tile_count[0] << ", " << tile_count[1] << ")\n";
        std::cout << "[DEBUG][KERNEL_WRAPPER]|" << __func__ << "| total_xblocks_widen: " 
                  << total_xblocks_widen << "\n";
        std::cout << "[DEBUG][KERNEL_WRAPPER]|" << __func__ << "| kernel_tile_size: (" 
                  << kernel_tile_size[0] << ", " << kernel_tile_size[1] << ")\n";
        std::cout << "[DEBUG][KERNEL_WRAPPER]|" << __func__ << "| kernel_last_tile_size: (" 
                  << kernel_last_tile_size[0] << ", " << kernel_last_tile_size[1] << ")\n";
    #endif
#endif
#ifdef DEBUG_LOG
        printAccessRange(range, "common access range");
        printGridProp(arg0.originalProperty, "arg0_originalGridProp");
        printStencilConfig(read_stencilConfig, "read_stencilConfig");
#endif

		unsigned int total_iter_par_factor = 2;
		unsigned int adjusted_outer_iter = (outer_iter + total_iter_par_factor - 1) / total_iter_par_factor;
        int narg = 0;
         
        // OCL_CHECK(err, err = m_kernel_0.setArg(narg++, (unsigned short)0));
        OCL_CHECK(err, err = m_kernel_0.setArg(narg++, adjusted_outer_iter));
        // OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.grid_size[0]));
        // OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.grid_size[1]));
        // OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.dim));
        OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.total_itr));
#ifndef OPS_TILING
        // OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.lower_limit[0]));
        // OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.lower_limit[1]));
        // OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.upper_limit[0]));
        // OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.upper_limit[1]));
 #endif
        // OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.outer_loop_limit));
#ifndef OPS_TILING
        // OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.batch_size));
#else
        // OCL_CHECK(err, err = m_kernel_0.setArg(narg++, kernel_tile_size[0]));
        // OCL_CHECK(err, err = m_kernel_0.setArg(narg++, kernel_last_tile_size[0]));
        // OCL_CHECK(err, err = m_kernel_0.setArg(narg++, tile_count[0]));
        // OCL_CHECK(err, err = m_kernel_0.setArg(narg++, total_xblocks_widen));
        // OCL_CHECK(err, err = m_kernel_0.setArg(narg++, last_tile_upper_limit_x));
#endif
        narg = 0; 
        // OCL_CHECK(err, err = m_kernel_1.setArg(narg++, (unsigned short)1));
        OCL_CHECK(err, err = m_kernel_1.setArg(narg++, adjusted_outer_iter));
        // OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.grid_size[0]));
        // OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.grid_size[1]));
        // OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.dim));
        OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.total_itr));
#ifndef OPS_TILING
        // OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.lower_limit[0]));
        // OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.lower_limit[1]));
        // OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.upper_limit[0]));
        // OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.upper_limit[1]));
 #endif
        // OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.outer_loop_limit));
#ifndef OPS_TILING
        // OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.batch_size));
#else
        // OCL_CHECK(err, err = m_kernel_1.setArg(narg++, kernel_tile_size[0]));
        // OCL_CHECK(err, err = m_kernel_1.setArg(narg++, kernel_last_tile_size[0]));
        // OCL_CHECK(err, err = m_kernel_1.setArg(narg++, tile_count[0]));
        // OCL_CHECK(err, err = m_kernel_1.setArg(narg++, total_xblocks_widen));
        // OCL_CHECK(err, err = m_kernel_1.setArg(narg++, last_tile_upper_limit_x));
#endif

#ifndef OPS_HLS_NO_LOOPBACK
        bool loopback_enbl = true;
#else
        bool loopback_enbl = false;
#endif 
        narg = 0;
        // OCL_CHECK(err, err = m_datamover.setArg(narg++, read_range.start[0]));
        // OCL_CHECK(err, err = m_datamover.setArg(narg++, read_range.end[0]));
        // OCL_CHECK(err, err = m_datamover.setArg(narg++, read_range.start[1]));
        // OCL_CHECK(err, err = m_datamover.setArg(narg++, read_range.end[1]));
        // OCL_CHECK(err, err = m_datamover.setArg(narg++, arg0.originalProperty.grid_size[0]));
        // OCL_CHECK(err, err = m_datamover.setArg(narg++, arg0.originalProperty.grid_size[1]));
        OCL_CHECK(err, err = m_datamover.setArg(narg++, num_beats));
        OCL_CHECK(err, err = m_datamover.setArg(narg++, num_of_pkts));
        OCL_CHECK(err, err = m_datamover.setArg(narg++, adjusted_outer_iter - 1));
#ifndef OPS_TILING
        // OCL_CHECK(err, err = m_datamover.setArg(narg++, arg0.originalProperty.batch_size)); 
#else
        // OCL_CHECK(err, err = m_datamover.setArg(narg++, tile_size[0]));
        // OCL_CHECK(err, err = m_datamover.setArg(narg++, overlap_size[0]));
        // OCL_CHECK(err, err = m_datamover.setArg(narg++, effective_tile_size[0]));
        // OCL_CHECK(err, err = m_datamover.setArg(narg++, last_tile_size[0]));
        // OCL_CHECK(err, err = m_datamover.setArg(narg++, tile_count[0]));
        // OCL_CHECK(err, err = m_datamover.setArg(narg++, total_xblocks_widen));
#endif
#ifdef OPS_TILING
    printf("Tiles per grid: %d \n", arg0.alt_banks);
        OCL_CHECK(err, err = m_datamover.setArg(narg++, arg0.deviceBuffer[0]));
#else
        OCL_CHECK(err, err = m_datamover.setArg(narg++, arg0.deviceBuffer));
#endif
#ifdef OPS_TILING
    printf("Tiles per grid: %d \n", arg0.alt_banks);
        OCL_CHECK(err, err = m_datamover.setArg(narg++, arg1.deviceBuffer[0]));
#else
        OCL_CHECK(err, err = m_datamover.setArg(narg++, arg1.deviceBuffer));
#endif

        std::vector<cl::Event> h2d_events;
        cl::Event event_h2d_arg0 = arg0.set_as_arg();
        cl::Event event_h2d_arg1 = arg1.set_as_arg();
#ifdef PROFILE
        h2d_events.push_back(event_h2d_arg0);
        h2d_events.push_back(event_h2d_arg1);
        recordH2DEvent(h2d_events);
#endif

        cl::Event event_kernel_0;
        cl::Event event_kernel_1;
        cl::Event event_datamover;

        std::vector<cl::Event> activeEvents;
        activeEvents.insert(activeEvents.end(), arg0.activeEvents.begin(), arg0.activeEvents.end());
        activeEvents.insert(activeEvents.end(), arg1.activeEvents.begin(), arg1.activeEvents.end());

    // enque in backward order
        OCL_CHECK(err, err = m_fpga->getCommandQueue().enqueueTask(m_kernel_0, &activeEvents, &event_kernel_0));
        OCL_CHECK(err, err = m_fpga->getCommandQueue().enqueueTask(m_kernel_1, &activeEvents, &event_kernel_1));
        OCL_CHECK(err, err = m_fpga->getCommandQueue().enqueueTask(m_datamover, &activeEvents, &event_datamover));


#ifdef PROFILE
    std::vector<cl::Event> execEvents;
    execEvents.push_back(event_datamover);
        execEvents.push_back(event_kernel_0);
        execEvents.push_back(event_kernel_1);
    recordExecEvent(event_datamover);
    registerProfileEvents();
#endif

        arg0.isDevBufDirty = true;
        arg0.activeEvents.resize(0);
        arg0.activeEvents.push_back(event_datamover);
        arg0.activeEvents.push_back(event_kernel_0);
        arg0.activeEvents.push_back(event_kernel_1);
        arg1.isDevBufDirty = true;
        arg1.activeEvents.resize(0);
        arg1.activeEvents.push_back(event_datamover);
        arg1.activeEvents.push_back(event_kernel_0);
        arg1.activeEvents.push_back(event_kernel_1);

#ifndef ASYNC_DISPATCH
        event_datamover.wait();
        event_kernel_0.wait();
        event_kernel_1.wait();
#else
    #ifdef DEBUG_LOG
        printf("[DEBUG_HOST] Async dispatch enabled, not waiting for kernel completion.\n");
    #endif
#endif

/*
        //Sync maped dats
        activeEvents.resize(0);
        activeEvents.insert(activeEvents.end(), arg0.activeEvents.begin(), arg0.activeEvents.end());
        activeEvents.insert(activeEvents.end(), arg1.activeEvents.begin(), arg1.activeEvents.end());

        cl::Event event_arg0_arg1_bufCpy;
        size_t arg0_total_bytes = arg0.originalProperty.grid_size[0] * arg0.originalProperty.grid_size[1] * arg0.originalProperty.grid_size[2] * sizeof(stencil_type);

        if (adjusted_outer_iter %2 != 0)
        {
            OCL_CHECK(err, err = m_fpga->getCommandQueue().enqueueCopyBuffer(arg0.deviceBuffer, arg1.deviceBuffer, 0, 0, arg0_total_bytes, &activeEvents, &event_arg0_arg1_bufCpy));
        }
        else
        {
            OCL_CHECK(err, err = m_fpga->getCommandQueue().enqueueCopyBuffer(arg1.deviceBuffer, arg0.deviceBuffer, 0, 0, arg0_total_bytes, &activeEvents, &event_arg0_arg1_bufCpy));
        }
#ifndef ASYNC_DISPATCH
        event_arg0_arg1_bufCpy.wait();
#endif
*/
    }
private:
    std::string m_kernelName;

    std::string m_datamoverName;
    cl::Kernel m_kernel_0;
    cl::Kernel m_kernel_1;
    cl::Kernel m_datamover;
};
#else 
void tapa_kernel_wrapper(ops::hls::AccessRange& range, unsigned int outer_iter,
            ops::hls::Grid<float>& arg0,
            ops::hls::Grid<float>& arg1)
{   
    ops::hls::SizeType read_stencil_d_m = { 1, 1, 0 };
    ops::hls::SizeType read_stencil_d_p = { 1, 1, 0 };
    ops::hls::SizeType write_stencil_d_m = {0,0,0};
    ops::hls::SizeType write_stencil_d_p = {0,0,0};
    auto read_stencilConfig = getStencilConfig(arg0.originalProperty, range, vector_factor, mem_vector_factor, read_stencil_d_m, read_stencil_d_p);
    ops::hls::AccessRange read_range;
    getAdjustedRange(arg0.originalProperty, range, read_range, read_stencil_d_m, read_stencil_d_p);
    ops::hls::MemConfig memconfig;
    ops::hls::genMemConfig<mem_data_width, axis_data_width, data_width>(arg0.originalProperty.grid_size, read_range, memconfig);
    const unsigned int num_beats = memconfig.total_xblocks * arg0.originalProperty.batch_size;
    const unsigned int num_of_pkts_per_beat = mem_data_width / axis_data_width;
    const unsigned int num_of_pkts = num_beats * num_of_pkts_per_beat;

    unsigned int total_iter_par_factor = 1;
    unsigned int adjusted_outer_iter = (outer_iter + total_iter_par_factor - 1) / total_iter_par_factor;
    
#ifdef DEBUG_LOG
    std::cout << "[DEBUG] Invoking TAPA mega-kernel in simulation mode..." << std::endl;
#endif

    size_t vec_size_arg0 = arg0.hostBuffer.size() / 16;
    size_t vec_size_arg1 = arg1.hostBuffer.size() / 16;
    // Invoke the TAPA software simulation
    // FLAGS_bitstream is empty ("") during SW simulation, avoiding actual XCLBIN loading.

    ::tapa::stream<::tapa::vec_t<float,vector_factor>,4,4096> arg0_axis("arg0_axis_stream");
    ::tapa::stream<::tapa::vec_t<float,vector_factor>,4,4096> arg1_axis("arg1_axis_stream");

//     tapa::invoke(sim_mega_kernel, "",
//         num_beats,
//         num_of_pkts,
//         adjusted_outer_iter-1,
//         adjusted_outer_iter,
//         read_stencilConfig.total_itr,
//         // Wrap the raw host pointers from the Grid class into TAPA memory maps
//         tapa::read_write_mmap<tapa::vec_t<float, 16>>(
//             reinterpret_cast<tapa::vec_t<float, 16>*>(arg0.hostBuffer.data()), 
//             vec_size_arg0
//         ),
//         tapa::read_write_mmap<tapa::vec_t<float, 16>>(
//             reinterpret_cast<tapa::vec_t<float, 16>*>(arg1.hostBuffer.data()), 
//             vec_size_arg1
//         )
//     );
        tapa::task()
            .invoke(datamover_outerloop_0, "",
                num_beats,
                num_of_pkts,
                adjusted_outer_iter,
                // Wrap the raw host pointers from the Grid class into TAPA memory maps
                tapa::read_write_mmap<tapa::vec_t<float, 16>>(
                reinterpret_cast<tapa::vec_t<float, 16>*>(arg0.hostBuffer.data()), 
                vec_size_arg0
                ),
                tapa::read_write_mmap<tapa::vec_t<float, 16>>(
                reinterpret_cast<tapa::vec_t<float, 16>*>(arg1.hostBuffer.data()), 
                vec_size_arg1
                ),
                arg0_axis,
                arg1_axis
        )
            .invoke(kernel_outerloop_0, "",
                adjusted_outer_iter,  
                num_of_pkts,          
                arg0_axis,
                arg1_axis
        );
}
#endif

void isl0(int outer_iter, int* ops_range,
            ops::hls::Grid<float>& arg0,
            ops::hls::Grid<float>& arg1
)
{
    ops::hls::AccessRange range;
    opsRange2hlsRange(2, ops_range, range, arg0.originalProperty);
#if !defined(TAPA_SW_EMU) && !defined(TAPA_HW_EMU)
    static  KernelWrapper_outerloop_0 kernelWrapper_inst;

    sendGrid(arg0);
    
    kernelWrapper_inst.run(range, outer_iter,
            arg0,
            arg1
    );
#else
    tapa_kernel_wrapper(range, outer_iter,
            arg0,
            arg1);
#endif
}
