# ⚡ EdgeModelScheduler (Enterprise Platform)
### Hardware-Aware Autonomous AI Engine & Micro-Architectural Arbiter for NVIDIA Jetson & Edge Robotics

> **Latest Commercial Release:** `v3.3.0 Extreme Performance Release`  
> **Target Silicon:** NVIDIA Jetson Orin Nano / NX / AGX, Xavier, & Edge Embedded Linux  
> **Key Benchmark:** Cuts P99 Tail Latency by **84.5%** and RAM footprint by **96.6%** over NVIDIA Triton Inference Server.

---

## 📂 Repository Navigation

The complete source code, tests, documentation, and benchmarks live in the [`edge-model-scheduler/`](edge-model-scheduler/) directory:

- 📖 **[Master Documentation & Architecture](edge-model-scheduler/README.md)**: Full architecture breakdown, CLI guides, benchmarks, and installation instructions.
- 🔬 **[Empirical Benchmark Whitepaper](edge-model-scheduler/docs/WHITE_PAPER_BENCHMARK_REPORT.md)**: Head-to-head empirical validation against NVIDIA Triton and Vanilla ROS 2.
- 🏛️ **[C4 Architecture Specification](edge-model-scheduler/docs/ARCHITECTURE.md)**: Deep-dive into UMA zero-copy memory, thermal hysteresis state machines, and lock-free SPSC POSIX shared memory.
- 🛡️ **[Technical Defense Playbook](edge-model-scheduler/docs/INTERVIEW_DEFENSE.md)**: Engineering defense scripts explaining micro-architectural decisions.
- 📦 **[Pre-Built Debian Package](edge-model-scheduler/build_v3/edge-scheduler-3.0.0-Linux.deb)**: Standalone `.deb` package ready for single-command deployment via `dpkg -i`.

---

## ⚡ Quickstart

```bash
cd edge-model-scheduler

# Build the high-performance C++17 engine and tests
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Run full native test battery
./test_scheduler_core
./test_v3_subsystems
./test_god_level
./test_model_compiler
./chaos_runner
```

For complete documentation, visit the [**EdgeModelScheduler Master README**](edge-model-scheduler/README.md).
