# Systems Benchmark Specification & Experimental Protocol
**EdgeModelScheduler Verification Suite — Standard Operating Procedure**  
*Methodology: IEEE Real-Time Systems Symposium (RTSS) / ACM EuroSys Empirical Rigor*

---

## 1. Experimental Philosophy & Standards

This specification dictates the methodology, execution parameters, statistical treatments, and reproduction commands for every experiment in EdgeModelScheduler.

### 1.1 Experimental Rules
1. **Zero Warm-Up Bias**: Discard initial execution rounds (minimum 50 iterations) to purge instruction cache cold misses, memory allocation initializations, and OS paging jitter.
2. **Statistical Transparency**: Never report lone arithmetic means. Always report Sample Count ($N$), Min, Max, Standard Deviation ($\sigma$), P50, P90, P95, P99, and Deadline Violation Rates ($\%$) with 95% Confidence Intervals.
3. **Hardware State Control**: Lock CPU/GPU clock frequencies during deterministic tests (`jetson_clocks` on Tegra, or recorded governor settings) to eliminate uncontrolled DVFS jitter unless testing the thermal governor itself.
4. **Reproducible Automation**: Every experiment must be runnable via a single deterministic CLI command that captures an environment manifest, writes raw CSV/JSON records, and generates publication plots.

---

## 2. Benchmark Suite Catalog

### 2.1 Experiment 1: The Impossible Workload Under Infeasible Contention
* **Identifier**: `EXP-01-IMPOSSIBLE-WORKLOAD`
* **Hypothesis**: When cumulative workload demand exceeds 100% of accelerator capacity ($\sum r_i > \mathcal{R}$), EdgeModelScheduler maintains $< 0.1\%$ deadline misses for safety-critical tasks through proactive stale-frame shedding and mission-utility arbitration, whereas conventional schedulers experience catastrophic cascading deadline collapse ($> 30\%$ miss rate).
* **Hardware Profile**: 6-core / 8-core CPU, unified memory architecture, hardware accelerator (GPU/DLA).
* **Workload Definition**:
  1. *Emergency Braking / Obstacle Detection*: Criticality $c=1.0$, Deadline $d=20\text{ ms}$, Execution $e=5\text{ ms}$, Rate $50\text{ Hz}$.
  2. *Front-Camera Perception*: Criticality $c=0.8$, Deadline $d=35\text{ ms}$, Execution $e=12\text{ ms}$, Rate $30\text{ Hz}$.
  3. *Side/Rear Perception*: Criticality $c=0.5$, Deadline $d=66\text{ ms}$, Execution $e=15\text{ ms}$, Rate $15\text{ Hz}$.
  4. *Semantic Segmentation*: Criticality $c=0.3$, Deadline $d=100\text{ ms}$, Execution $e=35\text{ ms}$, Rate $10\text{ Hz}$.
  5. *3D Mapping / SLAM*: Criticality $c=0.2$, Deadline $d=200\text{ ms}$, Execution $e=45\text{ ms}$, Rate $5\text{ Hz}$.
* **Load Curve**: Cumulative load swept from $50\%$ (feasible) $\to 100\%$ (saturation) $\to 180\%$ (severely over-subscribed).
* **Baselines Evaluated**:
  * Baseline A: Naive Sequential (FIFO)
  * Baseline B: Naive Concurrent (Unmanaged POSIX threads)
  * Baseline C: Round-Robin Queue
  * Baseline D: Static Priority Queue
  * Baseline E: Earliest-Deadline-First (EDF)
  * Baseline F: Server-Style Dynamic Batcher (Triton architectural model)
  * Baseline G: EdgeModelScheduler (Ours)
* **Sample Count**: $N = 1{,}000$ task dispatches per baseline. Warm-up: 100 tasks.
* **Metrics**: P50, P95, P99 latency (overall and critical-only), Deadline Miss $\%$, Stale Frames Dropped, Mission Utility Score $\mathcal{U}$.
* **Reproduction Command**: `python scripts/reproduce_all.py --experiment impossible_workload`

---

### 2.2 Experiment 2: Stale Data & Ingress Jitter Rejection
* **Identifier**: `EXP-02-STALE-SHEDDING`
* **Hypothesis**: Pre-execution staleness detection prevents wasted accelerator cycles on obsolete sensor data, reducing downstream queue latency by $> 60\%$ without sacrificing perception accuracy.
* **Workload**: Perception stream subjected to controlled synthetic sensor delay injections: $\Delta t_{\text{delay}} \in \{0, 10, 20, 30, 50, 100, 200\}\text{ ms}$.
* **Metrics**: Ingress frames dropped, computational cycles saved, end-to-end freshness of processed frames.
* **Reproduction Command**: `python scripts/reproduce_all.py --experiment stale_shedding`

