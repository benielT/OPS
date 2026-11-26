#!/bin/bash

if [ "$#" -lt 2 ]; then
    echo "Usage: ${0} <target_mode> <app_name> [<additional_args>]"
    exit 1
fi

SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"

TARGET_MODE=$1
APP_NAME=$2
PLATFORM=$3
shift 3
CXXFLAGS="$@"

PROFILE_DIR=./hls/profile_data/${TARGET_MODE}/
PROFILE_FILE=perf_profile.csv
POWER_PROFILE_FILE=hls_power_profile.csv
DEVICE_BDF=0000:a1:00.1

# Hardcoded parameter sets (sizex, sizey, iters, batch)
if [[ "${CXXFLAGS}" == *"-DPOWER_PROFILE"* ]]; then
    echo "Power profiling enabled"
    if [[ "${TARGET_MODE}" == "hw" ]]; then
        if [[ "${PLATFORM}" == *"u280"* ]]; then
            parameter_sets=(
                 "1000,1000,1024,100,512"
                "2500,2500,1024,100,512"
                "5000,5000,1024,50,512"
                "10000,10000,1024,30,512"
                "15000,15000,1024,20,512"
                "20000,20000,1024,10,512"
                "1000,1000,1024,100,1024"
                "2500,2500,1024,100,1024"
                "5000,5000,1024,50,1024"
                "10000,10000,1024,30,1024"
                "15000,15000,1024,20,1024"
                "20000,20000,1024,10,1024"
                "1000,1000,1024,100,2048"
                "2500,2500,1024,100,2048"
                "5000,5000,1024,50,2048"
                "10000,10000,1024,30,2048"
                "15000,15000,1024,20,2048"
                "20000,20000,1024,10,2048"
                "1000,1000,1024,100,4096"
                "2500,2500,1024,100,4096"
                "5000,5000,1024,50,4096"
                "10000,10000,1024,30,4096"
                "15000,15000,1024,20,4096"
                "20000,20000,1024,10,4096"
                "1000,1000,1024,100,8192"
                "2500,2500,1024,100,8192"
                "5000,5000,1024,50,8192"
                "10000,10000,1024,30,8192"
                "15000,15000,1024,20,8192"
                "20000,20000,1024,10,8192"
                # Add more parameter sets here as needed
            )
        else
            parameter_sets=(
                "100,100,60060,5000"
                "200,100,60060,5000"
                "200,200,60060,5000"
                "300,150,60060,2500"
                "300,300,60060,1000"
                "400,200,60060,500"
                "400,300,60060,500"
                "400,400,60060,500"
                # Add more parameter sets here as needed
            )
        fi
    else
        echo "Error: Cannot power profile sw_emu or hw_emu"
        exit 1
    fi
else
    if [[ "${TARGET_MODE}" == "hw" ]]; then
        if [[ "${PLATFORM}" == *"u280"* ]]; then
            parameter_sets=(
                "1000,1000,1024,10,512"
                "2500,2500,1024,10,512"
                "5000,5000,1024,5,512"
                "10000,10000,1024,3,512"
                "15000,15000,1024,2,512"
                "20000,20000,1024,1,512"
                "1000,1000,1024,10,1024"
                "2500,2500,1024,10,1024"
                "5000,5000,1024,5,1024"
                "10000,10000,1024,3,1024"
                "15000,15000,1024,2,1024"
                "20000,20000,1024,1,1024"
                "1000,1000,1024,10,2048"
                "2500,2500,1024,10,2048"
                "5000,5000,1024,5,2048"
                "10000,10000,1024,3,2048"
                "15000,15000,1024,2,2048"
                "20000,20000,1024,1,2048"
                "1000,1000,1024,10,4096"
                "2500,2500,1024,10,4096"
                "5000,5000,1024,5,4096"
                "10000,10000,1024,3,4096"
                "15000,15000,1024,2,4096"
                "20000,20000,1024,1,4096"
                "1000,1000,1024,10,8192"
                "2500,2500,1024,10,8192"
                "5000,5000,1024,5,8192"
                "10000,10000,1024,3,8192"
                "15000,15000,1024,2,8192"
                "20000,20000,1024,1,8192"
                # Add more parameter sets here as needed
                )
        else
            parameter_sets=(
                "30,30,110,100"
                "30,30,60060,100"
                "60,60,60060,100"
                "100,100,60060,100"
                "200,100,60060,100"
                "200,200,60060,100"
                "300,150,60060,20"
                "300,300,60060,20"
                "400,200,60060,20"
                "400,300,60060,20"
                "400,400,60060,20"
                "400,425,60060,20"
                "400,350,60060,20"
                "400,375,60060,20"
                "300,350,60060,20"
                "300,375,60060,20"
                "300,400,60060,20"
                "300,425,60060,20"
                "300,450,60060,20"
                "300,475,60060,20"
                "300,500,60060,20"
                "300,525,60060,20"
                "300,550,60060,20"
                "300,575,60060,20"
                "300,600,60060,10"
                "300,625,60060,10"
                "300,650,60060,10"
                "300,675,60060,10"
                "300,700,60060,10"
                "300,725,60060,10"
                "300,750,60060,5"
                "300,775,60060,5"
                "300,800,60060,5"
                "300,825,60060,5"
                "300,850,60060,5"
                # Add more parameter sets here as needed
                )
        fi
    else
        if [[ "${PLATFORM}" == *"u280"* ]]; then
            parameter_sets=(
                # "10,6,8,1,512"
                "80,8,120,1,512"
                # "50,50,162,4,128"
                # Add more parameter sets here as needed
            )
        else
            parameter_sets=(
                "30,30,110,1"
                # Add more parameter sets here as needed
            )
        fi
    fi
