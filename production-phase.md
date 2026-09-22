# EdgeModelScheduler: Production Phase Master Plan (v3.0 Commercial Product)

> **Document Type:** Production Engineering Transition & Task Breakdown Plan  
> **Status:** Draft / Ready for Review  
> **Target Release:** v3.0 Production-Ready Edge Platform  
> **Target Hardware:** NVIDIA Jetson Orin Nano, NX, AGX (JetPack 5.1+ / 6.0+) & Embedded x86/ARM SoCs  

---

## 1. Overview & Business Context

### 1.1 The Transition from MVP to Production Product
In the MVP phase (v1.0–v2.5), we successfully proved the core technical thesis:
- Dynamic UMA zero-copy page-locked allocation eliminates 4.5ms memcpy bottlenecks.
- Thermal hysteresis governance (FP16 ⇄ INT8) prevents DVFS clock collapse.
- Dual-stream priority SM preemption guarantees safety SLOs (<35ms) during multi-model contention.
- Sub-40ms emergency braking verified in ROS 2 Humble (safe stop in 28.93ms).

**In the Product Phase (v3.0)**, our objective shifts from "proof of technology" to **"enterprise deployment reliability, multi-process zero-copy IPC, dynamic hardware auto-tuning, fleet observability, and hardened fail-safes."**

A commercial edge customer (e.g. an autonomous robotics OEM or warehouse AMR fleet) requires:
1. Running the scheduler as a resilient system daemon (`systemd`) serving multiple uncoordinated client processes.
2. Inter-Process Communication (IPC) via shared memory (`/dev/shm` & CUDA IPC) with zero serialization overhead.
3. Automated hardware self-calibration that profiles the host SoC on boot and configures optimal parameters.
4. Hardened security, crash resilience, watchdog timers, and remote telemetry syncing.

---

## 2. Project Classification & Success Criteria

- **Project Type:** `SYSTEMS / EMBEDDED C++ / EDGE PLATFORM`
- **Primary Agent:** `project-planner` (coordinating `backend-specialist`, `devops-engineer`, `test-engineer`, `security-auditor`)
- **Key Constraints:** 
  - Absolute C++17 thread safety and memory boundary protection.
  - Zero allocation (`malloc`/`new`) in the real-time inference dispatch critical path.
  - No Python GIL dependencies in daemon core.

### Measurable Success Criteria
| Category | Metric | MVP Baseline | Product Phase (v3.0 Target) |
| :--- | :--- | :--- | :--- |
| **Inter-Process Latency** | Client process to Scheduler dispatch | N/A (single process) | **< 0.8 ms** via POSIX / CUDA IPC |
| **System Daemon Reliability** | MTBF (Mean Time Between Failures) | N/A | **> 720 hours continuous stress** |
| **Watchdog Recovery** | Auto-restart & context recovery | Manual | **< 1.5 seconds failover** |
| **Hardware Auto-Tuning** | Platform calibration time | Manual tuning | **< 60 seconds automated profile** |
| **Footprint & Overhead** | Scheduler daemon CPU usage | ~8% single core | **< 2.5% CPU overhead** |

---

## 3. Production Architecture (v3.0)

