#!/usr/bin/env bash
# ──────────────────────────────────────────────────────────────
# Canonical Reproducibility Entry Point — EdgeModelScheduler
# Usage: ./scripts/reproduce_all.sh
# ──────────────────────────────────────────────────────────────
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"

echo "==============================================================="
echo "  Launching EdgeModelScheduler Scientific Reproducibility Engine"
echo "==============================================================="

python3 "${SCRIPT_DIR}/reproduce_all.py" "$@"
