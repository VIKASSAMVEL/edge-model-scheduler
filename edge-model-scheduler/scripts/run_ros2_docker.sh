#!/bin/bash
set -e

echo "========================================================"
echo "🚀 EdgeModelScheduler: ROS 2 Humble Docker Build & Test"
echo "========================================================"

cd "$(dirname "$0")/.."

echo "[1/3] Checking Docker installation..."
if ! command -v docker &> /dev/null; then
    echo "❌ Error: Docker is not detected in your environment."
    echo "Please ensure Docker Desktop / WSL2 Docker service is active."
    exit 1
fi

echo "[2/3] Building ROS 2 Humble Container & Workspace..."
docker compose -f docker/docker-compose.yml build

echo "[3/3] Launching EdgeModelScheduler ROS 2 Node..."
docker compose -f docker/docker-compose.yml up
