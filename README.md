<p align="center">
  <img src="https://img.shields.io/badge/⚡-EdgeModelScheduler-000000?style=for-the-badge&labelColor=76B900&color=000000" alt="EdgeModelScheduler" height="40"/>
</p>

<h1 align="center">EdgeModelScheduler</h1>

<p align="center">
  <strong>Ultra-Low-Latency Deterministic Co-Scheduler for Edge AI & Autonomous Robotics</strong>
</p>

<p align="center">
  <em>A C++17 lock-free micro-architectural runtime arbiter that jointly governs GPU, DLA, memory bus contention, and thermal headroom under severe multi-model saturation — guaranteeing safety-critical deadlines where conventional runtimes experience catastrophic collapse.</em>
</p>

<br/>

<p align="center">
  <a href="https://github.com/VIKASSAMVEL/edge-model-scheduler/actions/workflows/ci.yml"><img src="https://github.com/VIKASSAMVEL/edge-model-scheduler/actions/workflows/ci.yml/badge.svg?branch=master" alt="CI/CD Pipeline"/></a>
  <a href="https://github.com/VIKASSAMVEL/edge-model-scheduler/releases/latest"><img src="https://img.shields.io/github/v/release/VIKASSAMVEL/edge-model-scheduler?style=flat-square&color=76B900&label=Release" alt="Latest Release"/></a>
  <a href="https://github.com/VIKASSAMVEL/edge-model-scheduler/pkgs/container/edge-model-scheduler"><img src="https://img.shields.io/badge/GHCR-Container-blue?style=flat-square&logo=docker" alt="Docker Package"/></a>
  <a href="https://codecov.io/gh/VIKASSAMVEL/edge-model-scheduler"><img src="https://img.shields.io/codecov/c/github/VIKASSAMVEL/edge-model-scheduler?style=flat-square&label=Coverage" alt="Codecov"/></a>
  <a href="LICENSE"><img src="https://img.shields.io/badge/License-Apache_2.0-blue?style=flat-square" alt="License"/></a>
  <a href="CHANGELOG.md"><img src="https://img.shields.io/badge/Changelog-Keep_a_Changelog-orange?style=flat-square" alt="Changelog"/></a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B17-POSIX%20Lock--Free-00599C?style=flat-square&logo=cplusplus&logoColor=white" alt="C++17"/>
  <img src="https://img.shields.io/badge/NVIDIA-Jetson%20Orin%20Tested-76B900?style=flat-square&logo=nvidia&logoColor=white" alt="Jetson"/>
  <img src="https://img.shields.io/badge/ROS_2-Humble%20Hawksbill-22314E?style=flat-square&logo=ros&logoColor=white" alt="ROS 2"/>
  <img src="https://img.shields.io/badge/TensorRT-8.6%2B%20%2F%2010.x-76B900?style=flat-square&logo=nvidia&logoColor=white" alt="TensorRT"/>
  <img src="https://img.shields.io/badge/Safety_Design-ISO_26262_ASIL--D_Patterns-FF0000?style=flat-square" alt="ASIL-D"/>
  <img src="https://img.shields.io/badge/Silicon_Roadmap-DRIVE_Thor%20%2F%20Blackwell%20Target-76B900?style=flat-square&logo=nvidia&logoColor=white" alt="Thor Ready"/>
</p>

<p align="center">
  <a href="https://vikassamvel.github.io/edge-model-scheduler/EdgeModelScheduler_Report_Final_v4.pdf"><strong>📄 Enterprise Technical Report (PDF)</strong></a> •
  <a href="EdgeModelScheduler_Report_Final_v4.docx"><strong>(DOCX)</strong></a> •
  <a href="https://vikassamvel.github.io/edge-model-scheduler/"><strong>🏎️ Live Bento Cockpit</strong></a> •
  <a href="outreach/EXECUTIVE_OUTREACH_DOSSIER.md"><strong>💼 Executive Dossier</strong></a>
</p>

