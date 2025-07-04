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
DEVICE_BDF=0000:c1:00.1

# Hardcoded parameter sets (sizex, iters, batch)
if [[ "${CXXFLAGS}" == *"-DPOWER_PROFILE"* ]]; then
    if [[ "${TARGET_MODE}" == "hw" ]]; then
        if [[ "${CXXFLAGS}" == *"-DBATCHING"* ]]; then
            if [[ "${PLATFORM}" == *"u280"* ]]; then
                parameter_sets=(
                    "1000,60060,10000,1"
                    "5000,60060,10000,1"
                    "2000,60060,10000,1"
                    "10000,60060,10000,1"
                    "20000,60060,5000,1"
                    "30000,60060,2000,1"
                    "1000,60060,10000,10"
                    "5000,60060,10000,10"
                    "2000,60060,10000,10"
                    "10000,60060,10000,10"
                    "20000,60060,5000,10"
                    "30000,60060,2000,10"
                    "1000,60060,10000,20"
                    "5000,60060,10000,20"
                    "2000,60060,10000,20"
                    "10000,60060,10000,20"
                    "20000,60060,5000,20"
                    "30000,60060,2000,20"
                    "1000,60060,10000,50"
                    "5000,60060,10000,50"
                    "2000,60060,10000,50"
                    "10000,60060,10000,50"
                    "20000,60060,5000,50"
                    "30000,60060,2000,50"
                    "1000,60060,10000,100"
                    "5000,60060,10000,100"
                    "2000,60060,10000,100"
                    "10000,60060,10000,100"
                    "20000,60060,5000,100"
                    "30000,60060,2000,100"
                    "1000,60060,10000,200"
                    "5000,60060,10000,200"
                    "2000,60060,10000,200"
                    "10000,60060,10000,200"
                    "20000,60060,5000,200"
                    "30000,60060,2000,200"
                    # Add more parameter sets here as needed
                )
            else
                parameter_sets=(
                    "1000,60000,10000,1"
                    "2000,60000,10000,1"
                    "5000,60000,10000,1"
                    "10000,60000,10000,1"
                    "20000,60000,5000,1"
                    "30000,60000,2000,1"
                    "1000,60000,10000,10"
                    "2000,60000,10000,10"
                    "5000,60000,10000,10"
                    "10000,60000,10000,10"
                    "20000,60000,5000,10"
                    "30000,60000,2000,10"
                    "1000,60000,10000,20"
                    "2000,60000,10000,20"
                    "5000,60000,10000,20"
                    "10000,60000,10000,20"
                    "20000,60000,5000,20"
                    "30000,60000,2000,20"
                    "1000,60000,10000,50"
                    "2000,60000,10000,50"
                    "5000,60000,10000,50"
                    "10000,60000,10000,50"
                    "20000,60000,5000,50"
                    "30000,60000,2000,50"
                    "1000,60000,100000,100"
                    "2000,60000,100000,100"
                    "5000,60000,100000,100"
                    "10000,60000,100000,100"
                    "20000,60000,50000,100"
                    "30000,60000,20000,100"
                    "1000,60000,100000,200"
                    "2000,60000,100000,200"
                    "5000,60000,100000,200"
                    "10000,60000,100000,200"
                    "20000,60000,50000,200"
                    "30000,60000,20000,200"
                # Add more parameter sets here as needed
                )
            fi
        else
            if [[ "${PLATFORM}" == *"u280"* ]]; then
                parameter_sets=(
                    "1000,60060,10000,1"
                    "5000,60060,10000,1"
                    "2000,60060,10000,1"
                    "10000,60060,10000,1"
                    "20000,60060,5000,1"
                    "30000,60060,2000,1"
                    # Add more parameter sets here as needed
                )
            else
                parameter_sets=(
                    "1000,60000,10000,1"
                    "2000,60000,10000,1"
                    "5000,60000,10000,1"
                    "10000,60000,10000,1"
                    "20000,60000,5000,1"
                    "30000,60000,2000,1"
                # Add more parameter sets here as needed
                )
            fi
        fi
    else
        echo "Error: Cannot power profile sw_emu or hw_emu"
        exit 1
    fi
