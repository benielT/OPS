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
  * @brief AuroraFlow link management for the OPS HLS multi-FPGA backend
  * @author Beniel Thileepan (maintainer)
  * @details Bridges the xocl-based FPGA handler to the XRT native API objects
  *     AuroraFlow requires (xrt::device, xrt::uuid), without migrating the rest
  *     of the runtime off OpenCL. Owns the per-QSFP-port AuroraFlow instances
  *     and their bring-up / monitoring lifecycle.
  * 
  *     Build requirements:
  *       CXXFLAGS += -I$(XILINX_XRT)/include $(AURORA_FLOW_INCLUDE)
  *       LDFLAGS  += -lxrt_coreutil -luuid          (in addition to -lOpenCL)
  *
  *     Compile with -DOPS_HLS_AURORA to enable. Note this is deliberately a
  *     separate macro from OPS_MULTI_FPGA: a multi-FPGA run may exchange halos
  *     over host-side MPI rather than over the QSFP links.

  */
  
#pragma once

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef OPS_HLS_AURORA

#include <memory>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>

#include <CL/cl2.hpp>
#include <CL/cl_ext_xilinx.h>
#include <xclbin.h>              // axlf
#include <xrt/xrt_device.h>
#include <xrt/xrt_uuid.h>

#if __has_include(<xrt/xrt_xclbin.h>)
  #include <xrt/xrt_xclbin.h>          // XRT 2023.1+
#else
  #include <experimental/xrt_xclbin.h> // XRT 2.14 / Vitis 2022.2
#endif  

#include <AuroraFlow.hpp>
#include "ops_hls_aurora_link_types.hpp"

namespace ops
{
namespace hls
{
namespace aurora_detail
{

/**
 * @brief Wrap the shim handle xocl is already using in an xrt::device.
 *
 * CL_DEVICE_HANDLE returns xocl::device::get_handle() as a void*, so the
 * resulting xrt::device refers to the same underlying core device rather than
 * opening a second one. The handle stays owned by the OpenCL runtime - the
 * cl::Device / cl::Context must outlive every xrt object derived from it.
 */
inline xrt::device xrtDeviceFromCl(const cl::Device& device)
{
    xclDeviceHandle dhdl = nullptr;
    cl_int err = clGetDeviceInfo(device(), CL_DEVICE_HANDLE,
                                 sizeof(dhdl), &dhdl, nullptr);
    if (err != CL_SUCCESS || dhdl == nullptr)
        throw std::runtime_error("[AURORA] CL_DEVICE_HANDLE query failed - "
                                 "XRT OpenCL extension unavailable?");
    return xrt::device{dhdl};
}
 
/**
 * @brief Recover the raw axlf image from the cl::Program.
 *
 * Avoids having to thread the xcl::import_binary_file() buffer through the
 * FPGA class. cl2.hpp's CL_PROGRAM_BINARIES specialisation handles the
 * two-stage size query for us.
 */
inline std::vector<unsigned char> xclbinImageFromProgram(const cl::Program& program)
{
    auto bins = program.getInfo<CL_PROGRAM_BINARIES>();
    if (bins.empty() || bins.at(0).empty())
        throw std::runtime_error("[AURORA] CL_PROGRAM_BINARIES returned nothing");
 
    std::vector<unsigned char> image(bins.at(0).begin(), bins.at(0).end());
    auto top = reinterpret_cast<const axlf*>(image.data());
    if (std::strncmp(top->m_magic, "xclbin2", 7) != 0)
        throw std::runtime_error("[AURORA] program binary is not an xclbin");
    return image;
}
 
inline xrt::uuid uuidFromImage(const std::vector<unsigned char>& image)
{
    auto top = reinterpret_cast<const axlf*>(image.data());
    return xrt::uuid{top->m_header.uuid};
}
 
/**
 * @brief Force xocl to download the bitstream.
 *
 * cl::Program construction alone does not program the device; xrt::ip
 * construction against a uuid that is not yet loaded fails. Creating one
 * cl::Kernel is the cheapest reliable way to force the download. The kernel
 * object is discarded - the CU context xocl opens for it does not conflict
 * with the aurora_flow_hw_* CUs, since contexts are per-CU.
 */
inline void forceXclbinDownload(const cl::Program& program, const xrt::xclbin& xclbin)
{
    for (const auto& k : xclbin.get_kernels())
    {
        const std::string name = k.get_name();
        if (name.rfind("aurora_flow", 0) == 0)
            continue;                       // skip the Aurora RTL kernels
 
        cl_int err = CL_SUCCESS;
        cl::Kernel probe(program, name.c_str(), &err);
        if (err == CL_SUCCESS)
            return;
    }
    throw std::runtime_error("[AURORA] could not instantiate any kernel to "
                             "force xclbin download");
}
 
} // namespace aurora_detail
 
 
/**
 * @brief Owns the AuroraFlow instances for one device and their lifecycle.
 *
 * Constructed after the xclbin is on the card. Probes IP_LAYOUT first, so a
 * single-FPGA xclbin built from the same host library simply reports
 * available() == false rather than throwing.
 */
class AuroraHandler
{
public:
    AuroraHandler(const cl::Device& device, const cl::Program& program,
                  int rank, int world_size, bool periodic = false)
      : m_rank(rank), m_world_size(world_size), m_periodic(periodic)
    {
        auto image = aurora_detail::xclbinImageFromProgram(program);
        m_uuid   = aurora_detail::uuidFromImage(image);
        m_xclbin = xrt::xclbin{reinterpret_cast<const axlf*>(image.data())};
 
        // Probe before doing anything expensive.
        for (const auto& ip : m_xclbin.get_ips())
        {
            const std::string name = ip.get_name();
            if (name.find("aurora_flow_hw_0") != std::string::npos ||
                name.find("aurora_flow_sw_emu") != std::string::npos)
                m_present[0] = true;
            if (name.find("aurora_flow_hw_1") != std::string::npos)
                m_present[1] = true;
        }
 
        if (!m_present[0] && !m_present[1])
        {
            std::cout << "[AURORA] no aurora_flow IPs in xclbin - "
                         "links disabled" << std::endl;
            return;
        }
 
        aurora_detail::forceXclbinDownload(program, m_xclbin);
        m_device = aurora_detail::xrtDeviceFromCl(device);
 
        // Cross-check: catches the case where the download silently did not
        // happen, which would otherwise surface as an opaque xrt::ip failure.
        if (m_device.get_xclbin_uuid() != m_uuid)
            throw std::runtime_error("[AURORA] device uuid does not match "
                                     "program - xclbin not loaded");
 
        for (unsigned i = 0; i < 2; ++i)
        {
            if (!m_present[i] || !instanceInUse(i))
                continue;
            m_aurora[i] = std::make_unique<AuroraFlow>(i, m_device, m_uuid);
        }
        m_available = true;
    }
 
