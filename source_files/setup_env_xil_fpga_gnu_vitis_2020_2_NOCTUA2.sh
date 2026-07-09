#!/bin/bash

#####################
# OPS setup script for Vitis 2020.2 for NOCTUNA2 PC2 cluster nodes.
# Author: Beniel.Thileepan@warwick.ac.uk
####################

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# 1: ############# ENV SETUP ################

module reset

module load fpga
module load xilinx/xrt/2.8

module load compiler/GCC/11.3.0
module load lang/Python/3.10.4-GCCcore-11.3.0

export XILINX_LOCAL_USER_DATA=$HOME/.Xilinx

# 2: ############### OPS SPECIFICS SETUP ###############

export OPS_COMPILER=gnu
export OPS_INSTALL_PATH=$SCRIPT_DIR/../ops
export C_INCLUDE_PATH=${OPS_INSTALL_PATH}c/include/:$C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH=${OPS_INSTALL_PATH}c/include/:$CPLUS_INCLUDE_PATH
export CPP_INCLUDE_PATH=${OPS_INSTALL_PATH}c/include/:$CPP_INCLUDE_PATH

# 3: ############ PYTHON VIRTUAL ENV SETUP #############

if [ -f ${OPS_INSTALL_PATH}/../ops_translator/ops_venv/bin/activate ]; then
    source ${OPS_INSTALL_PATH}/../ops_translator/ops_venv/bin/activate
else
    source ${OPS_INSTALL_PATH}/../ops_translator/setup_venv.sh
fi

# 4: ############ GRAPHVIZ SETUP #############

module load vis/Graphviz/5.0.0-GCCcore-11.3.0