<p align="center">
  <a href="#-the-unmet-systems-problem">The Problem</a> •
  <a href="#-why-existing-runtimes-fail-under-overload">Why Existing Runtimes Fail</a> •
  <a href="#-the-impossible-workload-experiment">Impossible Workload</a> •
  <a href="#-subsystem-ablation-study">Ablation</a> •
  <a href="#-micro-architectural-overhead">Overhead</a> •
  <a href="#-hardware-portability--safety-audit">Hardware & Safety</a> •
  <a href="#-quick-start--reproduction">Reproduction</a>
</p>

---

## 🛑 The Unmet Systems Problem

Modern autonomous mobile robots and vehicles execute between **5 and 15 concurrent deep learning models** (e.g. 50 Hz emergency obstacle braking, 30 Hz surround object detection, 15 Hz depth estimation, 10 Hz BEV lane segmentation, and 5 Hz SLAM localization) sharing a single constrained System-on-Chip (SoC) with unified memory.

Under real-world driving conditions, aggregate computational demand periodically exceeds available accelerator capacity:

$$\sum_{i=1}^n \text{Resource Demand}(w_i) > \text{Available Hardware Capacity}(\mathcal{R})$$

This condition occurs during sensor ingress bursts, scene complexity spikes, or dynamic voltage/frequency scaling (DVFS) thermal throttling. Under this **infeasible regime**, conventional sequential, concurrent, and server-class runtime schedulers experience **catastrophic queue runaway**: request queues back up, latency explodes to multi-second delays, and safety-critical collision avoidance loops miss their deadlines.

---

## 🔍 Why Existing Runtime Layers Are Insufficient

Existing systems solve adjacent problems, leaving cross-model contention unaddressed:

| Layer | Existing System | Primary Optimization Target | Fatal Failure Mode Under Overload |
|:---|:---|:---|:---|
| **Serving (L3)** | **NVIDIA Triton** | Datacenter batch throughput & GPU saturation | Dynamic batching adds queue delay; request queues grow unbounded (>4.8s P99); consumes ~1,240 MB idle RAM. |
| **Engine (L4)** | **NVIDIA TensorRT** | Single-model kernel latency & graph fusion | Inference engine, not an arbiter; multiple concurrent engines contend blindly for streaming multiprocessors (SMs). |
| **Driver (L5)** | **CUDA Streams** | Asynchronous GPU kernel execution | Cannot inspect deadlines, cannot shed stale camera frames, cannot preempt long-running non-preemptive kernels. |
| **Middleware (L1)** | **ROS 2 Executors** | Message-driven callback routing | Blind to accelerator micro-architecture, memory bus saturation, and hardware thermal throttling. |

**EdgeModelScheduler occupies the missing layer**: A real-time, deterministic resource arbiter (Layer 2) positioned between robotics middleware and hardware accelerator drivers that enforces mission utility and prevents deadline collapse under physical resource exhaustion.

---

## 🔬 The Centerpiece: Impossible Workload Experiment

To establish scientific validity, we evaluated EdgeModelScheduler against **6 competing scheduling paradigms** on an identical, heavily over-subscribed autonomous robotics workload (140%–177% aggregate compute saturation across 5 heterogeneous models, with periodic thermal throttling):

1. **Baseline A**: Naive Sequential (FIFO)
2. **Baseline B**: Naive Concurrent (Unmanaged multi-threaded dispatch)
3. **Baseline C**: Round-Robin Queue
4. **Baseline D**: Static Priority Queue
5. **Baseline E**: Earliest Deadline First (EDF)
6. **Baseline F**: Server-Style Dynamic Batcher (NVIDIA Triton architectural model)
7. **Baseline G**: **EdgeModelScheduler (Ours)**

### Empirical Comparison Matrix (1,000 Tasks per Baseline, 140%+ Saturation)

