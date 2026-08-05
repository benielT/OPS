#ifndef __OPS_HLS_FPGA_H
#define __OPS_HLS_FPGA_H

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
  * @brief FPGA handler class headerfile
  * @author Beniel Thileepan (maintainer)
  * @details This class manage FPGA platform interaction with XOCL API and wrapping related objects.
  */


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <iostream>
#include <vector>
#include <regex>
#include <unordered_map>

// This extension file is required for stream APIs
// #include "CL/cl_ext_xilinx.h"
// This file is required for OpenCL C++ wrapper APIs
#include "../../ext/xcl2/xcl2.hpp"
// #include "ops_hls_aurora.hpp"
#include "ops_hls_aurora_link_types.hpp"

#define OPS_HLS_TILE_INTERLEAVE_V2

template <typename T>
using host_buffer_t = std::vector<T, aligned_allocator<T> >;

namespace ops
{
namespace hls
{

/**
 * @brief This is a mock ops_block class to replace
 */
class Block
{
public:
	int dims;
	std::string name;
    int batch_size = 1;
};

typedef std::chrono::system_clock::time_point time_point;
typedef std::chrono::duration<double, std::nano> duration;



class RuntimeEventRecords
{
public:
    RuntimeEventRecords() = default;

    cl::Event kernel_event;
    cl::Event data_HtD_event;
};

//class bad_runtime_record : public std::exception
//{
//public:
//	bad_runtime_record() throw() { }
//
//#if __cplusplus >= 201103L
//  bad_runtime_record(const bad_runtime_record&) = default;
//  bad_runtime_record& operator=(const bad_runtime_record&) = default;
//#endif
//
//  // This declaration is not useless:
//  // http://gcc.gnu.org/onlinedocs/gcc-3.0.2/gcc_6.html#SEC118
//  virtual ~bad_runtime_record() throw();
//
//  // See comment in eh_exception.cc.
//  virtual const char* what() const throw();
//};

class AuroraHandler;    // fwd decl for aororaflow integration

struct AuroraHandlerDeleter { // Added to avoid default unique_ptr to delete which fails on non OPS_HLS_AURORA scenario
    void operator()(AuroraHandler* p) const noexcept;   // declaration only
};

/**
 * @brief This is a singleton class indended to use for single FPGA device handlings with 
 * thread local usage. 
*/
class FPGA {
   public:

    // Prevent cloning
    FPGA(FPGA &other) = delete;
    // Preventing assignment
    void operator=(const FPGA &) = delete;
    ~FPGA();
    static FPGA* getInstance();
//    const uint32_t next() const {
//        if (m_id == m_devices.size() - 1) {
//            return 0;
//        }
//
//        setID(m_id + 1);
//        return (m_id + 1);
//    }
    void setID(uint32_t id);  //Selecting device explicity through logical device ID orer
    bool xclbin(std::string binaryFile); // Programing device, seting device first and calling this would be preferable
    const cl::Context& getContext() const { return m_context; }
    const cl::CommandQueue& getCommandQueue() const { return m_queue; }
    cl::CommandQueue& getCommandQueue() { return m_queue; }
    const cl::Program& getProgram() const { return m_program; }
    void finish() const { m_queue.finish(); }
    template <typename T> cl::Buffer createDeviceBuffer(cl_mem_flags p_flags, const host_buffer_t<T>& p_buffer); // Create device buffer from given host buffer
    template <typename T> std::vector<cl::Buffer> createDeviceBuffer(cl_mem_flags p_flags, const std::vector<host_buffer_t<T> >& p_buffer); // Create device buffers for multiple host buffers
    template <typename T> void deleteDeviceBuffer(const host_buffer_t<T>& p_buffer); // Delete device buffer of the given host buffer
    void registerRuntimeEvents(const std::string& kernel_name, cl::Event& h2d_event, cl::Event& exec_event);
    bool runtimeEventRecExists(const std::string& kernel_name) const { auto it = m_runtimeEvents.find(kernel_name); return it != m_runtimeEvents.end();}

