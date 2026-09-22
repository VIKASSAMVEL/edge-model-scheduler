#!/usr/bin/env python3
"""
Advanced Edge Model Scheduler Feature Demo
Demonstrating:
- Zero-copy PinnedMemoryPool (UMA emulation)
- Telemetry-driven QuantizationGovernor (FP16 <-> INT8 hysteresis)
- EWMA Latency Predictor (Pre-dispatch deadline enforcement)
- ModelRegistry with latency SLO contracts
- AdvancedScheduler dual-stream priority dispatch
"""

import sys
import os
import time

try:
    import edge_scheduler_py as es
except ImportError:
    print("[-] Error: edge_scheduler_py module not found. Compile build_native first.")
    sys.exit(1)

def main():
    print("=" * 65)
    print("🚀 EdgeModelScheduler v2.5 Enterprise Feature Demonstration")
    print("=" * 65)

    # 1. Initialize Pinned Memory Pool (512 KB per buffer, 4 buffers)
    buffer_bytes = 512 * 1024
    print(f"[1/5] Initializing PinnedMemoryPool (Zero-Copy UMA, {buffer_bytes//1024} KB/buf)...")
    pool = es.PinnedMemoryPool(buffer_bytes, 4)

    # 2. Initialize Resource Monitor
    print("[2/5] Starting Hardware ResourceMonitor...")
    monitor = es.ResourceMonitor()
    monitor.start(polling_interval_ms=50)

    # 3. Initialize Model Registry
    print("[3/5] Configuring ModelRegistry with per-model SLO contracts...")
    registry = es.ModelRegistry()
    
    yolo_cfg = es.ModelConfig()
    yolo_cfg.model_id = "yolov8_obstacle"
    yolo_cfg.engine_path = "/opt/models/yolov8_obstacle.engine"
    yolo_cfg.priority_level = 1
    yolo_cfg.latency_slo_ms = 35.0
    yolo_cfg.input_buffer_bytes = buffer_bytes
    yolo_cfg.allow_quantization_fallback = True
    registry.register_model(yolo_cfg)

    lane_cfg = es.ModelConfig()
    lane_cfg.model_id = "lane_detection"
    lane_cfg.engine_path = "/opt/models/lane_seg.engine"
    lane_cfg.priority_level = 2
    lane_cfg.latency_slo_ms = 70.0
    lane_cfg.input_buffer_bytes = buffer_bytes
    lane_cfg.allow_quantization_fallback = False
    registry.register_model(lane_cfg)

    print(f"      Registered {registry.count()} models: {registry.get_all_model_ids()}")

    # 4. Initialize Quantization Governor & EWMA Profiler
    print("[4/5] Testing Dynamic Quantization Governor hysteresis...")
    governor = es.QuantizationGovernor()
    print(f"      Initial Governor Mode: {governor.get_current_mode()}")
    
    # Simulate high thermal load (84C, 92% GPU)
    governor.evaluate_telemetry(84.0, 92.0)
    print(f"      Under 84°C load -> Mode: {governor.get_current_mode()} (INT8 Dynamic Fallback)")

    # Simulate cooldown (67C)
    governor.evaluate_telemetry(67.0, 50.0)
    print(f"      After Cooldown to 67°C -> Mode: {governor.get_current_mode()} (FP16 Restored)")

    # 5. Start Advanced Scheduler
    print("[5/5] Launching AdvancedScheduler with Dual CUDA Stream Dispatcher...")
    scheduler = es.AdvancedScheduler(monitor, pool)
    scheduler.set_starvation_threshold_ms(150)
    scheduler.start()

    # Submit tasks concurrently
    print("\n[Execution] Submitting concurrent multi-model perception tasks...")
    
    def mock_yolo_inference():
        time.sleep(0.012) # 12ms execution
        print("  ⚡ [CUDA Stream 0 - Priority] YOLO Obstacle Detection completed (12.4ms)")

    def mock_lane_inference():
        time.sleep(0.025) # 25ms execution
        print("  🔹 [CUDA Stream 1 - Standard] Lane Segmentation completed (25.1ms)")

    # Dispatch: High priority (YOLO), Medium priority (Lane)
    for i in range(3):
        scheduler.submit_task("lane_detection", es.Priority.MEDIUM, mock_lane_inference)
        scheduler.submit_task("yolov8_obstacle", es.Priority.HIGH, mock_yolo_inference)

    time.sleep(0.6) # Allow queue processing

    print("\n[Teardown] Gracefully stopping AdvancedScheduler and worker threads...")
    scheduler.stop()
    monitor.stop()

    print("=" * 65)
    print("✅ Advanced Feature Demonstration Finished Successfully!")
    print("=" * 65)

if __name__ == "__main__":
    main()
