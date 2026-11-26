#!/bin/sh

# synthesis_script.sh

#SBATCH -t 24:00:00
#SBATCH --cpus-per-task=8
#SBATCH --mem=64G
#SBATCH -q fpgasynthesis
#SBATCH -A hpc-prf-acgasm
#SBATCH -p normal

source ${OPS_INSTALL_PATH}/../source_files/setup_env_xil_fpga_gnu_vitis_tapa_2022_2_NOCTUA2.sh

make
