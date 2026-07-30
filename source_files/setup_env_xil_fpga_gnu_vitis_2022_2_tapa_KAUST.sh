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
export PLATFORM_PATH=/opt/xilinx/platforms/

# 3: ############### OPS SPECIFICS SETUP ###############

export OPS_COMPILER=gnu

if [[ -n "$OPS_HLS_ARTIFACT_DIR" ]]; then
    export OPS_INSTALL_PATH=$OPS_HLS_ARTIFACT_DIR/ops
else
    export OPS_INSTALL_PATH=$SCRIPT_DIR/../ops
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


# 5: ############# CHECK TAPA ##########################
# tapa folder should be there in ~/.tapa or else in /opt/tapa
if [ -d "${HOME}/.tapa" ]; then
    export TAPA_INSTALL_DIR="${HOME}/.tapa"
elif [ -d "/opt/tapa" ]; then
    export TAPA_INSTALL_DIR="/opt/tapa"
else
    echo "Tapa not yet installed. Installing now..."
    export TAPA_INSTALL_DIR="${HOME}/.tapa"
    source $OPS_INSTALL_PATH/../tapa/install.sh
fi
