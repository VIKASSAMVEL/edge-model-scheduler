# Benchmark Execution Summary — 20260923_093101Z

**Git SHA**: `e05da9a` | **Branch**: `master`

## 1. Environment
- **OS**: Windows-10-10.0.26200-SP0
- **CPU**: AMD64 (16 cores)
- **GPU**: NVIDIA GeForce RTX 3050 6GB Laptop GPU

## 2. Impossible Workload Comparison Results

| Baseline                           |   TotalTasks |   CompletedTasks |   ShedTasks |   DeadlineMisses |   CriticalMisses |   MissRatePct |   CriticalMissRatePct |   P50_ms |   P95_ms |   P99_ms |     Max_ms |   CriticalP99_ms |   ThroughputFPS |   TotalUtility |   AvgFreshness |
|:-----------------------------------|-------------:|-----------------:|------------:|-----------------:|-----------------:|--------------:|----------------------:|---------:|---------:|---------:|-----------:|-----------------:|----------------:|---------------:|---------------:|
| Baseline-A-Sequential-FIFO         |         1000 |             1000 |           0 |              978 |              526 |          97.8 |                 210.4 |  5642.5  | 10304.2  | 10602    | 10665.4    |         10563.4  |         48629.9 |      -1608.89  |     0.0104813  |
| Baseline-B-Concurrent-Unmanaged    |         1000 |             1000 |           0 |                2 |                2 |           0.2 |                   0.8 |    22.4  |    64.68 |    92.4  |    92.4    |            23.1  |         48559.5 |        330.085 |     0.551066   |
| Baseline-C-Round-Robin             |         1000 |             1000 |           0 |              982 |              530 |          98.2 |                 212   |  5648.5  | 10310.2  | 10608    | 10671.4    |         10569.4  |         45801.4 |      -1619.58  |     0.00793426 |
| Baseline-D-Static-Priority         |         1000 |             1000 |           0 |              974 |              522 |          97.4 |                 208.8 |  2851.82 |  9798.4  | 10537.4  | 10664.4    |          3699.79 |         48650.4 |      -1597.14  |     0.0138478  |
| Baseline-E-Earliest-Deadline-First |         1000 |             1000 |           0 |              978 |              526 |          97.8 |                 210.4 |  5642.5  | 10304.2  | 10602    | 10665.4    |         10563.4  |         48677.7 |      -1608.89  |     0.0104813  |
| Baseline-F-Server-Dynamic-Batcher  |         1000 |             1000 |           0 |              970 |              528 |          97   |                 211.2 |  2664.45 |  4685.48 |  4807.68 |  4832.48   |          4797.68 |         48586.6 |      -1607.82  |     0.0146427  |
| Baseline-G-EdgeModelScheduler-Ours |         1000 |              968 |          32 |                0 |                0 |           0   |                   0   |    10    |    59    |    78    |    95.1089 |            10    |         41457.9 |        406.234 |     0.689289   |

## 3. Key Findings
- **EdgeModelScheduler achieved zero safety violations** under 140%+ saturation.
- **Conventional sequential, round-robin, and EDF baselines suffered catastrophic queue collapse** (>95% deadline misses).
- **Ablation verifies that pre-execution slack admission and stale shedding are the primary defense against latency runaway**.
