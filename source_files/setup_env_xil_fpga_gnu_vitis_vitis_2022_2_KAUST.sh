#!/bin/bash

#####################
# OPS setup script for Vitis 2022.2 for KAUST HPC nodes.
# Author: Beniel.Thileepan@warwick.ac.uk
####################
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# 1: ############# VITIS SPECIFIC SETUP ################

source /tools/Xilinx/Vivado/2022.2/settings64.sh
source /tools/Xilinx/Vitis/2022.2/settings64.sh
source /tools/Xilinx/Vitis_HLS/2022.2/settings64.sh
export XILINXD_LICENSE_FILE=21001@wthflexsr02.kaust.edu.sa
source /opt/xilinx/xrt/setup.sh

# 2: ############# CPATH and INCLUDE SETUP #############

export CPATH=/usr/include/x86_64-linux-gnu/
VIVADO_INCLUDE_PATH=/tools/Xilinx/Vivado/2022.2/include/
VITIS_INCLUDE_PATH=/tools/Xilinx/Vitis/2022.2/include/
VITIS_HLS_INCLUDE_PATH=/tools/Xilinx/Vitis_HLS/2022.2/include/
export C_INCLUDE_PATH=${VITIS_INCLUDE_PATH}:${VIVADO_INCLUDE_PATH}:${VITIS_HLS_INCLUDE_PATH}
export CPP_INCLUDE_PATH=${VITIS_INCLUDE_PATH}:${VIVADO_INCLUDE_PATH}:${VITIS_HLS_INCLUDE_PATH}
export CPLUS_INCLUDE_PATH=${VITIS_INCLUDE_PATH}:${VIVADO_INCLUDE_PATH}:${VITIS_HLS_INCLUDE_PATH}

# 3: ############### OPS SPECIFICS SETUP ###############

export OPS_COMPILER=gnu

if [[ -n "$OPS_HLS_ARTIFACT_DIR" ]]; then
    export OPS_INSTALL_PATH=$OPS_HLS_ARTIFACT_DIR/OPS/ops
else
    export OPS_INSTALL_PATH=$SCRIPT_DIR/../OPS/ops
fi
export C_INCLUDE_PATH=${OPS_INSTALL_PATH}c/include/:$C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH=${OPS_INSTALL_PATH}c/include/:$CPLUS_INCLUDE_PATH
export CPP_INCLUDE_PATH=${OPS_INSTALL_PATH}c/include/:$CPP_INCLUDE_PATH

# 4: ############ PYTHON VIRTUAL ENV SETUP #############

if [ -f ${OPS_INSTALL_PATH}/../ops_translator/ops_venv/bin/activate ]; then
    source ${OPS_INSTALL_PATH}/../ops_translator/ops_venv/bin/activate
else
    source ${OPS_INSTALL_PATH}/../ops_translator/setup_venv.sh
fi
