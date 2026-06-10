#!/bin/bash

PLATFORM="/opt/software/FPGA/Xilinx/platforms/xilinx_u280_gen3x16_xdma_1_202211_1/xilinx_u280_gen3x16_xdma_1_202211_1.xpfm"

# Select random indices for the arrays
NUM_TESTS=5
# TEST_SEED=2190821588    #passed 2190821588 3470835791
# TEST_VALUES=1
GENERATOR_SEED=86077981  #512-256 design
# DEBUG_VERBOSE=1

FLAGS=

[ -n "$NUM_TESTS" ] && FLAGS+="-DNUM_TESTS=$NUM_TESTS "
[ -n "$TEST_SEED" ] && FLAGS+="-DTEST_SEED=$TEST_SEED "
[ -n "$TEST_VALUES" ] && FLAGS+="-DTEST_VALUES "
[ -n "$DEBUG_VERBOSE" ] && FLAGS+="-DDEBUG_LOG "
# Format the compiler definitions
export USER_CFLAGS=$FLAGS 


echo "========================================"
echo " RUN SCRIPT SIDE PARAMTERS        "
echo "========================================"
echo "  NUM_TESTS = $NUM_TESTS"
echo "  TEST_SEED     = $TEST_SEED"
echo "----------------------------------------"



if [ ! -f .generated ]; then
echo "-------------------------------------------------"
echo "  Generated files not found. Generating files...."
echo "-------------------------------------------------"
  if [ -n "$GENERATOR_SEED" ]; then
    python generator.py -rs "$GENERATOR_SEED"
  else
    python generator.py
  fi
fi

# Run the makefile. You can change CSIM=1 to COSIM=1 or whatever targets you need.
make run CSIM=1 CSYNTH=0 COSIM=0 PLATFORM=${PLATFORM}