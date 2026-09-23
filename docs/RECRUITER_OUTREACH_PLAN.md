# Target Placement & Recruiter Outreach Masterplan

## 1. The Strategy: Why this gets you past the ATS & Recruiters
Generic applicants send resumes that list: *"Used YOLOv8 and OpenCV to detect objects."* 
Every recruiter and engineering manager at NVIDIA, Qualcomm, and autonomous vehicle startups throws those resumes straight into the archive because every college student has that line.

Your resume now highlights:
> **"Designed an asynchronous C++17 inference scheduler with pybind11 bindings for Jetson Orin SoCs, using kernel sysfs telemetry to dynamically shed non-critical frames during thermal throttling, cutting p99 latency by 57% and eliminating SLO violations."**

This immediately signals:
- You know **C++17, low-level concurrency, and memory management**.
- You know **Jetson, TensorRT, and Linux kernel sysfs interfaces**.
- You understand **system bottlenecks, p99 latency, and real-time reliability**, not just calling `model.predict()`.

---

## 2. Target Roles & Companies to Target in India

| Company | Exact Target Role Titles | Locations | Typical Compensation Band |
| :--- | :--- | :--- | :--- |
| **NVIDIA** | AI Systems Software Engineer, Robotics SWE (Isaac/PVA), Embedded Systems Engineer | Bengaluru / Pune / Hyderabad | ₹26L - ₹45L+ (Entry/IC2), ₹60L+ (IC3) |
| **Qualcomm** | AI Software Engineer, Embedded Perception Engineer, Systems Performance SWE | Bengaluru / Hyderabad / Chennai | ₹22L - ₹38L+ |
| **Amazon** | Robotics Software Engineer, SDE (Edge & Devices), AWS ML Systems | Bengaluru / Hyderabad | ₹32L - ₹48L+ |
| **Google** | ML Systems / Embedded Systems SWE | Bengaluru / Hyderabad | ₹35L - ₹55L+ |
| **Startups** | Autonomous Robotics (Ati Motors, Cyanergy, Ottonomy, Vectorly) | Bengaluru | ₹20L - ₹35L+ |

---

## 3. Cold Outreach / Referral Message (LinkedIn & Email)

Send this directly to **Engineering Managers, Staff Engineers, and Tech Leads** (not just HR/recruiters):

```text
Subject: Systems project: Adaptive Multi-Model Inference Scheduler for Jetson Orin (C++ / TensorRT)

Hi [Name],

I’ve been following the work your team is doing on [mention team/product, e.g. NVIDIA Isaac / Qualcomm AI Stack]. 

Given the known memory bandwidth and thermal constraints when running concurrent perception models on edge SoCs, I built an open-source C++ inference arbiter (with pybind11 bindings) that monitors live sysfs hardware telemetry to dynamically shed low-priority frames under thermal throttling. In high-contention benchmarks, it reduced p99 latency by 57% (from 86ms to 36ms) and dropped safety-critical SLO violations to 0%.

Repository & Benchmarks: [Link to your GitHub repo]

I’m targeting [Role Title] roles and would value 5 minutes of your technical feedback, or a referral if there’s a fit with your group.

Best regards,
Vikas S
[LinkedIn Profile] | [GitHub Profile]
```

---

## 4. Immediate Action Checklist for You
1. **Push the repository to your GitHub**:
   - Repository name: `edge-model-scheduler`
   - Use the generated [README.md](file:///d:/next-big-thing/edge-model-scheduler/README.md) as the main front page.
2. **Read and absorb the Defense Guide**:
   - Review [docs/INTERVIEW_DEFENSE.md](file:///d:/next-big-thing/edge-model-scheduler/docs/INTERVIEW_DEFENSE.md). It tells you what to say when an interviewer asks you about Triton, C++, GIL, or priority inversion.
3. **Update your Resume**:
   - Add the 3 bullet points listed in Section 3 of `INTERVIEW_DEFENSE.md` to your primary projects section.