else
    if [[ "${TARGET_MODE}" == "hw" ]]; then
        if [[ "${CXXFLAGS}" == *"-DBATCHING"* ]]; then
            if [[ "${PLATFORM}" == *"u280"* ]]; then
                parameter_sets=(
                    "100,60060,100,1"
                    "150,60060,100,1"
                    "200,60060,100,1"
                    "500,60060,100,1"
                    "1000,60060,100,1"
                    "2000,60060,100,1"
                    "5000,60060,100,1"
                    "10000,60060,100,1"
                    "20000,60060,50,1"
                    "30000,60060,20,1"
                    "100,60060,100,10"
                    "150,60060,100,10"
                    "200,60060,100,10"
                    "500,60060,100,10"
                    "1000,60060,100,10"
                    "2000,60060,100,10"
                    "5000,60060,100,10"
                    "10000,60060,100,10"
                    "20000,60060,50,10"
                    "30000,60060,20,10"
                    "100,60060,100,20"
                    "150,60060,100,20"
                    "200,60060,100,20"
                    "500,60060,100,20"
                    "5000,60060,100,20"
                    "1000,60060,100,20"
                    "2000,60060,100,20"
                    "10000,60060,100,20"
                    "20000,60060,60,20"
                    "30000,60060,20,20"
                    "100,60060,100,50"
                    "150,60060,100,50"
                    "200,60060,100,50"
                    "500,60060,100,50"
                    "1000,60060,100,50"
                    "2000,60060,100,50"
                    "5000,60060,100,50"
                    "10000,60060,100,50"
                    "20000,60060,50,50"
                    "30000,60060,50,50"
                    "100,60060,100,100"
                    "150,60060,100,100"
                    "200,60060,100,100"
                    "500,60060,100,100"
                    "1000,60060,100,100"
                    "2000,60060,100,100"
                    "5000,60060,100,100"
                    "10000,60060,100,100"
                    "20000,60060,100,100"
                    "30000,60060,100,100"
                    "100,60060,200,200"
                    "150,60060,200,200"
                    "200,60060,200,200"
                    "500,60060,200,200"
                    "1000,60060,200,200"
                    "2000,60060,200,200"
                    "5000,60060,200,200"
                    "10000,60060,200,200"
                    "20000,60060,200,200"
                    "30000,60060,200,200"
                    # Add more parameter sets here as needed
                    )
            else
                parameter_sets=(
                    "100,60000,100,1"
                    "150,60000,100,1"
                    "200,60000,100,1"
                    "500,60000,100,1"
                    "1000,60000,100,1"
                    "2000,60000,100,1"
                    "5000,60000,100,1"
                    "10000,60000,100,1"
                    "20000,60000,50,1"
                    "30000,60000,20,1"
                    "100,60000,100,10"
                    "150,60000,100,10"
                    "200,60000,100,10"
                    "500,60000,100,10"
                    "1000,60000,100,10"
                    "2000,60000,100,10"
                    "5000,60000,100,10"
                    "10000,60000,100,10"
                    "20000,60000,50,10"
                    "30000,60000,20,10"
                    "100,60060,100,20"
                    "150,60060,100,20"
                    "200,60060,100,20"
                    "500,60060,100,20"
                    "1000,60060,100,20"
                    "2000,60060,100,20"
                    "5000,60060,100,20"
                    "10000,60060,100,20"
                    "20000,60060,60,20"
                    "30000,60060,20,20"
                    "100,60060,100,50"
                    "150,60060,100,50"
                    "200,60060,100,50"
                    "500,60060,100,50"
                    "1000,60060,100,50"
                    "2000,60060,100,50"
                    "5000,60060,100,50"
                    "10000,60060,100,50"
                    "20000,60060,50,50"
                    "30000,60060,50,50"
                    "100,60060,100,100"
                    "150,60060,100,100"
                    "200,60060,100,100"
                    "500,60060,100,100"
                    "1000,60060,100,100"
                    "2000,60060,100,100"
                    "5000,60060,100,100"
                    "10000,60060,100,100"
                    "20000,60060,50,100"
                    "30000,60060,20,100"
                    # Add more parameter sets here as needed
                    )
            fi
        else
            if [[ "${PLATFORM}" == *"u280"* ]]; then
                parameter_sets=(
                    "100,60060,100,1"
                    "150,60060,100,1"
                    "200,60060,100,1"
                    "500,60060,100,1"
                    "1000,60060,100,1"
                    "2000,60060,100,1"
                    "5000,60060,100,1"
                    "10000,60060,100,1"
                    "20000,60060,50,1"
                    "30000,60060,20,1"
                    # Add more parameter sets here as needed
                    )
            else
                parameter_sets=(
                    "100,60000,100,1"
                    "150,60000,100,1"
                    "200,60000,100,1"
                    "500,60000,100,1"
                    "1000,60000,100,1"
                    "2000,60000,100,1"
                    "5000,60000,100,1"
                    "10000,60000,100,1"
                    "20000,60000,50,1"
                    "30000,60000,20,1"
                    # Add more parameter sets here as needed
                    )
            fi
        fi
    else
        if [[ "${CXXFLAGS}" == *"-DBATCHING"* ]]; then
            if [[ "${PLATFORM}" == *"u280"* ]]; then
                parameter_sets=(
                    "100,132,1,1"
                    # Add more parameter sets here as needed
                )
            else
                parameter_sets=(
                    "100,96,1,1"
                    # Add more parameter sets here as needed
                )
            fi
        else
                if [[ "${PLATFORM}" == *"u280"* ]]; then
                parameter_sets=(
                    "100,132,4,2"
                    # Add more parameter sets here as needed
                )
            else
                parameter_sets=(
                    "100,96,4,2"
                    # Add more parameter sets here as needed
                )
            fi
        fi
    fi
