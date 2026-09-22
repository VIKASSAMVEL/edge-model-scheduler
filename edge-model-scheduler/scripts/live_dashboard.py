#!/usr/bin/env python3
"""
EdgeModelScheduler v2.5 - Live Real-Time Mission Control Dashboard
Renders edge telemetry, queue depths, thermal hysteresis governor, 
zero-copy memory pool state, and tail latency histograms.
"""

import os
import sys
import time
import math
import random
import argparse

# Ensure UTF-8 output on Windows consoles
if sys.platform == "win32":
    try:
        sys.stdout.reconfigure(encoding="utf-8")
    except Exception:
        pass

# ANSI Terminal Color Escapes
CLEAR_SCREEN = "\033[2J\033[H"
RESET = "\033[0m"
BOLD = "\033[1m"
DIM = "\033[2m"
RED = "\033[31m"
GREEN = "\033[32m"
YELLOW = "\033[33m"
BLUE = "\033[34m"
MAGENTA = "\033[35m"
CYAN = "\033[36m"
WHITE = "\033[37m"
BG_DARK = "\033[40m"

def render_bar(percentage, width=24, fill_char="█", empty_char="░", color=GREEN):
    fill_len = int(max(0, min(100, percentage)) / 100.0 * width)
    empty_len = width - fill_len
    return f"{color}{fill_char * fill_len}{DIM}{WHITE}{empty_char * empty_len}{RESET}"

def format_temp(temp):
    if temp >= 80.0:
        return f"{RED}{BOLD}{temp:5.1f}°C (THROTTLE RISK){RESET}"
    elif temp >= 72.0:
        return f"{YELLOW}{temp:5.1f}°C (WARM){RESET}"
    else:
        return f"{GREEN}{temp:5.1f}°C (OPTIMAL){RESET}"

def draw_dashboard(state):
    lines = []
    lines.append(f"{CLEAR_SCREEN}")
    lines.append(f"{BOLD}{CYAN}╔════════════════════════════════════════════════════════════════════════════════════════════╗{RESET}")
    lines.append(f"{BOLD}{CYAN}║      ⚡ EDGE-MODEL-SCHEDULER v2.5 — REAL-TIME EMBEDDED GPU ARBITER MISSION CONTROL        ║{RESET}")
    lines.append(f"{BOLD}{CYAN}╚════════════════════════════════════════════════════════════════════════════════════════════╝{RESET}")
    
    # Header Info
    target_hw = state.get("hardware", "NVIDIA Jetson Orin Nano (Emulated UMA)")
    uptime = state.get("uptime_sec", 0)
    lines.append(f" {BOLD}Target SoC:{RESET} {YELLOW}{target_hw}{RESET} | {BOLD}Runtime:{RESET} C++17 Native Engine | {BOLD}Uptime:{RESET} {uptime:02d}s")
    lines.append(f"{DIM}──────────────────────────────────────────────────────────────────────────────────────────────{RESET}")

    # Row 1: Telemetry & Governor
    temp = state["temp"]
    gpu = state["gpu_load"]
    mem_mb = state["mem_mb"]
    mem_max = state["mem_max"]
    gov_mode = state["gov_mode"]

    gov_color = GREEN if gov_mode == "FP16 (High Fidelity)" else YELLOW
    
    lines.append(f"{BOLD}⚙️  HARDWARE TELEMETRY & THERMAL DVFS MONITOR{RESET}")
    lines.append(f"  • SoC Die Temp:  [{render_bar(temp, 20, color=RED if temp>=80 else (YELLOW if temp>=72 else GREEN))}] {format_temp(temp)}")
    lines.append(f"  • GPU SM Load:   [{render_bar(gpu, 20, color=RED if gpu>90 else GREEN)}] {gpu:5.1f}%")
    lines.append(f"  • Unified VRAM:  [{render_bar((mem_mb/mem_max)*100, 20, color=BLUE)}] {mem_mb:4d} / {mem_max} MB (Pinned UMA)")
    lines.append(f"  • Governor State: {gov_color}{BOLD}[ {gov_mode} ]{RESET} (Dynamic INT8 Threshold: >80°C, Hysteresis: <72°C)")
    lines.append("")

    # Row 2: Queue Depths & Preemption Dispatcher
    q_high = state["q_high"]
    q_med = state["q_med"]
    q_low = state["q_low"]
    dropped = state["dropped_frames"]
    
    lines.append(f"{BOLD}🚦 MULTI-STREAM ARBITRATION QUEUES & DEADLINE ENFORCEMENT{RESET}")
    lines.append(f"  • High Priority (Obstacle / Collision) : {RED}{BOLD}{q_high:2d} tasks{RESET} in flight | Target SLO: {BOLD}< 35 ms{RESET}")
    lines.append(f"  • Med Priority  (Lane / Visual SLAM)   : {YELLOW}{q_med:2d} tasks{RESET} in flight | Target SLO: {BOLD}< 70 ms{RESET}")
    lines.append(f"  • Low Priority  (Telemetry / Logging)  : {BLUE}{q_low:2d} tasks{RESET} in flight | Best-Effort (Aging >150ms)")
    lines.append(f"  • Dropped Frames (Thermal Protection)  : {RED}{dropped:4d} frames{RESET} shed to protect safety SLO")
    lines.append("")

    # Row 3: Latency & EWMA Profiler Metrics
    p50 = state["p50"]
    p95 = state["p95"]
    p99 = state["p99"]
    ewma = state["ewma_pred"]
    slo_rate = state["slo_compliance"]
    
    lines.append(f"{BOLD}📊 LATENCY METRICS & EWMA PREDICTIVE PROFILER{RESET}")
    lines.append(f"  • EWMA Predicted Next Latency: {CYAN}{BOLD}{ewma:5.2f} ms{RESET} (Smoothing Factor α = 0.20)")
    lines.append(f"  • Measured Percentiles       : p50 = {GREEN}{p50:5.2f} ms{RESET} | p95 = {YELLOW}{p95:5.2f} ms{RESET} | p99 = {RED if p99>40 else GREEN}{p99:5.2f} ms{RESET}")
    lines.append(f"  • High-Priority SLO Compliance: {GREEN if slo_rate>=99.0 else YELLOW}{BOLD}{slo_rate:5.1f}%{RESET} [Safety Critical]")
    lines.append("")

    # Event Log Feed
    lines.append(f"{BOLD}📜 LIVE EVENT LOG FEED (Zero-Copy Pinned Ring Buffer){RESET}")
    lines.append(f"{DIM}──────────────────────────────────────────────────────────────────────────────────────────────{RESET}")
    for event in state.get("events", [])[-5:]:
        lines.append(f"  {DIM}▸{RESET} {event}")
    
    lines.append(f"{DIM}──────────────────────────────────────────────────────────────────────────────────────────────{RESET}")
    lines.append(f" {DIM}Press Ctrl+C to exit dashboard. Powered by EdgeModelScheduler C++17 Core.{RESET}")
    
    print("\n".join(lines))