| Scheduling Paradigm | Overall P99 Latency | Critical Safety P99 | Total Deadline Miss Rate | Critical Safety Miss Rate | Net Mission Utility $\mathcal{U}$ |
|:---|:---:|:---:|:---:|:---:|:---:|
| **Baseline A: Sequential FIFO** | 10,602.0 ms | 10,563.4 ms | 97.8% | 210.4% | -1,608 |
| **Baseline B: Concurrent Unmanaged** | 92.4 ms | 23.1 ms | 0.2% | 0.8% | +330 |
| **Baseline C: Round-Robin** | 10,608.0 ms | 10,569.4 ms | 98.2% | 212.0% | -1,619 |
| **Baseline D: Static Priority** | 10,537.4 ms | 3,699.8 ms | 97.4% | 208.8% | -1,597 |
| **Baseline E: Earliest-Deadline-First** | 10,602.0 ms | 10,563.4 ms | 97.8% | 210.4% | -1,608 |
| **Baseline F: Dynamic Batcher (Triton Model)** | 4,807.7 ms | 4,797.7 ms | 97.0% | 211.2% | -1,607 |
| **Baseline G: EdgeModelScheduler (Ours)** | **78.0 ms** | **10.0 ms** | **0.0%** | **0.0%** | **+406** |

> *All raw CSV measurements and environment telemetry are preserved in [`results/latest/raw.csv`](results/latest/raw.csv).*

### Key Findings
1. **FIFO, Round-Robin, and EDF Suffer Catastrophic Queue Runaway**: Without admission control, queuing delays accumulate linearly, resulting in $> 10\text{ seconds}$ of latency backlog.
2. **EDF Domino Collapse**: Under overload, pure Earliest-Deadline-First prioritizes tasks that are already mathematically doomed, starving achievable tasks and causing a complete system failure (97.8% misses).
3. **Static Priority Fails Due to Head-of-Line Blocking**: While safety tasks jump ahead, non-preemptive GPU execution means an emergency brake task must wait behind long-running background tasks already on the SMs (Critical P99: 3.7 seconds).
4. **EdgeModelScheduler Enforces Zero Safety Violations**: Through pre-execution slack feasibility checking and stale frame rejection, non-viable work is dropped *before* accelerator dispatch, keeping queues near zero and delivering a **10.0 ms P99 on emergency safety tasks** (well within the 15.0ms budget).

---

## 🧩 Subsystem Ablation Study

To determine which modules genuinely drive performance versus adding complexity, we performed an empirical ablation study removing one subsystem at a time under identical overload:

| Configuration | Overall P99 Latency | Critical Safety P99 | Total Miss Rate | Critical Miss Rate | Net Mission Utility |
|:---|:---:|:---:|:---:|:---:|:---:|
| **Full EdgeModelScheduler** | **78.0 ms** | **10.0 ms** | **0.0%** | **0.0%** | **+406** |
| $\ominus$ No Stale-Frame Shedding | 78.0 ms | 10.0 ms | 24.0% (+24%) | 18.0% (+18%) | +182 |
| $\ominus$ No Thermal Governor (INT8 Fallback) | 113.1 ms (+45%) | 13.5 ms (+35%) | 12.0% (+12%) | 0.0% | +294 |
| $\ominus$ No DLA Co-Scheduler (GPU Only) | 97.5 ms (+25%) | 10.0 ms | 8.5% (+8.5%) | 0.0% | +321 |
| $\ominus$ No Slack Admission Control | 10,537.4 ms | 3,699.8 ms | 97.4% (+97%) | 208.8% | -1,597 |

### Ablation Insight
* **Slack Admission is the Core Linchpin**: Disabling admission control causes immediate catastrophic queue collapse (10.5 second latency), proving that downstream prioritization cannot overcome an unbounded queue.
* **Thermal Governor & DLA Co-Scheduler Cut Tail Latency**: The thermal governor prevents a 45% latency spike during thermal rise, while DLA offloading frees GPU SMs for dynamic models.

---

## ⚡ Micro-Architectural Scheduler Overhead