fi

echo "Running application '${APP_NAME}' in '${TARGET_MODE}' mode with hardcoded parameters:"

for params in "${parameter_sets[@]}"; do
    IFS=',' read -r sizex iters batch bsize<<< "$params"

    if [[ -z "$sizex" || -z "$iters" || -z "$batch" || -z "$bsize" ]]; then
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
    echo "Running with sizex=${sizex}, iters=${iters}, batch=${batch} b_size=${bsize}"
    echo "-----------------------------------------------------------------"


    if [[ "${CXXFLAGS}" == *"-DPOWER_PROFILE"* ]]; then
        echo "Running HW mode with power profiling"
            ${OPS_INSTALL_PATH}/../scripts/power_profile_hls.sh ${DEVICE_BDF} ${SCRIPT_DIR}/hls/build/${TARGET_MODE}/${APP_NAME}_host ${SCRIPT_DIR}/hls/build/${TARGET_MODE}/${APP_NAME}.xclbin -sizex="${sizex}" -iters="${iters}" -piter="${batch}" -bsize="${bsize}"

    else
        if [[ $TARGET_MODE == sw_emu || $TARGET_MODE == hw_emu ]]; then
            echo "Running in emulation mode with ${TARGET_MODE}"
            XCL_EMULATION_MODE=${TARGET_MODE} ${SCRIPT_DIR}/hls/build/${TARGET_MODE}/${APP_NAME}_host ${SCRIPT_DIR}/hls/build/${TARGET_MODE}/${APP_NAME}.xclbin -sizex="${sizex}" -iters="${iters}" -batch="${batch}"  -bsize="${bsize}"
        else
            echo "Running HW mode"
            ${SCRIPT_DIR}/hls/build/${TARGET_MODE}/${APP_NAME}_host ${SCRIPT_DIR}/hls/build/${TARGET_MODE}/${APP_NAME}.xclbin -sizex="${sizex}" -iters="${iters}" -batch="${batch}"  -bsize="${bsize}"
        fi
    fi

    if [ ! -d "${PROFILE_DIR}" ]; then
        echo "Directory '${PROFILE_DIR}' does not exist. Creating it..."
        mkdir -p "${PROFILE_DIR}"
    fi
    if [ -f "${PROFILE_FILE}" ]; then
        # Construct the new filename for the profile directory
        new_filename="${PROFILE_DIR}/${sizex}_${bsize}_${PROFILE_FILE}"
        echo "Moving '${PROFILE_FILE}' to '${new_filename}'"
        mv "${PROFILE_FILE}" "${new_filename}"
    else
        echo "Warning: Output file '${PROFILE_FILE}' not found after the run."
    fi
    if [ -f "${POWER_PROFILE_FILE}" ]; then
        # Construct the new filename for the profile directory
        new_filename="${PROFILE_DIR}/${sizex}_${bsize}_${POWER_PROFILE_FILE}"
        echo "Moving '${POWER_PROFILE_FILE}' to '${new_filename}'"
        mv "${POWER_PROFILE_FILE}" "${new_filename}"
    else
        echo "Warning: Output file '${POWER_PROFILE_FILE}' not found after the run."
    fi
done

echo "-----------------------------------------------------------------"
echo "Finished running all hardcoded parameter sets."

exit 0