    template<typename DurationType> double getExecutionRuntime(const std::string& kernel_name, const int execId = 0) const{
        if (not runtimeEventRecExists(kernel_name))
            throw std::runtime_error((std::string("bad_runtime_record. Record do not exists for ") + kernel_name).c_str());
        else if (execId >= m_runtimeEvents.at(kernel_name).size())
            throw std::runtime_error((std::string("bad_runtime_record. Record do not exists for ") + kernel_name + " with execId " + std::to_string(execId)).c_str());
        
        m_runtimeEvents.at(kernel_name)[execId].kernel_event.wait();
        return std::chrono::duration_cast<DurationType>(std::chrono::nanoseconds(m_runtimeEvents.at(kernel_name)[execId].kernel_event.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
                                                        m_runtimeEvents.at(kernel_name)[execId].kernel_event.getProfilingInfo<CL_PROFILING_COMMAND_START>())).count();
        
    }

    template<typename DurationType> double getTotalExecutionRuntime(const std::string& kernel_name) const {
        if (not runtimeEventRecExists(kernel_name))
            throw std::runtime_error((std::string("bad_runtime_record. Record do not exists for ") + kernel_name).c_str());
        
        double total_runtime = 0.0;

        
        auto start_record =  m_runtimeEvents.at(kernel_name)[0];
        auto end_record =  m_runtimeEvents.at(kernel_name)[m_runtimeEvents.at(kernel_name).size() - 1];

        if (start_record.kernel_event.getInfo<CL_EVENT_COMMAND_EXECUTION_STATUS>() != CL_COMPLETE) {
            std::cout <<"[WARNING] Waiting for start kernel event to complete." << std::endl;
            start_record.kernel_event.wait();
        }
        
        if (end_record.kernel_event.getInfo<CL_EVENT_COMMAND_EXECUTION_STATUS>() != CL_COMPLETE) {
            std::cout <<"[WARNING] Waiting for end kernel event to complete." << std::endl;
            end_record.kernel_event.wait();
        }

        auto start_time = start_record.kernel_event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        auto end_time = end_record.kernel_event.getProfilingInfo<CL_PROFILING_COMMAND_END>();
        total_runtime = std::chrono::duration_cast<DurationType>(std::chrono::nanoseconds(end_time - start_time)).count();
        return total_runtime;
    }

    template<typename DurationType> double getHtoDRuntime(const std::string& kernel_name, const int execId = 0) const
    {
        if (not runtimeEventRecExists(kernel_name))
            throw std::runtime_error((std::string("bad_runtime_record. Record do not exists for ") + kernel_name).c_str());
        else if (execId >= m_runtimeEvents.at(kernel_name).size())
            throw std::runtime_error((std::string("bad_runtime_record. Record do not exists for ") + kernel_name + " with execId " + std::to_string(execId)).c_str());
        
        m_runtimeEvents.at(kernel_name)[execId].data_HtD_event.wait();
        return std::chrono::duration_cast<DurationType>(std::chrono::nanoseconds(m_runtimeEvents.at(kernel_name)[execId].data_HtD_event.getProfilingInfo<CL_PROFILING_COMMAND_END>() -
                                                        m_runtimeEvents.at(kernel_name)[execId].data_HtD_event.getProfilingInfo<CL_PROFILING_COMMAND_START>())).count();
    }

    void setOPSTiling() { OPS_tiling = true; }
    bool isOPSTiling() { return OPS_tiling; }
    void setOPSTileSizeX(unsigned short tile_x) { OPS_tiling_size_x = tile_x; }
    void setOPSTileSizeY(unsigned short tile_y) { OPS_tiling_size_y = tile_y; }
    void setOPSBatchSize(unsigned int batch_size) { OPS_batch_size = batch_size; }

    unsigned short getOPSTileSizeX() {
        if (isOPSTiling())
            return OPS_tiling_size_x;
        else 
            return 0;
    }

    unsigned short getOPSTileSizeY() {
        if (isOPSTiling())
            return OPS_tiling_size_y;
        else 
            return 0;
    }

    std::string getProgramName() { return m_programName; }
    unsigned int getOPSBatchSize() { return OPS_batch_size; }

//**************** OPS_MULTI_FPGA ********************/
    int getLocalRank() { return m_mpi_local_rank; }
    int getWorldSize() { return m_mpi_world_size; }
    bool isRootRank() const;
    unsigned int resolveDeviceId(unsigned int requested) const;
    void setMPIContext(int global_rank, int world_size, int local_rank, int local_size);
 
//**************** OPS_HLS_AURORA ********************/

