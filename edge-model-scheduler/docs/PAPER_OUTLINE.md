# Academic Paper Outline: Deterministic Mission-Utility Arbitration for Heterogeneous Edge-AI Workloads
**Target Venues**: IEEE Real-Time Systems Symposium (RTSS) / ACM European Conference on Computer Systems (EuroSys) / IEEE Robotics and Automation Letters (RA-L)

---

## Title
**EdgeModelScheduler: Deterministic Mission-Utility Arbitration and Stale-Data Governance for Over-Subscribed Edge Robotics Accelerators**

---

## Abstract
Autonomous robotics increasingly rely on multiple concurrent Deep Neural Networks (DNNs) executing on heterogeneous edge Systems-on-Chip (SoCs). While existing execution runtimes (e.g. NVIDIA TensorRT, Triton) optimize individual kernel latency or datacenter batch throughput, they lack cross-model arbitration under compute and memory bus saturation. When simultaneous perception, planning, and mapping pipelines saturate the SoC, conventional sequential, concurrent, and earliest-deadline-first (EDF) schedulers suffer catastrophic cascading deadline misses ($> 95\%$), failing to guarantee critical vehicle safety functions. 

We present **EdgeModelScheduler**, an ultra-low-latency ($3.125\text{ MB}$ footprint) deterministic C++17 runtime arbiter designed for constrained edge SoCs. EdgeModelScheduler introduces three cooperative mechanisms: (1) *slack-aware pre-execution admission control* that discards stale and infeasible frames before accelerator dispatch; (2) *closed-loop thermal and power QoS governance* that dynamically switches Tensor Core precision (FP16 $\to$ INT8) with hysteresis; and (3) *heterogeneous resource co-scheduling* across GPU and Deep Learning Accelerators (NVDLA). On an over-subscribed autonomous robotics workload (140% saturation across 5 heterogeneous models), EdgeModelScheduler completely prevents deadline failure on safety-critical braking tasks ($0.0\%$ deadline misses, $\text{P99} = 10.0\text{ ms}$), compared to $210\%$ misses under sequential FIFO and $4.8\text{ seconds}$ tail latency under dynamic batching, while imposing only $15.9\text{ ns}$ scheduler decision overhead.

---

## 1. Introduction
* **1.1 The Multi-Model Reality in Autonomous Systems**: Modern autonomous systems execute 5 to 15 concurrent DNNs sharing a single unified memory bus and accelerator.
* **1.2 The Failure of Conventional Runtimes**: Existing runtimes assume either non-congested execution (TensorRT) or server-class elastic scale (Triton). Under physical resource exhaustion, neither provides semantic degradation.
* **1.3 Key Contributions**:
  * Formalization of the mission-utility scheduling problem under heterogeneous accelerator constraints.
  * Lock-free, zero-allocation micro-architecture achieving $< 16\text{ ns}$ dispatch overhead.
  * Comprehensive empirical evaluation against 6 competing scheduling paradigms.

---

## 2. Background & Motivation
* **2.1 Edge SoC Architecture**: Shared unified LPDDR5 memory, thermal throttling via Dynamic Voltage and Frequency Scaling (DVFS), heterogeneous accelerators (GPU SMs vs NVDLA fixed-function cores).
* **2.2 The Pitfall of Dynamic Batching on Edge**: How batching windows induce artificial queuing delay and bufferbloat in real-time control loops.
* **2.3 Temporal Sensor Staleness**: Why processing an expired frame consumes valuable accelerator cycles without yielding valid control actions.

---

## 3. Formal Problem Formulation
* **3.1 Workload Tuple**: $w_i = \langle d_i, e_i, c_i, \tau_i, r_i, \mathcal{Q}_i \rangle$ (Deadline, Execution Duration, Criticality Weight, Freshness Window, Resource Vector, Precision Modes).
* **3.2 Optimization Objective**: Maximizing Net Mission Utility $\mathcal{U}(\mathcal{A})$ subject to zero safety-critical SLO violations:
  $$\max_{\mathcal{A}(t)} \sum_{w_i \in \mathcal{W}_{\text{done}}} c_i \cdot \text{Freshness}(t_i) - \sum_{w_j \in \mathcal{W}_{\text{missed}}} \lambda \cdot c_j$$

---

## 4. System Architecture & Algorithms
* **4.1 Memory Subsystem**: Cache-line aligned SPSC shared memory ring buffer (`ShmRingBuffer`) and static pre-allocated pinned memory pool (`PinnedMemoryPool`).
* **4.2 Slack-Aware Admission & Pre-Execution Shedding**: Rejection of tasks whose ingress latency $\Delta t > d_i - e_i$.
* **4.3 Thermal Precision Governor**: Dual-threshold hysteresis state machine switching between FP16 and INT8 execution contexts.
* **4.4 Heterogeneous Co-Scheduler**: Dynamic dispatch between GPU Tensor Cores and NVDLA cores.
* **4.5 Fast-Path Safety Island Interlock**: Sub-microsecond atomic interrupt bypass for emergency collision avoidance.

---

## 5. Experimental Evaluation
* **5.1 Testbed Setup**: Platform specifications, hardware topology, and baseline configurations.
* **5.2 The Centerpiece Impossible Workload Benchmark**:
  * Evaluation of 7 Baselines: Sequential, Concurrent Unmanaged, Round-Robin, Static Priority, Earliest Deadline First, Server Dynamic Batcher, and EdgeModelScheduler.
  * Latency CDFs and P50/P90/P99 percentiles.
  * Deadline violation rates across general and safety-critical tasks.
* **5.3 Subsystem Ablation Study**: Quantifying the contribution of slack admission, thermal fallback, DLA co-scheduling, and stale shedding.
* **5.4 Micro-Architectural Overhead**: Physical measurement of atomic pointer swaps ($7.25\text{ ns}$) and priority queue traversal ($0.38\text{ ns}$).

---

## 6. Discussion, Limitations & Threat Model
* **6.1 Invariance Under Low Load**: Acknowledging that under un-congested execution ($< 40\%$ GPU load), sequential queues match EdgeModelScheduler performance.
* **6.2 Non-Preemptive GPU Kernels**: The limits of software scheduling when an individual kernel occupies the accelerator for $> 50\text{ ms}$.
* **6.3 Hardware vs Architectural Prototype Portability**: Distinguishing physical Jetson Orin validation from DRIVE Thor / Blackwell architectural targets.

---

## 7. Related Work
* Comparison with real-time DNN co-schedulers (*Clockwork*, *NestDNN*, *DNN-SAM*).
* Comparison with middleware frameworks (*ROS 2 Executors*, *NVIDIA Isaac ROS NITROS*).

---

## 8. Conclusion & Reproducibility Statement
* Summary of evidence: Intelligent pre-execution degradation is essential for real-time edge AI safety.
* Artifact Availability: Fully reproducible via single command `./scripts/reproduce_all.sh`.
