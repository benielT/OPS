# Makefile_tapa.mk

# Checks for required Xilinx tools
XSA_PLATFORMS := vck5000 v80

# Checks for XILINX_VITIS
ifndef XILINX_VITIS
$(error XILINX_VITIS variable is not set. Please set it using "source <Vitis_install_path>/Vitis/<Version>/settings64.sh" and rerun.)
endif

# Checks for XILINX_VITIS
ifndef XILINX_HLS
$(error XILINX_HLS variable is not set. Please set it using "source <Vitis_install_path>/Vitis/<Version>/settings64.sh" and rerun.)
endif

# Checks for XILINX_XRT
ifndef XILINX_XRT
$(error XILINX_XRT variable is not set. Please set it using "source /opt/xilinx/xrt/setup.sh" and rerun.)
endif

# Check PLATFORM 
ifeq ($(PLATFORM),)
ifneq ($(DEVICE),)
$(warning WARNING: DEVICE is deprecated in make command. Please use PLATFORM instead.)
PLATFORM := $(DEVICE)
else
$(error PLATFORM not set. Please set the PLATFORM properly and rerun.)
endif
else
$(info hls platform: $(PLATFORM))
endif

# define OPS_HLS_XSA_PLATFORM if PLATFORM is in XSA_PLATFORMS
$(foreach P,$(XSA_PLATFORMS), \
	$(if $(findstring $(P),$(PLATFORM)), \
		$(eval OPS_HLS_XSA_PLATFORM := 1) \
	) \
)

ifdef OPS_HLS_XSA_PLATFORM
$(info Found XSA platform)
else
$(info Found non XSA platform)
endif

# ---------------------------------------------------------
# TAPA Submodule Configuration
# ---------------------------------------------------------
TAPA_BASE_DIR = $(OPS_INSTALL_PATH)/hls
TAPA_INC_DIR  = $(TAPA_BASE_DIR)/include
TAPA_LIB_DIR  = $(TAPA_BASE_DIR)/build


#check tapa installed or not
ifeq ($(TAPA_INSTALL_DIR),)
$(error TAPA_INSTALL_DIR not set. Please set the TAPA_INSTALL_DIR properly and rerun.)
endif 

TAPA_INCLUDE_DIR=$(TAPA_INSTALL_DIR)/usr/include
TAPA_LIBRARY_DIR=$(TAPA_INSTALL_DIR)/usr/lib

# TAPAC compiler location. 
# Defaults to the system PATH, but can be overridden if executing from a local virtualenv or build directory.
TAPAC ?= tapacc

ifeq (, $(shell which $(TAPAC)))
$(error ERROR: $(TAPAC) not found in PATH. Ensure the TAPA submodule has been built and the binary is accessible.)
endif

TAPA ?= tapa
ifeq (, $(shell which $(TAPA)))
$(error  ERROR: $(TAPA) not found in PATH. Ensure the TAPA submodule has been built and the binary is accessible.)
endif

# ---------------------------------------------------------
# Platform and Target Checks
# ---------------------------------------------------------
ifeq ($(PLATFORM),)
$(error PLATFORM not set. Please set the PLATFORM properly and rerun.)
else ifeq ($(PLATFORM_PATH),)
$(error PLATFORM_PATH not set. Please set the PLATFORM_PATH properly and rerun.)
endif

ifeq ($(HLS_JOBS),)
HLS_JOBS = 10
endif

