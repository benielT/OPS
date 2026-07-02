
/* 
* Open source copyright declaration based on BSD open source template:
* http://www.opensource.org/licenses/bsd-license.php
*
* This file is part of the OPS distribution.
*
* Copyright (c) 2013, Mike Giles and others. Please see the AUTHORS file in
* the main source directory for a full list of copyright holders.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
*/

/** @file
  * @brief kernel handle class
  * @author Beniel Thileepan (maintainer)
  * @details Abstract class definition to manage kernel with XOCL api as a utility 
  * to support host side and device side communication.
  */

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#pragma once

#include <iostream>
#include <vector>
#include <regex>
#include <utility>
#include <string>
#include <cassert>
#include <unordered_map>
#include "../../common/include/ops_hls_defs.hpp"
#include "ops_hls_fpga.hpp"
// This extension file is required for stream APIs
// #include "CL/cl_ext_xilinx.h"
// This file is required for OpenCL C++ wrapper APIs
#include "../../ext/xcl2/xcl2.hpp"

#ifndef OPS_HLS_TILE_BANKS
    #define OPS_HLS_TILE_BANKS 1
#endif
namespace ops
{
namespace hls
{

//class bad_kernel_name : public std::exception
//{
//public:
//	bad_kernel_name() throw() { }
//
//#if __cplusplus >= 201103L
//	bad_kernel_name(const bad_kernel_name&) = default;
//	bad_kernel_name& operator=(const bad_kernel_name&) = default;
//#endif
//
//  // This declaration is not useless:
//  // http://gcc.gnu.org/onlinedocs/gcc-3.0.2/gcc_6.html#SEC118
//  virtual ~bad_kernel_name() throw();
//
//  // See comment in eh_exception.cc.
//  virtual const char* what() const throw();
//};


#ifndef OPS_HLS_V2
template <typename T>
class Grid
{
public:
	GridPropertyCore originalProperty;
	unsigned short vector_factor = 8; //grid vector factor considered for adjustements
	host_buffer_t<T> hostBuffer;
	cl::Buffer deviceBuffer;
	std::vector<cl::Event> activeEvents;
	std::vector<std::pair<cl::Event, std::string>> allEvents;
	bool isHostBufDirty;
	bool isDevBufDirty;
	bool isSetAsArg;

	void* get_raw_pointer()
	{
		getGrid(*this, true);
		return (void*)hostBuffer.data();
	}

    cl::Event set_as_arg()
	{
		isSetAsArg = true;
        auto event = sendGrid(*this);
		isSetAsArg = false;
        return event;
	}
};
#else
template <typename T>
class Grid
{
public:
	GridPropertyCoreV2 originalProperty;
	unsigned short vector_factor = 8; //grid vector factor considered for adjustements
	host_buffer_t<T> hostBuffer;
    #ifdef OPS_TILING
    std::vector<host_buffer_t<T>> altHostBuffers = std::vector<host_buffer_t<T>>(OPS_HLS_TILE_BANKS);
    std::vector<cl::Buffer> deviceBuffer;
    #else
    cl::Buffer deviceBuffer;
    #endif
	std::vector<cl::Event> activeEvents;
	std::vector<std::pair<cl::Event, std::string>> allEvents;
	bool isHostBufDirty;
	bool isDevBufDirty;
	bool isSetAsArg;
    #ifdef OPS_TILING
    static const unsigned short alt_banks = OPS_HLS_TILE_BANKS;
    #else
    static const unsigned short alt_banks = 1;
    #endif
    typedef T type;

	void* get_raw_pointer()
	{
		getGrid(*this, true);
		return (void*)hostBuffer.data();
	}