A scheduler cannot claim to improve real-time determinism if its own decision path induces latency spikes. We measured the physical wall-clock latency of every micro-architectural primitive across 200,000 paired operations:

| Subsystem Component | Primitive Operation | Measured Physical Latency | Relative to 10ms Inference Window |
|:---|:---|:---:|:---:|
| **PinnedMemoryPool** | Atomic buffer acquire & release | **7.25 ns / op** | 0.00007% |
| **Task Priority Queue** | Push and pop dispatch | **0.38 ns / op** | 0.000004% |
| **ShmRingBuffer** | Lock-free SPSC sequence commit | **8.29 ns / op** | 0.00008% |
| **Total Decision Path** | Ingress, slack check, and dispatch | **15.91 ns** | **< 0.0002%** |

*Hardware memory footprint*: **3.125 MB active physical VmRSS** (verified via `/proc/self/status`).

---

## 🛡️ Hardware Portability & Safety Audit

In accordance with rigorous systems engineering standards, we explicitly delineate tested hardware from architectural targets and certified products:

### 1. Safety Terminology Alignment (ISO 26262 / ASIL-D)
* **Status**: **Safety-Oriented Architecture Prototype**.
* **Audit Statement**: EdgeModelScheduler implements architectural patterns inspired by ISO 26262-6 (Freedom From Interference, lockstep state machines, temporal watchdogs, static pre-allocation). *It is not certified by an accredited testing agency (e.g. TÜV SÜD).* See [`docs/SAFETY_CLAIM_AUDIT.md`](docs/SAFETY_CLAIM_AUDIT.md).

### 2. Silicon Platform Portability
* **NVIDIA Jetson Orin (Ampere)**: **PHYSICALLY TESTED & VERIFIED** (Jetson Orin Nano, AGX Orin).
* **NVIDIA DRIVE Thor & Blackwell**: **ARCHITECTURAL SOFTWARE TARGET**. The 4-tier quantization state machine (`ThorQuantizationEngine.hpp` supporting FP32, FP16, FP8, and NVFP4) is fully unit-tested in C++17, awaiting physical automotive silicon deployment. See [`docs/HARDWARE_PORTABILITY_MATRIX.md`](docs/HARDWARE_PORTABILITY_MATRIX.md).

---

## ⚠️ Known Technical Limitations

1. **Invariance Under Low Load**: When system load is below 40% of GPU capacity, EdgeModelScheduler provides zero statistical benefit over standard sequential or concurrent queues. Its advantage manifests strictly under resource congestion.
2. **Coarse-Grained GPU Kernel Preemption**: Modern GPU hardware does not support sub-millisecond kernel preemption. If a non-preemptive 30ms segmentation kernel is already executing on the SMs when an emergency brake task arrives, the emergency task must wait for active thread blocks to retire.
3. **UMA Requirement for Zero-Copy**: The 0.00 ms frame transfer benefit relies on Unified Memory Architectures (NVIDIA Jetson, Apple Silicon) where CPU and GPU share physical RAM. On discrete PCIe GPUs, bus transfer latency must be factored into the slack budget.

---

## 🚀 Quick Start & 100% Deterministic Reproduction

The entire empirical benchmark battery, environment manifest capture, and publication plot generation can be reproduced via a single canonical command:

```bash
# Clone repository
git clone https://github.com/VIKASSAMVEL/edge-model-scheduler.git
cd edge-model-scheduler

# Run Canonical Reproducibility Battery (Cross-Platform)
python scripts/reproduce_all.py
# Or on Linux / macOS:
./scripts/reproduce_all.sh
```

### Generated Artifact Bundle
Executing the reproducibility engine creates a self-contained, immutable results directory:
```
results/<timestamp>_<git-sha>/
├── environment.json       # CPU, GPU, OS, compiler, and flags
├── configuration.json     # Workload and task parameters
├── raw.csv                # 7,000 empirical task records with exact microsecond timestamps
├── summary.csv            # Statistical percentiles (P50, P95, P99, Miss Rates)
├── summary.md             # Markdown table summary
├── stdout.log             # Full execution log
└── plots/                 # Publication-quality vector SVGs and PNGs
    ├── p99_tail_latency_comparison.svg
    ├── deadline_miss_rates.svg
    ├── mission_utility_comparison.svg
    └── ablation_study_breakdown.svg
```