```
                     ┌──────────────────────────────────────────────────────────┐
                     │          UNCOORDINATED CLIENT PROCESSES                  │
                     │  (ROS2 Node 1)     (ROS2 Node 2)     (External Python App)│
                     └─────────┬─────────────────┬──────────────────┬───────────┘
                               │                 │                  │
                               ▼                 ▼                  ▼
                     ┌──────────────────────────────────────────────────────────┐
                     │     ZERO-COPY IPC TRANSPORT LAYER (POSIX SHM / CUDA IPC) │
                     │       - /dev/shm Ring Buffer for CPU Frame Buffers       │
                     │       - cudaIpcMemHandle for Direct GPU Virtual Memory   │
                     └──────────────────────────┬───────────────────────────────┘
                                                │
                                                ▼
┌───────────────────────────────────────────────────────────────────────────────────────────────┐
│                    EDGE-MODEL-SCHEDULER SYSTEM DAEMON (C++17)                                 │
│                                                                                               │
│  ┌───────────────────────┐   Dynamic Hysteresis   ┌────────────────────────────────┐          │
│  │ Tegra / Sysfs Monitor │ ─────────────────────> │ Quantization Governor          │          │
│  │ (Direct Kernel Poll)  │                        │ (Hysteresis FP16 ⇄ INT8)       │          │
│  └───────────────────────┘                        └────────────────────────────────┘          │
│              │                                                     │                          │
│              ▼                                                     ▼                          │
│  ┌───────────────────────┐                        ┌────────────────────────────────┐          │
│  │ EWMA Latency Tracker  │ ─────────────────────> │ Dual CUDA Stream Dispatcher    │          │
│  │ (Pre-dispatch SLO)    │                        │ (SM Hardware Preemption)       │          │
│  └───────────────────────┘                        └────────────────────────────────┘          │
│                                                                    │                          │
│  ┌───────────────────────┐                                         ▼                          │
│  │ Zero-Copy Pinned Pool │ ─────────────────────> ┌────────────────────────────────┐          │
│  │ (Recycling Arena)     │                        │ Real TensorRT Engine Pool      │          │
│  │                       │                        │ - Dynamic Batching / Streams   │          │
│  └───────────────────────┘                        │ - CUDA Graph Execution Replay  │          │
│                                                   └────────────────────────────────┘          │
└───────────────────────────────────────┬───────────────────────────────────────────────────────┘
                                        │
                    ┌───────────────────┴───────────────────┐
                    ▼                                       ▼
    ┌───────────────────────────────┐       ┌───────────────────────────────┐
    │ Watchdog & Health Daemon      │       │ OpenTelemetry / MQTT Sync     │
    │ (systemd sd_notify / heartbeat│       │ (Fleet Metrics Exporter)      │
    └───────────────────────────────┘       └───────────────────────────────┘
```

---

## 4. Work Breakdown Structure (Tasks & Phases)

### Phase 1: Zero-Copy Multi-Process IPC Subsystem (P0 - Critical Foundation)
*Goal: Allow multiple independent ROS 2 nodes and external processes to feed inference requests into a single centralized scheduler daemon without copying frame buffers.*

- [ ] **Task 1.1: POSIX Shared Memory Ring Buffer (`ShmRingBuffer.hpp`)**
  - **Owner:** `backend-specialist` | **Skills:** `@clean-code`, `@architecture`
  - **Input:** Request metadata, frame timestamps, shared memory name `/edge_scheduler_shm`.
  - **Output:** Lockless multi-producer single-consumer circular buffer in `/dev/shm`.
  - **Verify:** Multi-process benchmark transferring 1080p frames in <0.3ms.

- [ ] **Task 1.2: CUDA Inter-Process Memory Mapping (`CudaIpcAllocator.hpp`)**
  - **Owner:** `performance-optimizer` | **Skills:** `@performance-profiling`, `@clean-code`
  - **Input:** `cudaIpcGetMemHandle` and `cudaIpcOpenMemHandle`.
  - **Output:** Direct zero-copy GPU pointer passing across process boundaries.
  - **Verify:** External CUDA process submits GPU tensor pointer; daemon executes inference with zero memcpy.

- [ ] **Task 1.3: C++ & Python Client SDK (`libedge_scheduler_client.so`)**
  - **Owner:** `backend-specialist` | **Skills:** `@clean-code`
  - **Input:** IPC transport protocols.
  - **Output:** Lightweight client library with sync/async `.submit_inference()` methods.
  - **Verify:** Client connects to daemon, dispatches frame, receives completion promise.

---

### Phase 2: Production TensorRT Engine Integration & CUDA Graphs (P1 - Core Acceleration)
*Goal: Replace synthetic inference mocks with production TensorRT execution loading real engine files with CUDA Graph replay.*

- [ ] **Task 2.1: Dynamic TensorRT Engine Loader & Execution Context Pool**
  - **Owner:** `backend-specialist` | **Skills:** `@clean-code`
  - **Input:** TensorRT serialized engine (`.engine` / `.plan`).
  - **Output:** Safe `nvinfer1::IExecutionContext` pooling with dynamic tensor bindings.
  - **Verify:** Load dual FP16 and INT8 engines for YOLOv8 and confirm inference on physical Jetson or mock.

- [ ] **Task 2.2: CUDA Graphs Capture for Zero-Jitter Execution**
  - **Owner:** `performance-optimizer` | **Skills:** `@performance-profiling`
  - **Input:** Static execution sequence of TensorRT inference kernels.
  - **Output:** Captured `cudaGraphExec_t` instances replayed in real-time loops.
  - **Verify:** CPU dispatch latency reduced from ~120μs to <15μs per kernel dispatch.