    cl::Event set_as_arg()
	{
		isSetAsArg = true;
        auto event = sendGrid(*this);       
		isSetAsArg = false;
        return event;
	}
    #ifdef OPS_TILING
    std::vector<size_t> getAltBufferRowsCounts() {
        std::vector<size_t> alt_buffer_row_counts(alt_banks);
        if (alt_banks > 1 && originalProperty.dim >= 2) {
            auto total_rows = originalProperty.grid_size[1] * originalProperty.grid_size[2];
            for (int bank = 0; bank < alt_banks; bank++) {
                alt_buffer_row_counts[bank] = total_rows / alt_banks + ((bank < (total_rows % alt_banks)) ? 1 : 0);
    // #ifdef DEBUG_LOG
            // printf("[DEBUG]| %s | Bank %d: %zu rows\n", __func__, bank, alt_buffer_row_counts[bank]);
    // #endif
            }
        }
        return alt_buffer_row_counts;
    }

    std::vector<size_t> getAltBufferSizes(bool with_batch_size = false) {
        std::vector<size_t> alt_buffer_sizes(alt_banks);
    // #if defined(OPS_HLS_TILE_INTERLEAVE)
    //     size_t total_vect_elements = (originalProperty.grid_size[0] + mem_vector_factor - 1) / mem_vector_factor;
    //     total_vect_elements *= originalProperty.grid_size[1] * originalProperty.grid_size[2];
        
    //     for (int bank = 0; bank < alt_banks; bank++) {
    //         size_t bank_vect_elements = (total_vect_elements / alt_banks) + (bank < (total_vect_elements % alt_banks) ? 1 : 0);
    //         alt_buffer_sizes[bank] = bank_vect_elements * mem_vector_factor;
    //         alt_buffer_sizes[bank] *= originalProperty.batch_size;
    #if defined(OPS_HLS_TILE_INTERLEAVE)
        size_t total_vect_element_x = (originalProperty.grid_size[0] + mem_vector_factor - 1) / mem_vector_factor;
        auto size_y_mult_size_z = originalProperty.grid_size[1] * originalProperty.grid_size[2];

        for (int bank = 0; bank < alt_banks; bank++) {
            size_t bank_vect_elements = (total_vect_element_x / alt_banks) + (bank < (total_vect_element_x % alt_banks) ? 1 : 0);
            bank_vect_elements *= size_y_mult_size_z;
            alt_buffer_sizes[bank] = bank_vect_elements * mem_vector_factor;
            if (with_batch_size)
                alt_buffer_sizes[bank] *= originalProperty.batch_size;
    #else 
        auto alt_buffer_row_counts = getAltBufferRowsCounts();
        
        for (int bank = 0; bank < alt_banks; bank++) {
            size_t row_size = originalProperty.grid_size[0];
            alt_buffer_sizes[bank] = row_size * alt_buffer_row_counts[bank];
            if (with_batch_size)
                alt_buffer_sizes[bank] *= originalProperty.batch_size;
    #endif
    // #ifdef DEBUG_LOG
    //         printf("[DEBUG]| %s | Bank %d: %zu bank_buffer_size\n", __func__, bank, alt_buffer_sizes[bank]);
    // #endif
        }
        return alt_buffer_sizes;
    }

    std::vector<size_t> getAltBufferSizesBytes() {
        std::vector<size_t> alt_buffer_sizes = getAltBufferSizes();
        // auto alt_buffer_row_counts = getAltBufferRowsCounts();
        for (int bank = 0; bank < alt_banks; bank++) {
            alt_buffer_sizes[bank] *= sizeof(T);

    #ifdef DEBUG_LOG
            printf("[DEBUG]| %s | Bank %d: %zu bank_buffer_size (bytes)\n", __func__, bank, alt_buffer_sizes[bank]);
    #endif
        }
        return alt_buffer_sizes;
    }

    void splitGrid()
    {
        if (alt_banks > 1 && originalProperty.dim >=2) {
        #if defined(OPS_HLS_TILE_INTERLEAVE)
            size_t total_vect_element_x = (originalProperty.grid_size[0] + mem_vector_factor - 1) / mem_vector_factor;
            auto size_y_mult_size_z = originalProperty.grid_size[1] * originalProperty.grid_size[2];
        #endif
            for (unsigned int b = 0; b < originalProperty.batch_size; b++) {
                for (unsigned int k = 0; k < originalProperty.grid_size[2]; k++) {
                    for (unsigned int j = 0; j < originalProperty.grid_size[1]; j++) {
        #if defined(OPS_HLS_TILE_INTERLEAVE)
                        for (unsigned int i = 0; i < originalProperty.grid_size[0]; i+=mem_vector_factor) {
                            size_t src_offset = i + j * originalProperty.grid_size[0] 
                                    + k * originalProperty.grid_size[0] * originalProperty.grid_size[1] 
                                    + b * originalProperty.grid_size[0] * originalProperty.grid_size[1] * originalProperty.grid_size[2];
                            size_t vect_i = i / mem_vector_factor;
                            // unsigned long vectored_index = abs_index / mem_vector_factor;
                            unsigned int bank = vect_i % alt_banks;
                            unsigned int bank_size_x = (total_vect_element_x / alt_banks) + (bank < (total_vect_element_x % alt_banks) ? 1 : 0);
                            size_t bank_index = vect_i / alt_banks 
                                    + j * bank_size_x 
                                    + k * bank_size_x * originalProperty.grid_size[1];
                            bank_index *= mem_vector_factor;
                            size_t dst_offset = b * getAltBufferSizes()[bank] + bank_index;
                            std::memcpy(&altHostBuffers[bank][dst_offset],
                                    &hostBuffer[src_offset],
                                    mem_vector_factor * sizeof(T));
                        }
        // #if defined(OPS_HLS_TILE_INTERLEAVE)
        //                 for (unsigned int i = 0; i < originalProperty.grid_size[0]; i+=mem_vector_factor) {
        //                     unsigned long abs_index = i + j * originalProperty.grid_size[0] 
        //                                                 + k * originalProperty.grid_size[0] * originalProperty.grid_size[1];
                            
        //                     unsigned long vectored_index = abs_index / mem_vector_factor;
        //                     unsigned int bank = vectored_index % alt_banks;
        //                     unsigned long bank_index = vectored_index / alt_banks;
        //                     bank_index *= mem_vector_factor;
        //                     size_t src_offset = b * originalProperty.grid_size[0] * originalProperty.grid_size[1] * originalProperty.grid_size[2]
        //                                 + k * originalProperty.grid_size[0] * originalProperty.grid_size[1]
        //                                 + j * originalProperty.grid_size[0] + i;
        //                     size_t dst_offset = b * getAltBufferSizes()[bank] + bank_index;
        //                     std::memcpy(&altHostBuffers[bank][dst_offset],
        //                             &hostBuffer[src_offset],
        //                             mem_vector_factor * sizeof(T));
        //                 }
        #else

                        unsigned int abs_row_id = j + k * originalProperty.grid_size[1];
                        unsigned int bank = abs_row_id % alt_banks;
                        unsigned int bank_row = abs_row_id / alt_banks;
                        
                        size_t src_offset = b * originalProperty.grid_size[0] * originalProperty.grid_size[1] * originalProperty.grid_size[2]
                                        + k * originalProperty.grid_size[0] * originalProperty.grid_size[1]
                                        + j * originalProperty.grid_size[0];
                        
                        size_t dst_offset = b * getAltBufferSizes()[bank] + (bank_row * originalProperty.grid_size[0]);
                        // size_t dst_offset = b * getAltBufferSizes()[bank] 
                        //                 + k * (originalProperty.grid_size[0] * (originalProperty.grid_size[1] / alt_banks + ((bank < (originalProperty.grid_size[1] % alt_banks)) ? 1 : 0)))
                        //                 + bank_row * originalProperty.grid_size[0];
                        
            #ifdef DEBUG_LOG
		                printf("j: %d, k: %d, bank: %d, bank_row: %d, src_offset: %d dst_offset: %d\n", j, k, bank, bank_row, src_offset, dst_offset);
            #endif
                        std::memcpy(&altHostBuffers[bank][dst_offset],
                                    &hostBuffer[src_offset],
                                    originalProperty.grid_size[0] * sizeof(T));
        #endif //OPS_HLS_TILE_INTERLEAVE
                    }
                }
            }
        }
    }
 
    void mergeGrid() 
    {
        if (alt_banks > 1 && originalProperty.dim >=2) {
        #if defined(OPS_HLS_TILE_INTERLEAVE)
            size_t total_vect_element_x = (originalProperty.grid_size[0] + mem_vector_factor - 1) / mem_vector_factor;
            auto size_y_mult_size_z = originalProperty.grid_size[1] * originalProperty.grid_size[2];
        #endif
            for (unsigned int b = 0; b < originalProperty.batch_size; b++) {
                for (unsigned int k = 0; k < originalProperty.grid_size[2]; k++) {
                    for (unsigned int j = 0; j < originalProperty.grid_size[1]; j++) {
        #if defined(OPS_HLS_TILE_INTERLEAVE)
                        for (unsigned int i = 0; i < originalProperty.grid_size[0]; i+=mem_vector_factor) {
                            size_t src_offset = i + j * originalProperty.grid_size[0] 
                                    + k * originalProperty.grid_size[0] * originalProperty.grid_size[1] 
                                    + b * originalProperty.grid_size[0] * originalProperty.grid_size[1] * originalProperty.grid_size[2];
                            size_t vect_i = i / mem_vector_factor;
                            // unsigned long vectored_index = abs_index / mem_vector_factor;
                            unsigned int bank = vect_i % alt_banks;
                            unsigned int bank_size_x = (total_vect_element_x / alt_banks) + (bank < (total_vect_element_x % alt_banks) ? 1 : 0);
                            size_t bank_index = vect_i / alt_banks 
                                    + j * bank_size_x 
                                    + k * bank_size_x * originalProperty.grid_size[1];
                            bank_index *= mem_vector_factor;
                            size_t dst_offset = b * getAltBufferSizes()[bank] + bank_index;
                            std::memcpy(&hostBuffer[src_offset],
                                    &altHostBuffers[bank][dst_offset],
                                    mem_vector_factor * sizeof(T));
                        }
        // #if defined(OPS_HLS_TILE_INTERLEAVE)
        //                 for (unsigned int i = 0; i < originalProperty.grid_size[0]; i+=mem_vector_factor) {
        //                     unsigned long abs_index = i + j * originalProperty.grid_size[0] 
        //                                                 + k * originalProperty.grid_size[0] * originalProperty.grid_size[1];
        //                     unsigned long vectored_index = abs_index / mem_vector_factor;
        //                     unsigned int bank = vectored_index % alt_banks;
        //                     unsigned long bank_index = vectored_index / alt_banks;
        //                     bank_index *= mem_vector_factor;
        //                     size_t src_offset = b * originalProperty.grid_size[0] * originalProperty.grid_size[1] * originalProperty.grid_size[2]
        //                                 + k * originalProperty.grid_size[0] * originalProperty.grid_size[1]
        //                                 + j * originalProperty.grid_size[0] + i;
        //                     size_t dst_offset = b * getAltBufferSizes()[bank] + bank_index;
        //                     std::memcpy(&hostBuffer[src_offset],
        //                             &altHostBuffers[bank][dst_offset],
        //                             mem_vector_factor * sizeof(T));
        //                 }
        #else
                        unsigned int abs_row_id = j + k * originalProperty.grid_size[1];
                        unsigned int bank = abs_row_id % alt_banks;
                        unsigned int bank_row = abs_row_id / alt_banks;
                        
                        size_t src_offset = b * originalProperty.grid_size[0] * originalProperty.grid_size[1] * originalProperty.grid_size[2]
                                        + k * originalProperty.grid_size[0] * originalProperty.grid_size[1]
                                        + j * originalProperty.grid_size[0];
                        
                        size_t dst_offset = b * getAltBufferSizes()[bank] + (bank_row * originalProperty.grid_size[0]);
                        // size_t dst_offset = b * getAltBufferSizes()[bank]
                        //                 + k * (originalProperty.grid_size[0] * (originalProperty.grid_size[1] / alt_banks + ((bank < (originalProperty.grid_size[1] % alt_banks)) ? 1 : 0)))
                        //                 + bank_row * originalProperty.grid_size[0];
                        
                        std::memcpy(&hostBuffer[src_offset],
                                    &altHostBuffers[bank][dst_offset],
                                    originalProperty.grid_size[0] * sizeof(T));
        #endif //OPS_HLS_TILE_INTERLEAVE
                    }
                }
            }
        }
    }
    #endif  //OPS_TILING
};
#endif

template <typename T>
void addEvent(Grid<T>& p_grid, cl::Event& p_event, std::string prompt="")
{
	p_grid.allEvents.push_back(std::make_pair(p_event, prompt +
			std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count())));
}

template <typename T>
cl::Event& emplaceEvent(Grid<T>& p_grid, std::string prompt="")
{
	p_grid.allEvents.emplace(p_grid.allEvents.end());
	p_grid.allEvents[p_grid.allEvents.size()-1].second =  prompt +
			std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	return p_grid.allEvents[p_grid.allEvents.size()-1].first;
}

template <typename T>
cl::Event getGrid(Grid<T>& p_grid, bool force_sync = false)
{
	if (p_grid.isDevBufDirty)
	{
		cl_int err;
		cl::Event event;
#ifndef OPS_TILING
		OCL_CHECK(err, err = FPGA::getInstance()->getCommandQueue().enqueueMigrateMemObjects({p_grid.deviceBuffer}, CL_MIGRATE_MEM_OBJECT_HOST, &p_grid.activeEvents, &event));
#else
        if (p_grid.alt_banks == 1) {
            OCL_CHECK(err, err = FPGA::getInstance()->getCommandQueue().enqueueMigrateMemObjects({p_grid.deviceBuffer[0]}, CL_MIGRATE_MEM_OBJECT_HOST, &p_grid.activeEvents, &event));
        }
        else {
            //temp std::vetor<cl::Memory>
            std::vector<cl::Memory> tmp;
            for (auto buf : p_grid.deviceBuffer)
                tmp.push_back(buf);
            OCL_CHECK(err, err = FPGA::getInstance()->getCommandQueue().enqueueMigrateMemObjects(tmp, CL_MIGRATE_MEM_OBJECT_HOST, &p_grid.activeEvents, &event));
        }
#endif
		addEvent(p_grid, event, __func__);
		p_grid.activeEvents.resize(0);
		p_grid.activeEvents.push_back(event);
		p_grid.isDevBufDirty = false;
#ifndef ASYNC_DISPATCH
		event.wait();
    #if defined(OPS_TILING)
        p_grid.mergeGrid();
    #endif
#else
        if (force_sync)
        {
            event.wait();
            p_grid.mergeGrid();
        }
#endif
        return event;
	}
    return cl::Event();
}

template <typename T>
cl::Event sendGrid(Grid<T>& p_grid)
{
	if (p_grid.isHostBufDirty and p_grid.isSetAsArg)
	{
#ifdef DEBUG_LOG
		printf("Sending dirty Host buffer to device. \n");
#endif
		cl_int err;
		cl::Event event;
#ifndef OPS_TILING
		OCL_CHECK(err, err = FPGA::getInstance()->getCommandQueue().enqueueMigrateMemObjects({p_grid.deviceBuffer}, 0, &p_grid.activeEvents, &event));
#else
        if (p_grid.alt_banks == 1) {
            OCL_CHECK(err, err = FPGA::getInstance()->getCommandQueue().enqueueMigrateMemObjects({p_grid.deviceBuffer[0]}, 0, &p_grid.activeEvents, &event));
        }
        else {
            //temp std::vetor<cl::Memory>
            p_grid.splitGrid();
            std::vector<cl::Memory> tmp;
            for (auto buf : p_grid.deviceBuffer)
                tmp.push_back(buf);
            OCL_CHECK(err, err = FPGA::getInstance()->getCommandQueue().enqueueMigrateMemObjects(tmp, 0, &p_grid.activeEvents, &event));
        }
#endif
//		addEvent(p_grid, event, __func__);
		p_grid.activeEvents.resize(0);
		p_grid.activeEvents.push_back(event);
		p_grid.isHostBufDirty = false;
		p_grid.isSetAsArg = false;
#ifndef ASYNC_DISPATCH
    #ifdef DEBUG_LOG
		printf("Waiting for sync completion. \n");
    #endif
		event.wait();
    #ifdef DEBUG_LOG
		printf("Sync completed \n");
    #endif
#endif
        return event;
	}
    return cl::Event();
}
class Kernel
{
	public:
//		Kernel(FPGA* p_fpga = nullptr) : m_fpga(p_fpga) {}

