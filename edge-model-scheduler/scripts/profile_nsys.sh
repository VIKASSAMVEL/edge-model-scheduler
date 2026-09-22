#!/bin/bash
set -e

echo "=========================================================="
echo "⚡ EdgeModelScheduler: NVIDIA Nsight Systems Profiling Suite"
echo "=========================================================="

cd "$(dirname "$0")/.."

OUTPUT_DIR="profile_reports"
mkdir -p "$OUTPUT_DIR"

REPORT_NAME="edge_scheduler_trace_$(date +%Y%m%d_%H%M%S)"

echo "[1/4] Checking for NVIDIA Nsight Systems (nsys)..."
if ! command -v nsys &> /dev/null; then
    echo "⚠️ Warning: 'nsys' command not found in PATH."
    echo "If running on Jetson, nsys is pre-installed at /opt/nvidia/nsight-systems/bin/nsys"
    echo "If running on x86 desktop, install from: https://developer.nvidia.com/nsight-systems"
    echo "Running simulation trace fallback..."
    PROFILER=""
else
    PROFILER="nsys profile --trace=cuda,nvtx,osrt --output=${OUTPUT_DIR}/${REPORT_NAME} --force-overwrite=true"
fi

echo "[2/4] Executing Hardware-Level Trace Profile..."
if [ -n "$PROFILER" ]; then
    $PROFILER python3 benchmarks/advanced_features_benchmark.py
    echo "✅ Trace captured: ${OUTPUT_DIR}/${REPORT_NAME}.nsys-rep"
    echo "You can open this file in NVIDIA Nsight Systems GUI to view the timeline."
else
    python3 benchmarks/advanced_features_benchmark.py
fi

echo "[3/4] Generating Text Timeline Statistics..."
if command -v nsys &> /dev/null; then
    nsys stats --report cuda_api_sum "${OUTPUT_DIR}/${REPORT_NAME}.nsys-rep" > "${OUTPUT_DIR}/${REPORT_NAME}_stats.txt" || true
fi

echo "[4/4] Profiling completed successfully."