fi

echo "Running application '${APP_NAME}' in '${TARGET_MODE}' mode with hardcoded parameters:"

for params in "${parameter_sets[@]}"; do
    IFS=',' read -r sizex sizey iters batch tilex <<< "$params"

    if [[ -z "$sizex" || -z "$sizey" || -z "$iters" || -z "$batch" || -z "$tilex" ]]; then
        echo "Warning: Skipping invalid parameter set: $params"
        continue
    fi

    # Removing previous residues
    if [ -f "${PROFILE_FILE}" ]; then
        rm ${PROFILE_FILE}
    fi
    if [ -f "${POWER_PROFILE_FILE}" ]; then
        rm ${POWER_PROFILE_FILE}
    fi


    echo "-----------------------------------------------------------------"
    echo "Running with sizex=${sizex}, sizey=${sizey}, iters=${iters}, batch=${batch}, tilex=${tilex}"
    echo "-----------------------------------------------------------------"


    if [[ "${CXXFLAGS}" == *"-DPOWER_PROFILE"* ]]; then
        echo "Running HW mode with power profiling"
            ${OPS_INSTALL_PATH}/../scripts/power_profile_hls.sh ${DEVICE_BDF} ${SCRIPT_DIR}/hls/build/${TARGET_MODE}/${APP_NAME}_host ${SCRIPT_DIR}/hls/build/${TARGET_MODE}/${APP_NAME}.xclbin -sizex="${sizex}" -sizey="${sizey}" -iters="${iters}" -piter="${batch}" OPS_TILING OPS_TILESIZE_X="${tilex}"

    else
        if [[ $TARGET_MODE == sw_emu || $TARGET_MODE == hw_emu ]]; then
            echo "Running in emulation mode with ${TARGET_MODE}"
            XCL_EMULATION_MODE=${TARGET_MODE} ${SCRIPT_DIR}/hls/build/${TARGET_MODE}/${APP_NAME}_host ${SCRIPT_DIR}/hls/build/${TARGET_MODE}/${APP_NAME}.xclbin -sizex="${sizex}" -sizey="${sizey}" -iters="${iters}" -batch="${batch}" OPS_TILING OPS_TILESIZE_X="${tilex}"
        else
            echo "Running HW mode"
            ${SCRIPT_DIR}/hls/build/${TARGET_MODE}/${APP_NAME}_host ${SCRIPT_DIR}/hls/build/${TARGET_MODE}/${APP_NAME}.xclbin -sizex="${sizex}" -sizey="${sizey}" -iters="${iters}" -batch="${batch}" OPS_TILING OPS_TILESIZE_X="${tilex}"
        fi
    fi

    if [ ! -d "${PROFILE_DIR}" ]; then
        echo "Directory '${PROFILE_DIR}' does not exist. Creating it..."
        mkdir -p "${PROFILE_DIR}"
    fi
    if [ -f "${PROFILE_FILE}" ]; then
        # Construct the new filename for the profile directory
        new_filename="${PROFILE_DIR}/${sizex}_${sizey}_${tilex}_${PROFILE_FILE}"
        echo "Moving '${PROFILE_FILE}' to '${new_filename}'"
        mv "${PROFILE_FILE}" "${new_filename}"
    else
        echo "Warning: Output file '${PROFILE_FILE}' not found after the run."
    fi
    if [ -f "${POWER_PROFILE_FILE}" ]; then
        # Construct the new filename for the profile directory
        new_filename="${PROFILE_DIR}/${sizex}_${sizey}_${tilex}_${POWER_PROFILE_FILE}"
        echo "Moving '${POWER_PROFILE_FILE}' to '${new_filename}'"
        mv "${POWER_PROFILE_FILE}" "${new_filename}"
    else
        echo "Warning: Output file '${POWER_PROFILE_FILE}' not found after the run."
    fi
done

echo "-----------------------------------------------------------------"
echo "Finished running all hardcoded parameter sets."

exit 0 