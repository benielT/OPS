#!/bin/bash

# Define arrays for the discrete parameter options
MEM_DATA_WIDTH_OPTS=(256 512)
DATA_WIDTH_OPTS=(16 32 64)
OVERLAP_SIZE_OPTS=(1 2 3 4)
NUM_STREAMS_OPTS=(2 4 8 16)
PLATFORM="/opt/software/FPGA/Xilinx/platforms/xilinx_u280_gen3x16_xdma_1_202211_1/xilinx_u280_gen3x16_xdma_1_202211_1.xpfm"

# Select random indices for the arrays
MEM_DATA_WIDTH=${MEM_DATA_WIDTH_OPTS[$RANDOM % ${#MEM_DATA_WIDTH_OPTS[@]}]}
DATA_WIDTH=${DATA_WIDTH_OPTS[$RANDOM % ${#DATA_WIDTH_OPTS[@]}]}
OVERLAP_SIZE=${OVERLAP_SIZE_OPTS[$RANDOM % ${#OVERLAP_SIZE_OPTS[@]}]}
NUM_STREAMS=${NUM_STREAMS_OPTS[$RANDOM % ${#NUM_STREAMS_OPTS[@]}]}

# Generate random value for NUM_PKTS between 10 and 50 inclusive
NUM_PKTS=$((10 + RANDOM % 41))

# Format the compiler definitions
export USER_CFLAGS="-DCFG_MEM_DATA_WIDTH=$MEM_DATA_WIDTH \
-DCFG_DATA_WIDTH=$DATA_WIDTH \
-DCFG_OVERLAP_SIZE=$OVERLAP_SIZE \
-DCFG_NUM_STREAMS=$NUM_STREAMS \
-DCFG_NUM_PKTS=$NUM_PKTS"

echo "========================================"
echo " Starting Random Test Iteration         "
echo "========================================"
echo "  MEM_DATA_WIDTH = $MEM_DATA_WIDTH"
echo "  DATA_WIDTH     = $DATA_WIDTH"
echo "  OVERLAP_SIZE   = $OVERLAP_SIZE"
echo "  NUM_STREAMS    = $NUM_STREAMS"
echo "  NUM_PKTS       = $NUM_PKTS"
echo "----------------------------------------"

# Run the makefile. You can change CSIM=1 to COSIM=1 or whatever targets you need.
make run CSIM=1 CSYNTH=1 COSIM=1 PLATFORM=${PLATFORM}