TAPA_CXXFLAGS =
TAPA_ADDITIONAL_FLAGS = 
ifeq ($(HLS_TARGET_MODE),sw_emu)
$(error [ERROR] tapa hls do not support SW_EMU switching to TAPA_SW_EMU)
else ifeq ($(HLS_TARGET_MODE),tapa_sw_emu)
$(info tapa hls mode: TAPA_SW_EMU)
TAPA_CXXFLAGS += -DTAPA_SW_EMU
TAPA_ADDITIONAL_FLAGS += -DTAPA_SW_EMU
else ifeq ($(HLS_TARGET_MODE),tapa_hw_emu)
$(info tapa hls mode: TAPA_HW_EMU)
TAPA_CXXFLAGS += -DTAPA_HW_EMU
TAPA_ADDITIONAL_FLAGS += -DTAPA_HW_EMU
else ifeq ($(HLS_TARGET_MODE),hw_emu)
$(info tapa hls mode: HW_EMU)
else ifeq ($(HLS_TARGET_MODE),hw)
$(info tapa hls mode: HW)
else
$(error hls build mode is not set: please set HLS_TARGET_MODE=<tapa_sw_emu/tapa_hw_emu/hw_emu/hw>)
endif

ifeq ($(TAPA_CLOCK_PERIOD),)
TAPA_CLOCK_PERIOD = 3.33
endif 

ifeq ($(TAPA_WORK_DIR),)
TAPA_WORK_DIR = ./hls/build/$(HLS_TARGET_MODE)/tapa_work_dir
endif

$(info TAPA target mode: $(HLS_TARGET_MODE) on $(PLATFORM))

# Vitis and Workspace Variables
VPP = v++

# OPS Include Paths
TAPA_HLS_DEVICE_INC = -I$(OPS_INSTALL_PATH)/hls/include/device/ -I$(OPS_INSTALL_PATH)/hls/L1/include/
ifeq ($(HLS_TARGET_MODE),tapa_sw_emu)
TAPA_HLS_HOST_INC   = -I$(OPS_INSTALL_PATH)/c/include/ -I$(OPS_INSTALL_PATH)/hls/include/host/
else ifeq ($(HLS_TARGET_MODE),tapa_hw_emu)
TAPA_HLS_HOST_INC   = -I$(OPS_INSTALL_PATH)/c/include/ -I$(OPS_INSTALL_PATH)/hls/include/host/
else
TAPA_HLS_HOST_INC   = -I$(OPS_INSTALL_PATH)/c/include/ -I$(OPS_INSTALL_PATH)/hls/include/host/ -I$(OPS_INSTALL_PATH)/hls/ext/xcl2/ -I$(OPS_INSTALL_PATH)/hls/L1/include/ -I$(OPS_INSTALL_PATH)/hls/L2/include/
endif
# ---------------------------------------------------------
# Compilation & Linker Flags
# ---------------------------------------------------------

# TAPA Device Compilation Flags
# Note: The tapac command requires standard include directives to locate the submodule's tapac.h
TAPA_FLAGS = --platform $(PLATFORM_PATH)/$(PLATFORM) --clock-period $(TAPA_CLOCK_PERIOD)
# Vitis Linking Flags for .xo to .xclbin phase
VPP_LINK_FLAGS= --target $(HLS_TARGET_MODE) --platform $(PLATFORM) --hls.jobs $(HLS_JOBS) --remote_ip_cache $(HLS_IP_CACHE_DIR)

# Host Executable Flags (Injects the submodule include paths)
TAPA_CXXFLAGS += -I$(TAPA_INC_DIR) -I$(XILINX_XRT)/include/ -I$(XILINX_VIVADO)/include/ -I$(TAPA_INCLUDE_DIR) -DVITIS_PLATFORM=$(PLATFORM) -DOPS_FPGA -D__USE_XOPEN2K8 -I$(XILINX_HLS)/include/ -fmessage-length=0 $(TAPA_HLS_HOST_INC) -D__VITIS_HLS__


# Host Linker Flags (Injects the submodule library path for libtapa)
TAPA_LDFLAGS = -L$(TAPA_LIB_DIR) -lxilinxopencl -lops_seq -lops_hls -lpthread -lrt -lstdc++ -L$(XILINX_XRT)/lib/ -L$(OPS_INSTALL_PATH)/hls/lib/$(OPS_COMPILER)/ -L$(TAPA_LIBRARY_DIR) -Wl,-rpath-link,$(XILINX_XRT)/lib