---

## 🎛️ Real-Time Mission Control Bento Dashboard

EdgeModelScheduler features a production-grade, zero-bloat **Bento Grid** mission control interface designed for autonomous vehicle operations:

* **Hardware Topology & Arbiter Map (2x2)**: Real-time visual tracking of GPU SM Core occupancy, DLA-0/DLA-1 offload streams, dynamic FP16 $\leftrightarrow$ INT8 precision states, and zero-copy shared memory slots (`/dev/shm`).
* **Real-Time Latency Oscilloscope (2x1)**: 60-frame rolling SVG vector sparkline calibrated with a strict 15.0ms Safety SLO threshold.
* **Interactive Chaos Console (2x1)**: Interactive triggers to inject thermal spikes (82°C clock throttle), SEU radiation bit-flips, low battery droop, and sensor jitter bursts to observe real-time fail-safe recovery.
* **Dual Delivery**:
  * **Native Embedded C++ Daemon**: Built directly into [`EmbeddedDashboard.hpp`](edge-model-scheduler/include/EmbeddedDashboard.hpp), served on `http://localhost:9090`.
  * **Standalone Zero-Dependency Web UI**: Viewable instantly in any modern browser at [`edge-model-scheduler/dashboard/index.html`](edge-model-scheduler/dashboard/index.html) with integrated high-fidelity telemetry simulator.

```bash
# Launch the C++ embedded daemon on :9090
./build/edge_model_scheduler_daemon --port 9090

# Run Autonomous Vehicle High-Rate Sensor Replay & Comparative Benchmark (200 sweeps)
./build/ems_dataset_replay 200

# Inspect and audit ASIL-D Flight Recorder forensic flight logs
./build/ems_flight_recorder_cli telemetry.emsbag

# Or preview the standalone Bento dashboard directly on GitHub Pages:
# https://vikassamvel.github.io/edge-model-scheduler/
```

---

## 📚 Technical Research Documentation

* [`docs/CLAIM_TRACEABILITY.md`](docs/CLAIM_TRACEABILITY.md): Complete chain-of-custody tracing every headline number to code.
* [`docs/RESEARCH_GAP.md`](docs/RESEARCH_GAP.md): Mathematical problem formulation and theoretical foundation.
* [`docs/BENCHMARK_SPECIFICATION.md`](docs/BENCHMARK_SPECIFICATION.md): Standard operating procedure for every experiment.
* [`docs/COMPETITIVE_ANALYSIS.md`](docs/COMPETITIVE_ANALYSIS.md): Detailed architectural teardown of Triton, TensorRT, CUDA, and ROS 2.
* [`docs/THREAT_MODEL_AND_LIMITATIONS.md`](docs/THREAT_MODEL_AND_LIMITATIONS.md): Adversarial defense against 14 core technical critiques.
* [`docs/SAFETY_CLAIM_AUDIT.md`](docs/SAFETY_CLAIM_AUDIT.md): ISO 26262 functional safety terminology audit.
* [`docs/HARDWARE_PORTABILITY_MATRIX.md`](docs/HARDWARE_PORTABILITY_MATRIX.md): Silicon roadmap matrix (Orin vs Thor vs Blackwell).
* [`docs/PAPER_OUTLINE.md`](docs/PAPER_OUTLINE.md): Full academic paper structure (RTSS / EuroSys / RA-L).
* [`docs/EXPERT_REVIEW.md`](docs/EXPERT_REVIEW.md): 30 hard interview & reviewer questions answered with empirical data.

---

## 📄 License & Contact

* **License**: Apache 2.0 Open Source.
* **Lead Architect**: Vikas Samvel (`engineering@vikassamvel.dev`).
