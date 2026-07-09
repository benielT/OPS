# Makefile_tapa.mk

# Checks for required Xilinx tools
ifndef XILINX_VITIS
$(error XILINX_VITIS variable is not set. Please source the Vitis settings64.sh.)
endif

ifndef XILINX_XRT
$(error XILINX_XRT variable is not set. Please source the XRT setup.sh.)
endif

# ---------------------------------------------------------
# TAPA Submodule Configuration
# ---------------------------------------------------------
TAPA_BASE_DIR = $(OPS_INSTALL_PATH)/../tapa
TAPA_INC_DIR  = $(TAPA_BASE_DIR)/src
TAPA_LIB_DIR  = $(TAPA_BASE_DIR)/build

# TAPAC compiler location. 
# Defaults to the system PATH, but can be overridden if executing from a local virtualenv or build directory.
TAPAC ?= tapac

ifeq (, $(shell which $(TAPAC)))
$(warning WARNING: $(TAPAC) not found in PATH. Ensure the TAPA submodule has been built and the binary is accessible.)
endif

# ---------------------------------------------------------
# Platform and Target Checks
# ---------------------------------------------------------
ifeq ($(PLATFORM),)
$(error PLATFORM not set. Please set the PLATFORM properly and rerun.)
endif

ifeq ($(HLS_JOBS),)
HLS_JOBS = 10
endif

ifeq ($(HLS_TARGET_MODE),)
$(error hls build mode is not set: please set HLS_TARGET_MODE=<sw_emu/hw_emu/hw>)
endif

$(info TAPA target mode: $(HLS_TARGET_MODE) on $(PLATFORM))

# Vitis and Workspace Variables
VPP = v++
TAPA_WORK_DIR = ./tapa/build/$(HLS_TARGET_MODE)/tapa_work_dir

# OPS Include Paths
OPS_HLS_DEVICE_INC = -I$(OPS_INSTALL_PATH)/hls/include/device/ -I$(OPS_INSTALL_PATH)/hls/L1/include/
OPS_HLS_HOST_INC   = -I$(OPS_INSTALL_PATH)/c/include/ -I$(OPS_INSTALL_PATH)/hls/include/host/ -I$(OPS_INSTALL_PATH)/hls/ext/xcl2/ -I$(OPS_INSTALL_PATH)/hls/L1/include/ -I$(OPS_INSTALL_PATH)/hls/L2/include/

# ---------------------------------------------------------
# Compilation & Linker Flags
# ---------------------------------------------------------

# TAPA Device Compilation Flags
# Note: The tapac command requires standard include directives to locate the submodule's tapac.h
TAPA_FLAGS = --platform $(PLATFORM) --work-dir $(TAPA_WORK_DIR) --clock-period 3.33 

# Vitis Linking Flags for .xo to .xclbin phase
VPP_LINK_FLAGS = -t $(HLS_TARGET_MODE) --platform $(PLATFORM) --jobs $(HLS_JOBS)

# Host Executable Flags (Injects the submodule include paths)
TAPA_CXXFLAGS = -I$(TAPA_INC_DIR) -I$(XILINX_XRT)/include/ -I$(XILINX_VIVADO)/include/ -DVITIS_PLATFORM=$(PLATFORM) -DOPS_FPGA -D__USE_XOPEN2K8 -fmessage-length=0 $(OPS_HLS_HOST_INC) -D__VITIS_HLS__

# Host Linker Flags (Injects the submodule library path for libtapa)
TAPA_LDFLAGS = -L$(TAPA_LIB_DIR) -ltapa -lgflags -lglog -lxilinxopencl -lops_seq -lops_hls -lpthread -lrt -lstdc++ -L$(XILINX_XRT)/lib/ -L$(OPS_INSTALL_PATH)/hls/lib/$(OPS_COMPILER)/ -Wl,-rpath-link,$(XILINX_XRT)/lib