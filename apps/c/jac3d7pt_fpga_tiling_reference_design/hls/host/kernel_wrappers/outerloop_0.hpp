// Auto-generated at 2025-09-06 21:18:49.545567 by ops-translator

#pragma once 
#include <ops_hls_rt_support.h>

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
        OCL_CHECK(err, m_kernel_2 = cl::Kernel(m_fpga->getProgram(), m_kernelName.c_str(), &err));
        OCL_CHECK(err, m_datamover = cl::Kernel(m_fpga->getProgram(), m_datamoverName.c_str(), &err));    
    }

    void run(ops::hls::AccessRange& range, unsigned int outer_iter,
            ops::hls::Grid<float>& arg0,
            ops::hls::Grid<float>& arg1
    )
    {
        cl_int err;
    
        ops::hls::SizeType read_stencil_d_m = { 1, 1, 1 };
        ops::hls::SizeType read_stencil_d_p = { 1, 1, 1 };
        ops::hls::SizeType write_stencil_d_m = {0,0,0};
        ops::hls::SizeType write_stencil_d_p = {0,0,0};
        auto read_stencilConfig = getStencilConfig(arg0.originalProperty, range, vector_factor, mem_vector_factor, read_stencil_d_m, read_stencil_d_p);
        ops::hls::AccessRange read_range;
        getAdjustedRange(arg0.originalProperty, range, read_range, read_stencil_d_m, read_stencil_d_p);

#ifdef DEBUG_LOG
        printAccessRange(range, "common access range");
        printGridProp(arg0.originalProperty, "arg0_originalGridProp");
        printStencilConfig(read_stencilConfig, "read_stencilConfig");
#endif

		unsigned int total_iter_par_factor = iter_par_factor * 3;
		unsigned int adjusted_outer_iter = (outer_iter + total_iter_par_factor - 1) / total_iter_par_factor;

        int narg = 0;
         
        OCL_CHECK(err, err = m_kernel_0.setArg(narg++, (unsigned short)0));
        OCL_CHECK(err, err = m_kernel_0.setArg(narg++, adjusted_outer_iter));
        OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.grid_size[0]));
        OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.grid_size[1]));
        OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.grid_size[2]));
        OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.dim));
        OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.total_itr));
        OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.lower_limit[0]));
        OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.lower_limit[1]));
        OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.lower_limit[2]));
        OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.upper_limit[0]));
        OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.upper_limit[1]));
        OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.upper_limit[2]));
        OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.outer_loop_limit));
        OCL_CHECK(err, err = m_kernel_0.setArg(narg++, read_stencilConfig.batch_size));
        narg = 0; 
        OCL_CHECK(err, err = m_kernel_1.setArg(narg++, (unsigned short)1));
        OCL_CHECK(err, err = m_kernel_1.setArg(narg++, adjusted_outer_iter));
        OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.grid_size[0]));
        OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.grid_size[1]));
        OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.grid_size[2]));
        OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.dim));
        OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.total_itr));
        OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.lower_limit[0]));
        OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.lower_limit[1]));
        OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.lower_limit[2]));
        OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.upper_limit[0]));
        OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.upper_limit[1]));
        OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.upper_limit[2]));
        OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.outer_loop_limit));
        OCL_CHECK(err, err = m_kernel_1.setArg(narg++, read_stencilConfig.batch_size));
        narg = 0; 
        OCL_CHECK(err, err = m_kernel_2.setArg(narg++, (unsigned short)2));
        OCL_CHECK(err, err = m_kernel_2.setArg(narg++, adjusted_outer_iter));
        OCL_CHECK(err, err = m_kernel_2.setArg(narg++, read_stencilConfig.grid_size[0]));
        OCL_CHECK(err, err = m_kernel_2.setArg(narg++, read_stencilConfig.grid_size[1]));
        OCL_CHECK(err, err = m_kernel_2.setArg(narg++, read_stencilConfig.grid_size[2]));
        OCL_CHECK(err, err = m_kernel_2.setArg(narg++, read_stencilConfig.dim));
        OCL_CHECK(err, err = m_kernel_2.setArg(narg++, read_stencilConfig.total_itr));
        OCL_CHECK(err, err = m_kernel_2.setArg(narg++, read_stencilConfig.lower_limit[0]));
        OCL_CHECK(err, err = m_kernel_2.setArg(narg++, read_stencilConfig.lower_limit[1]));
        OCL_CHECK(err, err = m_kernel_2.setArg(narg++, read_stencilConfig.lower_limit[2]));
        OCL_CHECK(err, err = m_kernel_2.setArg(narg++, read_stencilConfig.upper_limit[0]));
        OCL_CHECK(err, err = m_kernel_2.setArg(narg++, read_stencilConfig.upper_limit[1]));
        OCL_CHECK(err, err = m_kernel_2.setArg(narg++, read_stencilConfig.upper_limit[2]));
        OCL_CHECK(err, err = m_kernel_2.setArg(narg++, read_stencilConfig.outer_loop_limit));
        OCL_CHECK(err, err = m_kernel_2.setArg(narg++, read_stencilConfig.batch_size));

