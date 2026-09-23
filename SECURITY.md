# Security Policy

## Supported Versions

We provide security and functional safety updates for the following active releases:

| Version | Supported          | Security Status |
| ------- | ------------------ | --------------- |
| 3.3.x   | :white_check_mark: | Active (Current)|
| 3.2.x   | :white_check_mark: | Maintenance     |
| 3.1.x   | :x:                | Deprecated      |
| < 3.1.0 | :x:                | End of Life     |

---

## Reporting a Vulnerability

The **Edge Model Scheduler** codebase is engineered for safety-critical environments including autonomous vehicles (ISO 26262 ASIL-D), medical robotics, and mission-critical edge gateways. We take security vulnerabilities and safety violations with utmost seriousness.

### Where to Report
Please do **NOT** report security vulnerabilities through public GitHub issues or discussions.

Instead, please submit reports using:
1. **GitHub Private Security Advisory:** [Open an Advisory](https://github.com/VIKASSAMVEL/edge-model-scheduler/security/advisories/new)
2. **Security Team Direct Contact:** Send an encrypted email with detailed reproduction steps to `security@vikassamvel.dev` (Subject: `[VULNERABILITY] Edge Model Scheduler`).

### What to Include
To help us triage and patch efficiently, please include:
- Description of the potential vulnerability or memory-safety issue.
- Target hardware platform (e.g., NVIDIA Jetson Orin, DRIVE Thor emulator, x86_64 simulation).
- Minimal Reproducible Example (MRE) or test payload.
- Potential impact on the ASIL-D safety heartbeat or real-time priority preemption invariants.

### Response Timeline
- **Initial Response:** Within 24 hours.
- **Triage & Severity Assessment:** Within 72 hours.
- **CVE Generation & Coordinated Disclosure:** We aim to release a patch within 14 days of triage.