		Kernel(std::string name = ""): m_kernel_name(name)
		{
			m_fpga = FPGA::getInstance();
			if (m_fpga->runtimeEventRecExists(name))
				throw std::runtime_error("bad_kernel_name");
#ifdef DEBUG_LOG
			printf("initiating: %s \n", m_kernel_name.c_str());
#endif
		}

		void fpga(FPGA* p_fpga) { m_fpga = p_fpga; }

		void finish() const { m_fpga->finish(); }

		void run(){};

		void getBuffer(std::vector<cl::Memory>& h_m)
		{
			cl_int err;
			OCL_CHECK(err, err = m_fpga->getCommandQueue().enqueueMigrateMemObjects(h_m, CL_MIGRATE_MEM_OBJECT_HOST));
			finish();
		}

		void sendBuffer(std::vector<cl::Memory>& h_m)
		{
			cl_int err;
			OCL_CHECK(err, err = m_fpga->getCommandQueue().enqueueMigrateMemObjects(h_m, 0)); /* 0 means from host*/
			finish();
		}

		template <typename T>
		void sendGrid(std::vector<Grid<T>>& p_grid_vect)
		{
			for (auto p_grid : p_grid_vect)
			{
				sendGrid<T>(*p_grid);
			}
		}

		template <typename T>
		cl::Buffer createDeviceBuffer(cl_mem_flags p_flags, const host_buffer_t<T>& p_buffer) const
		{
			return m_fpga->createDeviceBuffer(p_flags, p_buffer);
		}