#ifndef OPS_HLS_NO_LOOPBACK
        bool loopback_enbl = true;
#else
        bool loopback_enbl = false;
#endif 
        narg = 0;
        OCL_CHECK(err, err = m_datamover.setArg(narg++, loopback_enbl));
        OCL_CHECK(err, err = m_datamover.setArg(narg++, read_range.start[0]));
        OCL_CHECK(err, err = m_datamover.setArg(narg++, read_range.end[0]));
        OCL_CHECK(err, err = m_datamover.setArg(narg++, read_range.start[1]));
        OCL_CHECK(err, err = m_datamover.setArg(narg++, read_range.end[1]));
        OCL_CHECK(err, err = m_datamover.setArg(narg++, read_range.start[2]));
        OCL_CHECK(err, err = m_datamover.setArg(narg++, read_range.end[2]));
        OCL_CHECK(err, err = m_datamover.setArg(narg++, arg0.originalProperty.grid_size[0]));
        OCL_CHECK(err, err = m_datamover.setArg(narg++, arg0.originalProperty.grid_size[1]));
        OCL_CHECK(err, err = m_datamover.setArg(narg++, arg0.originalProperty.grid_size[2]));
 
        OCL_CHECK(err, err = m_datamover.setArg(narg++, adjusted_outer_iter));
        OCL_CHECK(err, err = m_datamover.setArg(narg++, arg0.originalProperty.batch_size));    
        OCL_CHECK(err, err = m_datamover.setArg(narg++, arg0.deviceBuffer));
        OCL_CHECK(err, err = m_datamover.setArg(narg++, arg1.deviceBuffer));

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
        cl::Event event_kernel_2;
        cl::Event event_datamover;

        std::vector<cl::Event> activeEvents;
        activeEvents.insert(activeEvents.end(), arg0.activeEvents.begin(), arg0.activeEvents.end());
        activeEvents.insert(activeEvents.end(), arg1.activeEvents.begin(), arg1.activeEvents.end());

        OCL_CHECK(err, err = m_fpga->getCommandQueue().enqueueTask(m_datamover, &activeEvents, &event_datamover));
        OCL_CHECK(err, err = m_fpga->getCommandQueue().enqueueTask(m_kernel_0, &activeEvents, &event_kernel_0));
        OCL_CHECK(err, err = m_fpga->getCommandQueue().enqueueTask(m_kernel_1, &activeEvents, &event_kernel_1));
        OCL_CHECK(err, err = m_fpga->getCommandQueue().enqueueTask(m_kernel_2, &activeEvents, &event_kernel_2));


#ifdef PROFILE
    std::vector<cl::Event> execEvents;
    execEvents.push_back(event_datamover);
        execEvents.push_back(event_kernel_0);
        execEvents.push_back(event_kernel_1);
        execEvents.push_back(event_kernel_2);
    recordExecEvent(event_datamover);
    registerProfileEvents();
#endif

        arg0.isDevBufDirty = true;
        arg0.activeEvents.resize(0);
        arg0.activeEvents.push_back(event_datamover);
        arg0.activeEvents.push_back(event_kernel_0);
        arg0.activeEvents.push_back(event_kernel_1);
        arg0.activeEvents.push_back(event_kernel_2);
        arg1.isDevBufDirty = true;
        arg1.activeEvents.resize(0);
        arg1.activeEvents.push_back(event_datamover);
        arg1.activeEvents.push_back(event_kernel_0);
        arg1.activeEvents.push_back(event_kernel_1);
        arg1.activeEvents.push_back(event_kernel_2);

#ifndef ASYNC_DISPATCH
        event_datamover.wait();
        event_kernel_0.wait();
        event_kernel_1.wait();
        event_kernel_2.wait();
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
    cl::Kernel m_kernel_2;
    cl::Kernel m_datamover;
};

void isl0(int outer_iter, int* ops_range,
            ops::hls::Grid<float>& arg0,
            ops::hls::Grid<float>& arg1
)
{
    static  KernelWrapper_outerloop_0 kernelWrapper_inst;
    ops::hls::AccessRange range;
    opsRange2hlsRange(3, ops_range, range, arg0.originalProperty);
    sendGrid(arg0);
    
    kernelWrapper_inst.run(range, outer_iter,
            arg0,
            arg1
    );
}
