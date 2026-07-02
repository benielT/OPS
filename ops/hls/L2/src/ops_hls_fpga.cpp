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

#include "../include/ops_hls_fpga.hpp"

ops::hls::FPGA* ops::hls::FPGA::FPGA_ = nullptr;

ops::hls::FPGA* ops::hls::FPGA::getInstance()
{
    if (FPGA_ == nullptr)
    {
        FPGA_ = new FPGA();
    }
    return FPGA_;
}
void _FPGA_set_args(ops::hls::FPGA *instance, const char *argv)
{
    char temp[64];
    const char *pch;
    pch = strstr(argv, "OPS_TILING");
    if (pch != NULL) {
        instance->setOPSTiling();
        std::cout << "OPS_TILING enabled at runtime" << std::endl;
    }

    pch = strstr(argv, "OPS_TILESIZE_X=");
    if (pch != NULL) {
        snprintf(temp, 64, "%s", pch);
        instance->setOPSTileSizeX((unsigned short)atoi(temp +  strlen("OPS_TILESIZE_X=")));
        std::cout << "\n OPS Tile size in X = " << instance->getOPSTileSizeX() << '\n';
    }

    pch = strstr(argv, "OPS_TILESIZE_Y=");
    if (pch != NULL) {
        snprintf(temp, 64, "%s", pch);
        instance->setOPSTileSizeY((unsigned short)atoi(temp +  strlen("OPS_TILESIZE_Y=")));
        std::cout << "\n OPS Tile size in Y = " << instance->getOPSTileSizeY() << '\n';
    }
}

void ops_init_backend(int argc, char** argv, unsigned int devId)
{
    std::string xclbinFile = argv[1];

    unsigned int deviceId = devId;

    ops::hls::FPGA * fpga = ops::hls::FPGA::getInstance();
    fpga->setID(deviceId);

    if(!fpga->xclbin(xclbinFile))
    {
        std::cerr << "[ERROR] Couldn't program fpga. exit" << std::endl;
		throw;
    }

    for (int n = 1; n < argc; n++) {
        _FPGA_set_args(fpga, argv[n]);
    }
}

template<typename DurationType>
double ops_hls_get_execution_runtime(const std::string& kernel_name, const int execId)
{
    ops::hls::FPGA * fpga = ops::hls::FPGA::getInstance();
    return fpga->getExecutionRuntime<DurationType>(kernel_name, execId);
}

void ops_exit_backend()
{
	  auto fpga_inst = ops::hls::FPGA::getInstance();
	  fpga_inst->finish();
}

template<typename DurationType>
double ops_hls_get_total_execution_runtime(const std::string& kernel_name)
{
    ops::hls::FPGA * fpga = ops::hls::FPGA::getInstance();
    return fpga->getTotalExecutionRuntime<DurationType>(kernel_name);
}

template double ops_hls_get_execution_runtime<std::chrono::microseconds>(const std::string& kernel_name, const int execId);
template double ops_hls_get_execution_runtime<std::chrono::nanoseconds>(const std::string& kernel_name, const int execId);
template double ops_hls_get_execution_runtime<std::chrono::milliseconds>(const std::string& kernel_name, const int execId);
template double ops_hls_get_execution_runtime<std::chrono::seconds>(const std::string& kernel_name, const int execId);

template double ops_hls_get_total_execution_runtime<std::chrono::microseconds>(const std::string& kernel_name);
template double ops_hls_get_total_execution_runtime<std::chrono::nanoseconds>(const std::string& kernel_name);
template double ops_hls_get_total_execution_runtime<std::chrono::milliseconds>(const std::string& kernel_name);
template double ops_hls_get_total_execution_runtime<std::chrono::seconds>(const std::string& kernel_name);