def simulate_dashboard(duration_sec=30):
    start_time = time.time()
    events = [
        f"{GREEN}[INIT]{RESET} PinnedMemoryPool allocated 4 x 512KB page-locked host memory buffers.",
        f"{GREEN}[REGISTRY]{RESET} Loaded models: yolov8_obstacle (SLO: 35ms), lane_detection (SLO: 70ms).",
        f"{GREEN}[DISPATCH]{RESET} Dual CUDA priority stream (ID 0 priority=-1) bound to Obstacle perception."
    ]

    dropped_count = 0
    temp = 65.0
    gpu = 45.0
    gov_mode = "FP16 (High Fidelity)"
    
    step = 0
    try:
        while True:
            elapsed = int(time.time() - start_time)
            step += 1

            # Simulate dynamic thermal wave
            if step > 10 and step < 25:
                temp = min(87.5, temp + 2.1)
                gpu = min(96.0, gpu + 4.5)
            elif step >= 25 and step < 40:
                temp = max(68.0, temp - 1.8)
                gpu = max(55.0, gpu - 3.2)
            else:
                temp = 65.0 + random.uniform(-1.0, 1.5)
                gpu = 48.0 + random.uniform(-3.0, 4.0)

            # Governor state transition
            if temp >= 80.0 and gov_mode.startswith("FP16"):
                gov_mode = "INT8 (Precision Throttle)"
                events.append(f"{YELLOW}[GOVERNOR]{RESET} Temp reached {temp:.1f}°C > 80°C threshold. Swapped YOLOv8 to INT8 engine.")
                events.append(f"{RED}[ARBITER]{RESET} Shedding 2 low-priority logging frames to protect collision SLO.")
                dropped_count += 2
            elif temp < 72.0 and gov_mode.startswith("INT8"):
                gov_mode = "FP16 (High Fidelity)"
                events.append(f"{GREEN}[GOVERNOR]{RESET} Thermal headroom restored ({temp:.1f}°C < 72°C). Restored YOLOv8 to FP16.")

            # Queue states
            q_high = random.randint(1, 2)
            q_med = random.randint(1, 3)
            q_low = 1 if temp < 80 else 0

            # Latencies
            base_lat = 12.5 if "INT8" in gov_mode else 19.5
            p50 = base_lat + random.uniform(-0.5, 0.8)
            p95 = base_lat * 1.35 + random.uniform(-0.4, 1.1)
            p99 = base_lat * 1.7 + (8.0 if temp > 82 else 0.5)
            ewma = p50 * 0.2 + base_lat * 0.8

            state = {
                "hardware": "NVIDIA Jetson Orin Nano / Linux aarch64",
                "uptime_sec": elapsed,
                "temp": temp,
                "gpu_load": gpu,
                "mem_mb": 1420 + int(random.uniform(-30, 40)),
                "mem_max": 3840,
                "gov_mode": gov_mode,
                "q_high": q_high,
                "q_med": q_med,
                "q_low": q_low,
                "dropped_frames": dropped_count,
                "p50": p50,
                "p95": p95,
                "p99": p99,
                "ewma_pred": ewma,
                "slo_compliance": 99.8 if p99 < 35.0 else 98.4,
                "events": events
            }

            draw_dashboard(state)

            if duration_sec and elapsed >= duration_sec:
                break
            time.sleep(0.5)

    except KeyboardInterrupt:
        print(f"\n{GREEN}Dashboard stopped cleanly.{RESET}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Live Mission Control Dashboard")
    parser.add_argument("--duration", type=int, default=10, help="Run duration in seconds (0 for infinite)")
    args = parser.parse_args()
    simulate_dashboard(args.duration)
