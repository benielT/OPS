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

#ifdef OPS_HLS_AURORA
#include "../include/ops_hls_aurora.hpp"
#endif 
#include "../include/ops_hls_fpga.hpp"

ops::hls::FPGA* ops::hls::FPGA::FPGA_ = nullptr;

ops::hls::FPGA::~FPGA() {
    m_bufferMaps.clear();
}

ops::hls::FPGA::FPGA(unsigned int p_id, std::string deviceName)
{
    OPS_tiling        = false;
    OPS_tiling_size_x = 0;
    OPS_tiling_size_y = 0;
    OPS_batch_size    = 1;
    m_id              = p_id;
    getDevices(deviceName);
    // Device selection deferred to ops_init_backend(): the MPI context is not
    // known at construction time.
}

ops::hls::FPGA::FPGA(std::string deviceName) : FPGA(0, deviceName) {}

ops::hls::FPGA::FPGA(unsigned int p_id, const std::vector<cl::Device>& devices)
{
    OPS_tiling        = false;
    OPS_tiling_size_x = 0;
    OPS_tiling_size_y = 0;
    OPS_batch_size    = 1;
    m_id              = p_id;
    m_devices         = devices;
}

ops::hls::FPGA* ops::hls::FPGA::getInstance() {
    if (FPGA_ == nullptr)
    {
        FPGA_ = new FPGA();
    }
    return FPGA_;
}