---

### 2.3 Experiment 3: Closed-Loop Thermal Hysteresis & Precision Governance
* **Identifier**: `EXP-03-THERMAL-GOVERNOR`
* **Hypothesis**: Dual-threshold hysteresis (trip at 85°C, restore at 70°C) switching between FP16 and INT8 maintains real-time throughput within 98% of target without triggering hardware DVFS thermal throttling.
* **Procedure**: Inject sustained simulated GPU workload while logging temperature, operating precision mode, and P99 latency over 60 seconds.
* **Metrics**: Precision state transitions, clock frequency stability, P99 latency during thermal event, recovery hysteresis time.
* **Reproduction Command**: `python scripts/reproduce_all.py --experiment thermal_governor`

---

### 2.4 Experiment 4: Battery & Power State-of-Charge (SoC) QoS
* **Identifier**: `EXP-04-BATTERY-QOS`
* **Hypothesis**: Dynamic transition from `NORMAL` (100% capacity) $\to$ `ECO` (60% capacity) $\to$ `CRITICAL_RTH` (20% capacity, Return-To-Home) preserves safety-critical trajectory planning while reducing total energy consumption per inference by $> 45\%$.
* **Metrics**: Energy per inference (mJ), non-critical frame shed percentage, critical task latency stability.
* **Reproduction Command**: `python scripts/reproduce_all.py --experiment battery_qos`

---

### 2.5 Experiment 5: Heterogeneous GPU + DLA Resource Arbitration
* **Identifier**: `EXP-05-DLA-ARBITRATION`
* **Hypothesis**: Offloading deterministic vision tasks to NVDLA co-processors while reserving GPU Tensor Cores for dynamic multi-task workloads eliminates memory bus contention and increases total system task throughput by $> 1.7\times$.
* **Configurations**:
  1. GPU Only (all 5 models on GPU)
  2. GPU + DLA0 (perception offloaded to DLA)
  3. GPU + DLA0 + DLA1 (dual DLA offload)
* **Metrics**: GPU memory bandwidth saturation, inference latency per device, system-wide task throughput.
* **Reproduction Command**: `python scripts/reproduce_all.py --experiment dla_arbitration`

---

### 2.6 Experiment 6: Micro-Architectural Scheduler Overhead
* **Identifier**: `EXP-06-SCHEDULER-OVERHEAD`
* **Hypothesis**: EdgeModelScheduler's lock-free memory and dispatch mechanisms impose $< 50\text{ ns}$ overhead per task, accounting for $< 0.01\%$ of total inference latency.
* **Measurements**:
  1. `PinnedMemoryPool::acquire()` and `release()` latency (100,000 iterations).
  2. `ShmRingBuffer` write and read sequence latency.
  3. `AdvancedScheduler` priority queue insertion and pop duration.
  4. CPU utilization of the scheduler daemon under 500 tasks/second.
* **Reproduction Command**: `python scripts/reproduce_all.py --experiment scheduler_overhead`

---

### 2.7 Experiment 7: Subsystem Ablation Study
* **Identifier**: `EXP-07-ABLATION-STUDY`
* **Hypothesis**: Every major subsystem contributes measurably to tail latency reduction and deadline satisfaction under stress.
* **Evaluated Variants**:
  1. Full EdgeModelScheduler System
  2. Variant $\ominus$ Stale-Frame Shedding
  3. Variant $\ominus$ Thermal Precision Governor
  4. Variant $\ominus$ DLA Co-Scheduler
  5. Variant $\ominus$ Lock-Free Pinned Memory Pool
  6. Variant $\ominus$ Slack Batcher
* **Metrics**: Delta in P99 Latency ($\Delta \text{ms}$), Delta in Deadline Miss Rate ($\Delta \%$).
* **Reproduction Command**: `python scripts/reproduce_all.py --experiment ablation`

---

### 2.8 Experiment 8: Adversarial Chaos & Fault Injection
* **Identifier**: `EXP-08-CHAOS-INJECTION`
* **Hypothesis**: The system recovers deterministically within $< 5\text{ ms}$ upon unexpected accelerator stalls, memory corruption errors, or sensor dropouts.
* **Faults Injected**:
  * Synthetic GPU hang (100ms timeout)
  * Out-of-memory buffer exhaustion
  * Corrupted frame payload
  * Watchdog heartbeat loss
* **Reproduction Command**: `python scripts/reproduce_all.py --experiment chaos`
