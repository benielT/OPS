/*
 * Copyright 2019 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

    pch = strstr(argv, "OPS_BATCH_SIZE=");
    if (pch != NULL) {
        snprintf(temp, 64, "%s", pch);
        instance->setOPSBatchSize((unsigned int)atoi(temp +  strlen("OPS_BATCH_SIZE=")));
        std::cout << "\n OPS Tile size in Y = " << instance->getOPSBatchSize() << '\n';
    }
}

unsigned int ops_get_batch_size()
{
    ops::hls::FPGA * fpga = ops::hls::FPGA::getInstance();
    return fpga->getOPSBatchSize();
}

void ops_init_backend(int argc, const char** argv, unsigned int devId)
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