    bool available() const { return m_available; }
 
    /** @brief Is this port actually cabled to a neighbour in this run?
     *  End ranks of a non-periodic decomposition have one unused port; asking
     *  AuroraFlow to wait for channel-up on it would block for the full
     *  timeout and then fail.
     */
    bool instanceInUse(unsigned instance) const
    {
        if (m_world_size < 2) return false;
        if (m_periodic)       return true;
        if (instance == static_cast<unsigned>(LinkDirection::LOWER))
            return m_rank > 0;
        return m_rank < m_world_size - 1;
    }
 
    AuroraFlow& get(LinkDirection dir)
    {
        const unsigned i = static_cast<unsigned>(dir);
        if (!m_aurora[i])
            throw std::runtime_error("[AURORA] instance " + std::to_string(i) +
                                     " not available on rank " +
                                     std::to_string(m_rank));
        return *m_aurora[i];
    }
 
    int neighbourRank(LinkDirection dir) const
    {
        const int delta = (dir == LinkDirection::UPPER) ? 1 : -1;
        const int n = m_rank + delta;
        if (m_periodic) return (n + m_world_size) % m_world_size;
        return (n < 0 || n >= m_world_size) ? -1 : n;
    }
 
    /** @brief Bring-up gate. Call once from ops_init_backend, before any
     *  kernel enqueue, and follow it with a barrier on OPS_MPI_GLOBAL.
     */
    bool linkCheck(int timeout_ms = 3000)
    {
        if (!m_available) return true;
 
        bool ok = true;
        for (unsigned i = 0; i < 2; ++i)
        {
            if (!m_aurora[i]) continue;
            if (!m_aurora[i]->core_status_ok(timeout_ms))
            {
                std::cerr << "[AURORA][rank " << m_rank << "] instance " << i
                          << " failed to come up" << std::endl;
                m_aurora[i]->print_core_status();
                ok = false;
            }
            else if (m_verbose)
            {
                m_aurora[i]->print_configuration();
            }
        }
        return ok;
    }
 
    void resetCounters()
    {
        for (auto& a : m_aurora)
            if (a) a->reset_counter();
    }
 
    LinkStats snapshot(LinkDirection dir)
    {
        LinkStats s;
        const unsigned i = static_cast<unsigned>(dir);
        if (!m_aurora[i]) return s;
        s.frames_with_errors = m_aurora[i]->get_frames_with_errors();
        s.fifo_rx_overflows  = m_aurora[i]->get_fifo_rx_overflow_count();
        s.channel_up         = m_aurora[i]->core_status_ok(0);
        return s;
    }
 
    void setVerbose(bool v) { m_verbose = v; }
 
private:
    xrt::device m_device;
    xrt::uuid   m_uuid;
    xrt::xclbin m_xclbin;
 
    std::unique_ptr<AuroraFlow> m_aurora[2];
    bool m_present[2] = {false, false};
 
    int  m_rank       = 0;
    int  m_world_size = 1;
    bool m_periodic   = false;
    bool m_available  = false;
    bool m_verbose    = false;
};
 
} // namespace hls
} // namespace ops



#endif /* OPS_HLS_AURORA */
#endif /* DOXYGEN_SHOULD_SKIP_THIS */