    bool initAurora(int rank, int world_size, bool periodic = false);
    bool hasAurora() const  { return m_aurora != nullptr; }
    bool auroraLinkCheck(int timeout_ms = 3000);
    void auroraResetCounters();
    bool auroraInstanceInUse(unsigned instance) const;
    LinkStats auroraSnapshot(LinkDirection dir) const;
    int auroraNeighbourRank(LinkDirection dir) const;

//***************************************************/

//***************************************************/

protected:

    bool bufferExists(const void* p_ptr) const { auto it = m_bufferMaps.find(p_ptr); return it != m_bufferMaps.end(); }
    void getDevices(std::string deviceName);

    FPGA (std::string deviceName);
    // {
    //     getDevices(deviceName);
    // #ifndef OPS_MULTI_FPGA
    //     m_device = m_devices[0];
    //     m_id = 0;
    // #else
    //     extern int ops_comm_global_size;
    //     extern int ops_my_global_rank;
    //     m_id = ops_my_global_rank % m_devices.size();
    //     m_device = m_devices[m_id];
    // #endif
    //     setID(m_id);
    //     OPS_tiling = false;
    //     OPS_tiling_size_x = 0;
    //     OPS_tiling_size_y = 0;
    //     OPS_batch_size = 1;
    // }

    FPGA(unsigned int p_id = 0, std::string deviceName = "");
    //  {
    //     getDevices(deviceName);
    // #ifndef OPS_MULTI_FPGA
    //     setID(p_id);
    // #else
    //     extern int ops_my_global_rank;
    //     setID(ops_my_global_rank % m_devices.size());
    // #endif   
    //     m_device = m_devices[m_id];
    //     OPS_tiling = false;
    //     OPS_tiling_size_x = 0;
    //     OPS_tiling_size_y = 0;
    //     OPS_batch_size = 1;
    // }

    FPGA(unsigned int p_id, const std::vector<cl::Device>& devices);
    //  {
    //     m_devices = devices;
    // #ifndef OPS_MULTI_FPGA
    //     setID(p_id);
    // #else
    //     extern int ops_my_global_rank;
    //     setID(ops_my_global_rank % m_devices.size());
    // #endif
    //     m_device = m_devices[m_id];
    //     OPS_tiling = false;
    //     OPS_tiling_size_x = 0;
    //     OPS_tiling_size_y = 0;
    //     OPS_batch_size = 1;
    // }

    static FPGA* FPGA_;

    unsigned int m_id;
    cl::Device m_device;
    std::vector<cl::Device> m_devices;
    cl::Context m_context;
    cl::CommandQueue m_queue;
    cl::Program m_program;
    std::string m_programName;
    std::unordered_map<const void*, cl::Buffer> m_bufferMaps;
    std::unordered_map<std::string, std::vector<RuntimeEventRecords>> m_runtimeEvents;
    bool OPS_tiling;
    unsigned short OPS_tiling_size_x;
    unsigned short OPS_tiling_size_y;
    unsigned int OPS_batch_size;

    int  m_mpi_global_rank = 0;
    int  m_mpi_local_rank  = 0;
    int  m_mpi_world_size  = 1;
    int  m_mpi_local_size  = 1;
    bool is_root_mpi_rank  = true;   // a single-process run is its own root
    bool m_mpi_initialised = false;

    std::unique_ptr<AuroraHandler, AuroraHandlerDeleter> m_aurora;
};

}
}

void _FPGA_set_args(ops::hls::FPGA *instance, const char *argv);

void ops_init_backend(int argc, char** argv, unsigned int devId = 0);

// template<typename _Period>
// double ops_hls_get_execution_runtime(const std::string&);

// template<typename _Period>
// double ops_hls_get_execution_runtime(const char*);
template<typename DurationType>
double ops_hls_get_execution_runtime(const std::string& kernel_name, const int execId = 0);

template<typename DurationType>
double ops_hls_get_total_execution_runtime(const std::string& kernel_name);

void ops_exit_backend();

unsigned int ops_get_batch_size();

#endif /* DOXYGEN_SHOULD_SKIP_THIS */
#endif /* __OPS_HLS_FPGA_H */
