# Makefile for jac2D9pt application
# WARNING: THIS IS A STANDALONE OPS HLS APPLICATION MAKEFILE

PLATFORM_PATH=/opt/software/FPGA/Xilinx/platforms
PLATFORM=xilinx_u280_gen3x16_xdma_1_202211_1
# PLATFORM=xilinx_vck5000_gen4x8_xdma_2_202210_1
HLS_TARGET_MODE=hw_emu
HLS_JOBS=64
include $(OPS_INSTALL_PATH)/../makefiles/Makefile_tapa.mk


HEADERS=jac2D9pt_kernel.h

OPS_FILES_GEN_HLS=jac2D9pt_ops_hls.cpp

OPS_GENERATED_SUFFIX=_ops

OTHER_FILES=


APP=jac2D9pt
MAIN_SRC=jac2D9pt

# OPS_GENERATOR_VERBOSE=1
# OPS_FPGA_ONLY=1 #Enable if building FPGA only mode
ifeq ($(PLATFORM),xilinx_u280_gen3x16_xdma_1_202211_1)
OPS_TARGET_CONFIG_FILE= ./config_u280.json
else
OPS_TARGET_CONFIG_FILE= ./config_vck5000.json
endif

########  PRFOFILE OPTIONS (ONLY ENABLE ONE OPTION) ######

### Enable if wanted to profile
CXXFLAGS += -DPROFILE
### Enable if wanted to power_profile
# CXXFLAGS += -DPOWER_PROFILE

########  VERIIFICATION OPTION  ######
#Enable if wanted to verify
# CXXFLAGS += -DVERIFICATION

######## ASYN_DISPATCH OPTION ########
#Enable if wanted to use asynchronous dispatch
# CXXFLAGS += -DASYNC_DISPATCH

####### BATCHING OPTION ########
# Enable if wanted to use batching
CXXFLAGS += -DBATCHING

######### DEBUG LOGGING OPTION ######## 
# CXXFLAGS += -DDEBUG_LOG
# VPP_FLAGS += -DDEBUG_LOG

# Default target
.DEFAULT_GOAL := $(APP)_tapa

$(APP)_tapa: Makefile ./hls/build/$(HLS_TARGET_MODE)/$(APP)_host ./hls/build/$(HLS_TARGET_MODE)/$(APP).xclbin
		@echo ""
		@echo ""
		@echo "Building ${APP}_tapa"
		@echo ""
		
./hls/build/$(HLS_TARGET_MODE)/$(APP)_host: $(OPS_INSTALL_PATH)/hls/lib/$(OPS_COMPILER)/libops_hls.a $(OPS_INSTALL_PATH)/hls/lib/$(OPS_COMPILER)/libops_seq.a 
		@echo ""
		@echo ""
		@echo "Building ${APP}_hls_host"
		@echo ""
		@echo ""
		test -d ./hls/build/$(HLS_TARGET_MODE) || mkdir -p ./hls/build/$(HLS_TARGET_MODE)
		$(CXX)  -o ./hls/build/$(HLS_TARGET_MODE)/$(APP)_host $(OPS_FILES_GEN_HLS) $(CXXFLAGS) $(TAPA_CXXFLAGS) -I./ -I./hls/host/kernel_wrappers/ $(TAPA_LDFLAGS)


# TOP_MODULES_XOS := ${patsubst %.cpp, %.xo, ${patsubst hls/device/src/%, hls/build/$(HLS_TARGET_MODE)/%, ${shell ls hls/device/src/*.cpp}}}

./hls/build/$(HLS_TARGET_MODE)/$(APP).xclbin: 
		@echo ""
		@echo ""
		@echo "Building ${APP}_hls.xclbin"
		@echo ""
		test -d $(TAPA_WORK_DIR) || mkdir -p $(TAPA_WORK_DIR)
		test -d hls/build/$(HLS_TARGET_MODE)/temp_dir || mkdir -p hls/build/$(HLS_TARGET_MODE)/temp_dir
		@for xo_file in ${basename ${notdir ${shell ls ./hls/device/src/*.cpp}}}; do \
			if [ ! -f "hls/build/$(HLS_TARGET_MODE)/$${xo_file}.xo" ]; then \
				echo "Compiling $${xo_file}"; \
# 				echo "TAPA_FLAGS: $(TAPA_FLAGS)"; \
				$(TAPA) -w $(TAPA_WORK_DIR) compile --top $${xo_file} $(TAPA_FLAGS) -o hls/build/$(HLS_TARGET_MODE)/$${xo_file}.xo -f ./hls/device/src/$${xo_file}.cpp $(foreach flag,$(TAPA_HLS_DEVICE_INC) -I./hls/device/include/,--cflags $(flag)); \
			fi \
		done

		@if [ -n "$(OPS_HLS_XSA_PLATFORM)" ]; then \
			echo "Building XSA platform"; \
			$(VPP) --link -s -O3 $(VPP_LINK_FLAGS) --temp_dir=hls/build/$(HLS_TARGET_MODE)/temp_dir/link --log_dir=hls/build/$(HLS_TARGET_MODE)/temp_dir/link/logs --config hls/host/xrt.cfg -o ./hls/build/$(HLS_TARGET_MODE)/$(APP).xsa  ${patsubst %.cpp, %.xo, ${patsubst ./hls/device/src/%, ./hls/build/$(HLS_TARGET_MODE)/%, $(shell ls ./hls/device/src/*.cpp 2>/dev/null)}}; \
			$(VPP) --package -s -O3 --target $(HLS_TARGET_MODE) --platform $(PLATFORM) --temp_dir=hls/build/$(HLS_TARGET_MODE)/temp_dir/link --log_dir=hls/build/$(HLS_TARGET_MODE)/temp_dir/link/logs ./hls/build/$(HLS_TARGET_MODE)/$(APP).xsa -o "$@"; \
		else \
			echo "Building non-XSA platform"; \
			$(VPP) --link -s -O3 $(VPP_LINK_FLAGS) --temp_dir=hls/build/$(HLS_TARGET_MODE)/temp_dir/link --log_dir=hls/build/$(HLS_TARGET_MODE)/temp_dir/link/logs --config hls/host/xrt.cfg -o "$@"  ${patsubst %.cpp, %.xo, ${patsubst ./hls/device/src/%, ./hls/build/$(HLS_TARGET_MODE)/%, $(shell ls ./hls/device/src/*.cpp 2>/dev/null)}}; \
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
	rm -rf ./work.out/
