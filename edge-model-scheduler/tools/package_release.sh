#!/usr/bin/env bash
set -euo pipefail

# ═══════════════════════════════════════════════════════════════
# EdgeModelScheduler v3.3 Production Release Packaging Script
# ═══════════════════════════════════════════════════════════════

echo "=========================================================="
echo "📦 Packaging EdgeModelScheduler v3.3 Official Release"
echo "=========================================================="

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${ROOT_DIR}/build_release"
DIST_DIR="${ROOT_DIR}/dist"

rm -rf "${BUILD_DIR}" "${DIST_DIR}"
mkdir -p "${BUILD_DIR}" "${DIST_DIR}"

echo "🔨 [1/5] Configuring Release Build..."
cd "${BUILD_DIR}"
cmake "${ROOT_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-O3 -DNDEBUG -march=native"

echo "🔨 [2/5] Compiling Core Daemon, Tools, and Python Bindings..."
make -j"$(nproc)" edge-scheduler-daemon scheduler_core benchmark_live_measurements edge_scheduler_py

echo "🧪 [3/5] Running Pre-Release Test Verification..."
ctest --output-on-failure

echo "📦 [4/5] Generating Debian Package (.deb)..."
cpack -G DEB
cp *.deb "${DIST_DIR}/"

echo "🐍 [5/5] Packaging Python Native Bindings..."
cp edge_scheduler_py*.so "${DIST_DIR}/"
cp "${ROOT_DIR}/packaging/edge-scheduler.service" "${DIST_DIR}/"

# Generate SHA256 Checksums
cd "${DIST_DIR}"
sha256sum * > SHA256SUMS.txt

echo "=========================================================="
echo "🎉 Release Build Complete! Artifacts packaged in ${DIST_DIR}:"
ls -lh "${DIST_DIR}"
echo "=========================================================="
