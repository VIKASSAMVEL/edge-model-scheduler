#!/bin/bash
set -e

echo "=========================================================="
echo "🚀 NVIDIA Jetson Orin Nano / NX One-Click Setup Script"
echo "=========================================================="

echo "[1/4] Checking JetPack and Linux for Tegra (L4T) version..."
if [ -f /etc/nv_tegra_release ]; then
    head -n 1 /etc/nv_tegra_release
else
    echo "⚠️ Not running on native Tegra hardware. Configuring generic Linux environment."
fi

echo "[2/4] Installing Required System Dependencies..."
sudo apt-get update && sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    python3-dev \
    python3-pip \
    libpython3-dev \
    pybind11-dev

echo "[3/4] Compiling EdgeModelScheduler Native Core..."
cd "$(dirname "$0")/.."
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j"$(nproc)"

echo "[4/4] Verifying Build Artifacts..."
if [ -f libscheduler_core.so ] || [ -f libscheduler_core.a ]; then
    echo "✅ Native core built successfully!"
else
    echo "❌ Build check warning: check CMake output."
fi

echo "Setup complete! Run 'python3 ../benchmarks/advanced_features_benchmark.py' to benchmark."