void _FPGA_set_args(ops::hls::FPGA *instance, const char *argv) {
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


unsigned int ops::hls::FPGA::resolveDeviceId(unsigned int requested) const
{
    if (m_mpi_world_size <= 1) return requested;
    if (requested != 0)
        std::cerr << "[WARNING] device id " << requested
                  << " ignored under MPI; using node-local rank "
                  << m_mpi_local_rank << std::endl;
    return static_cast<unsigned int>(m_mpi_local_rank % m_devices.size());
}

void ops::hls::FPGA::setID(uint32_t id) {
        m_id = id;
        if (m_id >= m_devices.size()) {
            std::runtime_error("Device specified by id =  " + std::to_string(m_id) + "  is not found. In Local_rank: " 
                    + std::to_string(m_mpi_local_rank) + " Global_rank: " + std::to_string(m_mpi_local_rank));
            throw;
            }
        m_device = m_devices[m_id];
}

bool ops::hls::FPGA::xclbin(std::string binaryFile) {
    cl_int err;
    // get_xil_devices() is a utility API which will find the xilinx
    // platforms and will return list of devices connected to Xilinx platform
    std::string cl_device_name;
    OCL_CHECK(err, err = m_device.getInfo(CL_DEVICE_NAME, &cl_device_name));
    std::cout << "programing device: " << cl_device_name << std::endl;
    // Creating Context
    OCL_CHECK(err, m_context = cl::Context(m_device, NULL, NULL, NULL, &err));

    // Creating Command Queue
    OCL_CHECK(err,
                m_queue = cl::CommandQueue(m_context, m_device,
                                            CL_QUEUE_PROFILING_ENABLE | CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err));
    // read_binary_file() is a utility API which will load the binaryFile
    // and will return the pointer to file buffer.
    cl::Program::Binaries bins = xcl::import_binary_file(binaryFile);

    // Creating Program
    OCL_CHECK(err, m_program = cl::Program(m_context, {m_device}, bins, NULL, &err));
    m_programName = binaryFile;
    return true;
}

template <typename T>
cl::Buffer ops::hls::FPGA::createDeviceBuffer(cl_mem_flags p_flags, const host_buffer_t<T>& p_buffer) {
#if !defined(TAPA_SW_EMU) && !defined(TAPA_HW_EMU)
        const void* l_ptr = (const void*)p_buffer.data();
        if (bufferExists(l_ptr)) return m_bufferMaps[l_ptr];

        size_t l_bufferSize = sizeof(T) * p_buffer.size();
        cl_int err;
        m_bufferMaps.insert(
            {l_ptr, cl::Buffer(m_context, p_flags, l_bufferSize, (void*)p_buffer.data(), &err)});
        if (err != CL_SUCCESS) {
            printf("Failed to allocate device buffer!\n");
            throw std::bad_alloc();
        }
#ifdef DEBUG_LOG
        else {
            printf("[FPGA Buffer allocated] {\n");
            printf("  host_ptr  : %p\n",        l_ptr);
            printf("  size      : %zu bytes\n", l_bufferSize);
            printf("  size_mb   : %.3f MB\n",   l_bufferSize / (1024.0 * 1024.0));
            printf("  elements  : %zu\n",       p_buffer.size());
            printf("  type_size : %zu bytes\n", sizeof(T));
            printf("  cl_flags  : 0x%lx\n",    (unsigned long)p_flags);
            printf("  cached    : false\n");
            printf("}\n");
        }
#endif
        return m_bufferMaps[l_ptr];
#else
        return cl::Buffer();
#endif
}

template <typename T>
std::vector<cl::Buffer> ops::hls::FPGA::createDeviceBuffer(cl_mem_flags p_flags, const std::vector<host_buffer_t<T> >& p_buffer) {
#if !defined(TAPA_SW_EMU) && !defined(TAPA_HW_EMU)
        size_t p_hbm_pc = p_buffer.size();
        std::vector<cl::Buffer> l_buffer(p_hbm_pc);
        for (int i = 0; i < p_hbm_pc; i++) {
            l_buffer[i] = createDeviceBuffer(p_flags, p_buffer[i]);
        }
        return l_buffer;
#else
        return std::vector<cl::Buffer>();
#endif
}

template <typename T>
void ops::hls::FPGA::deleteDeviceBuffer(const host_buffer_t<T>& p_buffer)
    {
#if !defined(TAPA_SW_EMU) && !defined(TAPA_HW_EMU)
    	const void* l_ptr = (const void*)p_buffer.data();
		if (bufferExists(l_ptr))
		{
			m_bufferMaps.erase(l_ptr);
		}
#endif
    }
//************************************** Muti-FPGA Components *************************/
bool ops::hls::FPGA::isRootRank() const {
    if (!m_mpi_initialised)
        std::cerr << "[WARNING] isRootRank() before setMPIContext()\n";
    return is_root_mpi_rank;
}

void ops::hls::FPGA::setMPIContext(int global_rank, int world_size, int local_rank, int local_size) {
        m_mpi_global_rank = global_rank;
        m_mpi_world_size  = world_size;
        m_mpi_local_rank  = local_rank;
        m_mpi_local_size  = local_size;
        is_root_mpi_rank  = (global_rank == 0);
        m_mpi_initialised = true;
    }

//************************************ Aurora Handler Components ****************************/


bool ops::hls::FPGA::initAurora(int rank, int world_size, bool periodic){
#ifdef OPS_HLS_AURORA
    m_aurora.reset(new AuroraHandler(m_device, m_program,
                                     rank, world_size, periodic));
    if (!m_aurora->available()) m_aurora.reset();   // no aurora IPs in this xclbin
    return m_aurora != nullptr;
#else
    (void)rank; (void)world_size; (void)periodic;
    std::cerr << "[WARNING][AURORA] library built without OPS_HLS_AURORA; "
                 "QSFP links unavailable" << std::endl;
    return false;
#endif
}

void ops::hls::AuroraHandlerDeleter::operator()(AuroraHandler* p) const noexcept
{
#ifdef OPS_HLS_AURORA
    delete p;                 
#else
    (void)p;                  
#endif
}

bool ops::hls::FPGA::auroraLinkCheck(int timeout_ms) {
#ifdef OPS_HLS_AURORA
    return m_aurora ? m_aurora->linkCheck(timeout_ms) : true;
#else
    (void)timeout_ms; return true;
#endif
}

void ops::hls::FPGA::auroraResetCounters()
{
#ifdef OPS_HLS_AURORA
    if (m_aurora) m_aurora->resetCounters();
#endif
}

bool ops::hls::FPGA::auroraInstanceInUse(unsigned instance) const
{
#ifdef OPS_HLS_AURORA
    return m_aurora ? m_aurora->instanceInUse(instance) : false;
#else
    (void)instance; return false;
#endif
}

ops::hls::LinkStats ops::hls::FPGA::auroraSnapshot(LinkDirection dir) const
{
#ifdef OPS_HLS_AURORA
    return m_aurora ? m_aurora->snapshot(dir) : LinkStats{};
#else
    (void)dir; return LinkStats{};
#endif
}

int ops::hls::FPGA::auroraNeighbourRank(LinkDirection dir) const
{
#ifdef OPS_HLS_AURORA
    return m_aurora ? m_aurora->neighbourRank(dir) : -1;
#else
    (void)dir; return -1;
#endif
}

//************************************** Runtime Related Components *************************/

void ops::hls::FPGA::registerRuntimeEvents(const std::string& kernel_name, cl::Event& h2d_event, cl::Event& exec_event) {
#ifdef DEBUG_LOG
    printf("registering runtime for %s\n", kernel_name.c_str());
#endif
    if (not runtimeEventRecExists(kernel_name))
    {
        std::vector<RuntimeEventRecords> newrecord;
        m_runtimeEvents[kernel_name] = newrecord;
    }

    RuntimeEventRecords new_record;
    new_record.data_HtD_event = h2d_event;
    new_record.kernel_event = exec_event;
    m_runtimeEvents[kernel_name].push_back(new_record);
}

//*************************** Protected Member Functions **************************/

void ops::hls::FPGA::getDevices(std::string deviceName) {
    cl_int err;
    auto devices = xcl::get_xil_devices();
    auto regexStr = std::regex(".*" + deviceName + ".*");
    for (auto device : devices) {
        std::string cl_device_name;
        OCL_CHECK(err, err = device.getInfo(CL_DEVICE_NAME, &cl_device_name));
        std::cout << "Found device: " << cl_device_name << std::endl;
        if (regex_match(cl_device_name, regexStr)) m_devices.push_back(device);
    }
    if (0 == m_devices.size()) {
        std::runtime_error("No device found in Local_Rank: " + std::to_string(m_mpi_local_rank) + " Global_rank: " + std::to_string(m_mpi_local_rank));
        throw;
    }
}

unsigned int ops_get_batch_size()
{
    ops::hls::FPGA * fpga = ops::hls::FPGA::getInstance();
    return fpga->getOPSBatchSize();
}

void ops_init_backend(int argc, char** argv, unsigned int devId)
{
    unsigned int deviceId = devId;

    ops::hls::FPGA * fpga = ops::hls::FPGA::getInstance();
   

#if !defined(TAPA_SW_EMU) && !defined(TAPA_HW_EMU)
    std::string xclbinFile = argv[1];
    fpga->setID(fpga->resolveDeviceId(devId));

    if(!fpga->xclbin(xclbinFile))
    {
        std::cerr << "[ERROR] Couldn't program fpga. exit" << std::endl;
		throw;
    }

    for (int n = 1; n < argc; n++) {
        _FPGA_set_args(fpga, argv[n]);
    }
#endif
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