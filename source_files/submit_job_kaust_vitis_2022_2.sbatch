#!/bin/bash

#SBATCH --job-name=u280-build
#SBATCH --partition=build
#SBATCH --nodelist=acclnode01        # has the platform, has NO U280 — harrier stays free
#SBATCH --cpus-per-task=48
#SBATCH --mem=192G
#SBATCH --time=12:00:00
#SBATCH --output=slurm-%j.out

SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"

echo $SLURMD_NODENAME

set -euo pipefail

source ${OPS_INSTALL_PATH}/../source_files/setup_env_xil_fpga_gnu_vitis_2022_2_tapa_KAUST.sh

PROJECT="$(realpath "$(pwd)")"
SCRATCH_ROOT="${TMPDIR:-/tmp}/${USER}"
mkdir -p "$SCRATCH_ROOT"
SCRATCH="$(mktemp -d "$SCRATCH_ROOT/Job-${SLURM_JOB_ID:-manual}.XXXXXX")"

echo "Project directory: $PROJECT"
echo "Scratch directory: $SCRATCH"

save_failure_diagnostics() {
    status=$?
    set +e
    if (( status != 0 )); then
        if [ -d "$SCRATCH/hls" ]; then
            mkdir -p "$PROJECT/hls"
            cp -a "$SCRATCH/hls/." "$PROJECT/hls/"
            echo "Generated HLS sources copied to: $PROJECT/hls"
        fi
        diagnostics="$PROJECT/failed-build-${SLURM_JOB_ID:-manual}"
        mkdir -p "$diagnostics"
        while IFS= read -r -d '' file; do
            relative_path="${file#"$SCRATCH"/}"
            mkdir -p "$diagnostics/$(dirname "$relative_path")"
            cp "$file" "$diagnostics/$relative_path"
        done < <(find "$SCRATCH" -type f \( -name '*.log' -o -name '*.rpt' -o -name '*.xml' -o -name '*.jou' \) -print0)
        echo "Build failed with exit code $status"
        echo "Diagnostics saved in: $diagnostics"
    fi
    rm -rf "$SCRATCH"
    exit "$status"
}
trap save_failure_diagnostics EXIT

echo "Copy into directory: $SCRATCH"
cp -rv "$PROJECT"/* "$SCRATCH"/
cd "$SCRATCH"

echo "Starting make"

make

for file in "$SCRATCH"/*_ops_hls.cpp; do
    [ -f "$file" ] && cp "$file" "$PROJECT"/
done
for file in "$SCRATCH"/*.txt; do
    [ -f "$file" ] && cp "$file" "$PROJECT"/
done
for file in "$SCRATCH"/*.log; do
    [ -f "$file" ] && cp "$file" "$PROJECT"/
done
if [ -f "$SCRATCH"/.generated_hls ]; then 
    cp "$SCRATCH"/.generated_hls "$PROJECT"/
fi
if [ -d "$SCRATCH"/hls ]; then 
    cp -r "$SCRATCH"/hls/ "$PROJECT"/
fi

echo "SLURM Build completed"