---

### Phase 3: Hardware Self-Calibration & Auto-Tuning Engine (P1 - Differentiator)
*Goal: Remove manual configuration. The platform automatically benchmarks the host SoC on boot and provisions optimal memory pools, EWMA parameters, and thermal thresholds.*

- [ ] **Task 3.1: Hardware Profiler CLI (`edge-scheduler calibrate`)**
  - **Owner:** `backend-specialist` | **Skills:** `@clean-code`
  - **Input:** Local GPU SM count, memory bandwidth, thermal dissipation rate.
  - **Output:** Automated calibration YAML (`/etc/edge-scheduler/hardware_profile.yaml`).
  - **Verify:** Profiler runs 30s benchmark, measures thermal trip points, outputs optimal configuration.

- [ ] **Task 3.2: Dynamic Parameter Reconfiguration API**
  - **Owner:** `backend-specialist` | **Skills:** `@clean-code`
  - **Input:** Running scheduler daemon.
  - **Output:** Dynamic adjustment of queue thresholds and starvation timers via REST/Unix Socket without restart.
  - **Verify:** Send `/api/v1/config` update; verify running scheduler adjusts hysteresis margin on the fly.

---

### Phase 4: Enterprise Daemon, Watchdog & Packaging (P1 - Productionization)
*Goal: Package the system for one-click installation on customer fleets with systemd supervisor integration.*

- [ ] **Task 4.1: Systemd Service & Watchdog (`systemd/edge-scheduler.service`)**
  - **Owner:** `devops-engineer` | **Skills:** `@bash-linux`, `@powershell-windows`
  - **Input:** Linux `systemd` daemon specification with `Type=notify` and `WatchdogSec=5s`.
  - **Output:** Self-healing service that pings systemd and automatically restarts on crash.
  - **Verify:** Kill daemon with `SIGSEGV`; verify systemd revives process within 1.5s with log record.

- [ ] **Task 4.2: Debian Package Builder (`cpack` / `.deb` packaging)**
  - **Owner:** `devops-engineer` | **Skills:** `@bash-linux`
  - **Input:** CMake install targets, systemd unit, default configuration files.
  - **Output:** Standard `.deb` package installable via `sudo dpkg -i edge-scheduler_3.0.0_arm64.deb`.
  - **Verify:** Install package on clean Ubuntu/JetPack image; confirm daemon starts automatically.

---

### Phase 5: Fleet Observability & Remote Telemetry Exporter (P2 - Cloud Integration)
*Goal: Enable edge fleet managers to monitor distributed robots from centralized Grafana/Datadog dashboards.*

- [ ] **Task 5.1: OpenTelemetry / MQTT Metric Exporter (`metrics_exporter.cpp`)**
  - **Owner:** `backend-specialist` | **Skills:** `@clean-code`, `@architecture`
  - **Input:** Daemon telemetry ring buffer.
  - **Output:** High-throughput, low-bandwidth telemetry push over MQTT or OTLP.
  - **Verify:** Publish metrics to local MQTT broker; verify temperature, queue delay, and dropped frames logged.

---

### Phase 6: Chaos & Failure Mode Verification (Phase X - Definition of Done)
*Goal: Subject the complete production platform to automated adversarial tests to guarantee robustness.*

- [ ] **Task 6.1: Thermal Shock Chaos Test**
  - Simulate sudden 15°C temperature jump within 200ms; verify zero dropped high-priority frames.
- [ ] **Task 6.2: Memory Bus Exhaustion Test**
  - Saturate LPDDR5 bus with background memory copies; verify safety perception stays within 35ms SLO.
- [ ] **Task 6.3: Client Process Crash Recovery**
  - Abruptly `kill -9` client processes mid-inference; verify daemon cleans up shared memory handles cleanly without deadlock.

---

## 5. Verification Gate & Deliverables Checklist

- [ ] Plan file written to `production-phase.md` in project root.
- [ ] All 6 phases mapped to explicit INPUT → OUTPUT → VERIFY criteria.
- [ ] Dedicated roles assigned across C++ backend, performance profiling, and DevOps.
- [ ] User review and approval obtained before implementation.
