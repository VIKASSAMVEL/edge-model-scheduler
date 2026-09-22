#!/usr/bin/env python3
"""
Comprehensive Automated Pytest Suite for EdgeModelScheduler Python Bindings
Validates zero-copy memory pool, hysteresis governor, EWMA profiler, 
model registry, and priority dispatch.
"""

import sys
import os
import time
import pytest

# Ensure build_native is in python path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "build_native")))

try:
    import edge_scheduler_py as es
    BINDINGS_AVAILABLE = True
except ImportError:
    BINDINGS_AVAILABLE = False

pytestmark = pytest.mark.skipif(not BINDINGS_AVAILABLE, reason="edge_scheduler_py native module not compiled")

class TestPinnedMemoryPool:
    def test_pool_creation_and_size(self):
        pool = es.PinnedMemoryPool(1024 * 256, 4)
        assert pool.buffer_size() == 1024 * 256

class TestQuantizationGovernor:
    def test_initial_mode(self):
        gov = es.QuantizationGovernor()
        assert gov.get_current_mode() == es.PrecisionMode.FP16

    def test_thermal_hysteresis(self):
        gov = es.QuantizationGovernor()
        
        # Normal temp: remains FP16
        gov.evaluate_telemetry(65.0, 50.0)
        assert gov.get_current_mode() == es.PrecisionMode.FP16

        # Critical temp (>80°C): drops to INT8
        gov.evaluate_telemetry(83.5, 88.0)
        assert gov.get_current_mode() == es.PrecisionMode.INT8

        # Hysteresis buffer: intermediate temp (76°C) must NOT prematurely switch back to FP16
        gov.evaluate_telemetry(76.0, 60.0)
        assert gov.get_current_mode() == es.PrecisionMode.INT8

        # Safe cooling (<72°C): restores FP16
        gov.evaluate_telemetry(69.0, 55.0)
        assert gov.get_current_mode() == es.PrecisionMode.FP16

class TestEWMAProfiler:
    def test_latency_tracking(self):
        profiler = es.EWMAExecutionProfiler(0.3)
        profiler.record_execution_time(20.0)
        profiler.record_execution_time(22.0)
        profiler.record_execution_time(21.0)
        
        expected = profiler.get_expected_latency_ms()
        assert 19.0 <= expected <= 23.0

    def test_deadline_prediction(self):
        profiler = es.EWMAExecutionProfiler(0.3)
        # Needs >= 3 samples for calibration
        profiler.record_execution_time(25.0)
        profiler.record_execution_time(25.0)
        profiler.record_execution_time(25.0)
        
        # Queue delay 15ms + 25ms execution = 40ms > SLO 30ms -> Must flag deadline miss
        assert profiler.will_exceed_deadline(30.0, 15.0) is True

        # Queue delay 5ms + 25ms execution = 30ms < SLO 50ms -> Within deadline
        assert profiler.will_exceed_deadline(50.0, 5.0) is False

class TestModelRegistry:
    def test_registration_and_lookup(self):
        registry = es.ModelRegistry()
        cfg = es.ModelConfig()
        cfg.model_id = "test_yolo"
        cfg.engine_path = "/tmp/test.engine"
        cfg.priority_level = 1
        cfg.latency_slo_ms = 40.0
        cfg.input_buffer_bytes = 1024 * 512
        cfg.allow_quantization_fallback = True

        assert registry.register_model(cfg) is True
        assert registry.has_model("test_yolo") is True
        assert registry.count() == 1
        assert "test_yolo" in registry.get_all_model_ids()

class TestAdvancedScheduler:
    def test_scheduler_task_dispatch(self):
        monitor = es.ResourceMonitor()
        pool = es.PinnedMemoryPool(1024 * 64, 2)
        scheduler = es.AdvancedScheduler(monitor, pool)
        
        scheduler.start()
        
        executed = []
        def task_payload(name):
            executed.append(name)

        scheduler.submit_task("yolo", es.Priority.HIGH, lambda: task_payload("yolo"))
        scheduler.submit_task("log", es.Priority.LOW, lambda: task_payload("log"))
        
        time.sleep(0.1)
        scheduler.stop()

        assert "yolo" in executed
        assert "log" in executed

if __name__ == "__main__":
    pytest.main([__file__, "-v"])