		template <typename T>
		void createDeviceBuffer(cl_mem_flags p_flags, Grid<T>& p_grid) const
		{
			p_grid.deviceBuffer = m_fpga->createDeviceBuffer(p_flags, p_grid.hostBuffer);
		}

		template <typename T>
		std::vector<cl::Buffer> createDeviceBuffer(cl_mem_flags p_flags, const std::vector<host_buffer_t<T> >& p_buffer) const
		{
			return m_fpga->createDeviceBuffer(p_flags, p_buffer);
		}

		template <typename T>
		void createDeviceBuffer(cl_mem_flags p_flags, std::vector<host_buffer_t<T> >& p_grid) const
		{
			for (auto it = p_grid.begin(); it = p_grid.end(); ++it)
			{
				it.deviceBuffer =  m_fpga->createDeviceBuffer(p_flags, it.hostBuffer);
			}
		}

        void recordH2DEvent(cl::Event& event)
        {
            m_h2d_event = event;
        }

        void recordH2DEvent(std::vector<cl::Event> events)
        {
            cl::Event event;
            m_fpga->getCommandQueue().enqueueMarkerWithWaitList(&events, &m_h2d_event);
        }

        void recordExecEvent(cl::Event& event)
        {  
            m_exec_event = event;
        }

        void recordExecEvent(std::vector<cl::Event>& events)
        {
            cl::Event event;
            m_fpga->getCommandQueue().enqueueMarkerWithWaitList(&events, &m_exec_event);
        }

		void registerProfileEvents() 
		{
            m_fpga->registerRuntimeEvents(m_kernel_name, m_h2d_event, m_exec_event);
		}

	protected:
		FPGA* m_fpga;
		std::string m_kernel_name;
	private:

        cl::Event m_h2d_event;
        cl::Event m_exec_event;
};

}
}

#endif /* DOXYGEN_SHOULD_SKIP_THIS */
