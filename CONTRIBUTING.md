# Contributing to Edge Model Scheduler

Thank you for your interest in contributing to **Edge Model Scheduler**, the world-class ultra-low-latency deterministic co-scheduling runtime for edge AI, autonomous driving (ISO 26262 ASIL-D), and robotics systems.

We welcome pull requests from developers, systems engineers, and autonomous robotics researchers worldwide!

---

## Code of Conduct

All contributors and participants are expected to adhere to our [Code of Conduct](CODE_OF_CONDUCT.md). Please treat all members with respect and professionalism.

---

## Architectural Principles

Before writing code, keep these micro-architectural invariants in mind:

1. **Zero Dynamic Allocation on the Fast Path:** The inference loop and preemption dispatch must remain strictly deterministic. Never allocate heap memory (`new`, `malloc`, `std::vector::push_back` exceeding capacity) inside `submitTask()`, `scheduleNext()`, or `yieldQuantum()`.
2. **Lock-Free Concurrency:** Use atomics (`std::atomic<T>`, memory order semantics `acquire`/`release`) and bounded ring buffers. Avoid blocking mutexes in safety-critical threads.
3. **Safety First (ISO 26262 ASIL-D):** Any critical failure must trigger the `SafetyIslandGovernor` and fail-safe transition within $\le 5.0\text{ ms}$.
4. **Hardware Portability:** While tailored for NVIDIA DRIVE Thor / Blackwell and Orin architectures, core modules must maintain graceful software fallbacks when physical CUDA Graphs, DLA, or Blackwell hardware is absent.

---

## Development Setup

### Prerequisites
- Linux (Ubuntu 22.04 LTS recommended) or Docker
- CMake $\ge 3.20$
- GCC $\ge 11$ / Clang $\ge 14$ with C++17 support
- Python 3.10+ with `pybind11` and `pytest`
- (Optional) ROS 2 Humble / Iron for Autoware surround perception stack

### Building & Running Unit Tests Locally

```bash
cd edge-model-scheduler
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTS=ON
make -j$(nproc)
ctest --output-on-failure
```

### Running with Docker

We provide an automated multi-stage development container:

```bash
docker build -t edge-model-scheduler:latest -f edge-model-scheduler/Dockerfile .
docker run --rm -it edge-model-scheduler:latest ctest --test-dir build --output-on-failure
```

---

## Submitting Pull Requests

1. **Fork the repository** on GitHub.
2. **Create a topic branch** from `main`:
   ```bash
   git checkout -b feature/your-feature-name
   ```
3. **Write clean, documented code:**
   - Adhere to Google C++ Style Guide conventions.
   - Use meaningful commit messages following the [Conventional Commits](https://www.conventionalcommits.org/) format (e.g., `feat:`, `fix:`, `perf:`, `docs:`, `test:`).
4. **Ensure all test suites pass:**
   - CTest (9/9 suites)
   - Pytest for Python bindings
   - ROS 2 perception integration checks
5. **Open a Pull Request:**
   - Fill out all sections in the [Pull Request Template](.github/PULL_REQUEST_TEMPLATE.md).
   - Reference any related issues.

---

## Community & Discussions

- **GitHub Discussions:** Join our architecture design reviews, roadmap proposals, and Q&A.
- **Bug Reports:** Open an issue via our [Bug Report Form](.github/ISSUE_TEMPLATE/bug_report.yml).
