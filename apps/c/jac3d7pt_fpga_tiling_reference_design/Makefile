#
# The following environment variables should be predefined:
#
# OPS_INSTALL_PATH
# OPS_COMPILER (gnu,intel,etc)
#

PLATFORM=xilinx_u280_gen3x16_xdma_1_202211_1
# PLATFORM=xilinx_vck5000_gen4x8_xdma_2_202210_1
HLS_TARGET_MODE=sw_emu
HLS_JOBS=64
include $(OPS_INSTALL_PATH)/../makefiles/Makefile_hls.mk

HEADERS=jac3D7pt_kernel.h

OPS_FILES=jac3D7pt.cpp
OPS_FILES_GEN_HLS=jac3D7pt_ops_hls.cpp

OPS_GENERATED_SUFFIX=_ops

OTHER_FILES=


APP=jac3D7pt
MAIN_SRC=jac3D7pt

# OPS_GENERATOR_VERBOSE=1
# OPS_FPGA_ONLY=1 #Enable if building FPGA only mode
ifeq ($(PLATFORM),xilinx_u280_gen3x16_xdma_1_202211_1)
OPS_TARGET_CONFIG_FILE= ./config_u280.json
else
OPS_TARGET_CONFIG_FILE= ./config_vck5000.json
endif


TARGETS=hls #tiled openmp mpi mpi_tiled mpi_openmp cuda mpi_cuda mpi_cuda_tiled hip mpi_hip mpi_hip_tiled ompoffload mpi_ompoffload mpi_ompoffload_tiled sycl mpi_sycl mpi_sycl_tiled openacc mpi_openacc mpi_openacc_tiled

########  PRFOFILE OPTIONS (ONLY ENABLE ONE OPTION) ######

#Enable if wanted to profile
CXXFLAGS += -DPROFILE
#Enable if wanted to power_profile
#CXXFLAGS += -DPOWER_PROFILE

########  VERIIFICATION OPTIONS  ######
#Enable if wanted to verify
#CXXFLAGS += -DVERIFICATION

######## ASYN_DISPATCH OPTION ########
#Enable if wanted to use asynchronous dispatch
# CXXFLAGS += -DASYNC_DISPATCH

######## BATCHING OPTION ########
# Enable if wanted to use batching
# CXXFLAGS += -DBATCHING

######## TILING OPTION ########
# Enable if wanted to use tiling
TILE_SIZE_X=32
TILE_SIZE_Y=32
CXXFLAGS += -DOPS_TILING -DOPS_TILESIZE_X=$(TILE_SIZE_X) -DOPS_TILESIZE_Y=$(TILE_SIZE_Y)
VPP_FLAGS += -DOPS_TILING -DOPS_TILESIZE_X=$(TILE_SIZE_X) -DOPS_TILESIZE_Y=$(TILE_SIZE_Y)

########  DEBUG OPTIONS  ######
#Enable if wanted to debug
# CXXFLAGS += -DDEBUG_LOG
# VPP_FLAGS += -DDEBUG_LOG


# Default target
.DEFAULT_GOAL := $(APP)_hls

$(APP)_hls: Makefile ./hls/build/$(HLS_TARGET_MODE)/$(APP)_host ./hls/build/$(HLS_TARGET_MODE)/$(APP).xclbin
		@echo ""
		@echo ""
		@echo "Building ${APP}_hls"
		@echo ""
		
./hls/build/$(HLS_TARGET_MODE)/$(APP)_host: $(OPS_INSTALL_PATH)/hls/lib/$(OPS_COMPILER)/libops_hls.a $(OPS_INSTALL_PATH)/hls/lib/$(OPS_COMPILER)/libops_seq.a 
		@echo ""
		@echo ""
		@echo "Building ${APP}_hls_host"
		@echo ""
		@echo ""
		test -d ./hls/build/$(HLS_TARGET_MODE) || mkdir -p ./hls/build/$(HLS_TARGET_MODE)
		$(CXX)  -o ./hls/build/$(HLS_TARGET_MODE)/$(APP)_host $(OPS_FILES_GEN_HLS) $(CXXFLAGS) $(HLS_CXXFLAGS) -I./ -I./hls/host/kernel_wrappers/ $(HLS_LDFLAGS)


# TOP_MODULES_XOS := ${patsubst %.cpp, %.xo, ${patsubst hls/device/src/%, hls/build/$(HLS_TARGET_MODE)/%, ${shell ls hls/device/src/*.cpp}}}

./hls/build/$(HLS_TARGET_MODE)/$(APP).xclbin: 
		@echo ""
		@echo ""
		@echo "Building ${APP}_hls.xclbin"
		@echo ""
		test -d hls/build/$(HLS_TARGET_MODE)/temp_dir/link/ || mkdir -p hls/build/$(HLS_TARGET_MODE)/temp_dir/link/

		@for xo_file in ${basename ${notdir ${shell ls ./hls/device/src/*.cpp}}}; do \
			echo "Compiling $${xo_file}"; \
			$(VPP) -s -O3 $(VPP_FLAGS) --temp_dir hls/build/$(HLS_TARGET_MODE)/temp_dir/ --log_dir hls/build/$(HLS_TARGET_MODE)/temp_dir/ --compile $(OPS_HLS_DEVICE_INC) -I./hls/device/include/ -k $${xo_file} -o hls/build/$(HLS_TARGET_MODE)/$${xo_file}.xo  hls/device/src/$${xo_file}.cpp; \
		done

		@if [ -n "$(OPS_HLS_XSA_PLATFORM)" ]; then \
			echo "Building XSA platform"; \
			$(VPP) -s -O3 $(VPP_FLAGS) --temp_dir hls/build/$(HLS_TARGET_MODE)/temp_dir/link/  --log_dir hls/build/$(HLS_TARGET_MODE)/temp_dir/link/logs/ --link --config hls/host/xrt.cfg -o ./hls/build/$(HLS_TARGET_MODE)/$(APP).xsa  ${patsubst %.cpp, %.xo, ${patsubst ./hls/device/src/%, ./hls/build/$(HLS_TARGET_MODE)/%, $(shell ls ./hls/device/src/*.cpp)}}; \
			$(VPP) -s -O3 --target $(HLS_TARGET_MODE) --platform $(PLATFORM) --temp_dir  hls/build/$(HLS_TARGET_MODE)/temp_dir/link/ --log_dir hls/build/$(HLS_TARGET_MODE)/temp_dir/link/logs/ --package  ./hls/build/$(HLS_TARGET_MODE)/$(APP).xsa -o "$@"; \
		else \
			echo "Building non-XSA platform"; \
			$(VPP) -s -O3 $(VPP_FLAGS) --temp_dir hls/build/$(HLS_TARGET_MODE)/temp_dir/link/  --log_dir hls/build/$(HLS_TARGET_MODE)/temp_dir/link/logs/ --link --config hls/host/xrt.cfg -o "$@"  ${patsubst %.cpp, %.xo, ${patsubst ./hls/device/src/%, ./hls/build/$(HLS_TARGET_MODE)/%, $(shell ls ./hls/device/src/*.cpp)}}; \
		fi
		
.PHONY: build_hls_host
build_hls_host: 
	rm -f ./hls/build/$(HLS_TARGET_MODE)/$(APP)_host
	make ./hls/build/$(HLS_TARGET_MODE)/$(APP)_host

.PHONY: run_hls_app
run_hls_app:
	bash ./run_script_hls.sh $(HLS_TARGET_MODE) $(APP) $(PLATFORM) $(CXXFLAGS)

clean:
	rm -rf ./hls/build/
	rm -rf ./hls/profile_data/
