#!/usr/bin/env python3
"""
EdgeModelScheduler v2.5 - Production Edge API Microservice
Provides REST endpoints and WebSocket telemetry stream for edge robotics clusters,
Kubernetes Edge (K3s/MicroK8s), and mission control dashboards.
"""

import sys
import os
import time
import asyncio
from typing import Optional, List, Dict, Any
from pydantic import BaseModel, Field

try:
    from fastapi import FastAPI, HTTPException, WebSocket, WebSocketDisconnect, status
    from fastapi.responses import PlainTextResponse, JSONResponse
    import uvicorn
except ImportError:
    print("[EdgeAPI] FastAPI/Uvicorn not installed. Please install: pip install fastapi uvicorn")
    # Provide lightweight mock server capability or exit gracefully if run directly
    FastAPI = None

# Initialize FastAPI application
app = FastAPI(
    title="EdgeModelScheduler Production API",
    description="Enterprise Edge AI Dynamic Resource Arbiter for NVIDIA Jetson & Autonomous Systems",
    version="2.5.0",
    docs_url="/docs",
    redoc_url="/redoc"
) if FastAPI else None

# Pydantic Schemas
class ModelRegisterRequest(BaseModel):
    model_id: str = Field(..., min_length=2, max_length=64, description="Unique model identifier")
    engine_path: str = Field(..., description="Absolute path to TensorRT/ONNX engine file")
    priority_level: int = Field(..., ge=1, le=3, description="1=HIGH, 2=MEDIUM, 3=LOW")
    latency_slo_ms: float = Field(..., gt=0.0, le=1000.0, description="Strict latency deadline in ms")
    input_buffer_bytes: int = Field(default=524288, gt=0, description="Allocated pinned memory in bytes")
    allow_quantization_fallback: bool = Field(default=True, description="Enable FP16->INT8 governor downscale")

class InferenceRequest(BaseModel):
    model_id: str = Field(..., description="Target registered model ID")
    priority: Optional[str] = Field("HIGH", regex="^(HIGH|MEDIUM|LOW)$")
    payload_base64: Optional[str] = Field(None, description="Optional raw image/tensor payload")
    timeout_ms: Optional[float] = Field(50.0, gt=0.0, description="Request timeout budget in ms")

class InferenceResponse(BaseModel):
    request_id: str
    model_id: str
    status: str
    execution_time_ms: float
    precision_mode: str
    stream_id: int
    queue_delay_ms: float
    slo_met: bool

# Mock / Bound State
SYSTEM_STATE = {
    "is_healthy": True,
    "uptime_start": time.time(),
    "active_models": {
        "yolov8_obstacle": {
            "engine_path": "/opt/models/yolov8_obstacle.engine",
            "priority": 1,
            "slo_ms": 35.0,
            "allow_quantization": True
        },
        "lane_detection": {
            "engine_path": "/opt/models/lane_seg.engine",
            "priority": 2,
            "slo_ms": 70.0,
            "allow_quantization": False
        }
    },
    "telemetry": {
        "soc_temp_c": 66.4,
        "gpu_utilization_pct": 52.1,
        "uma_vram_used_mb": 1410,
        "uma_vram_total_mb": 3840,
        "governor_mode": "FP16",
        "dropped_frames_total": 0
    }
}

if app:
    @app.get("/healthz", tags=["Probes"], summary="Kubernetes / Edge Liveness Probe")
    async def liveness():
        """Returns 200 if the scheduler core daemon and worker threads are healthy."""
        if SYSTEM_STATE["is_healthy"]:
            return {"status": "healthy", "uptime_sec": int(time.time() - SYSTEM_STATE["uptime_start"])}
        raise HTTPException(status_code=status.HTTP_503_SERVICE_UNAVAILABLE, detail="Scheduler daemon unhealthy")

    @app.get("/readyz", tags=["Probes"], summary="Kubernetes / Edge Readiness Probe")
    async def readiness():
        """Returns 200 if model registry has at least one valid engine loaded."""
        if len(SYSTEM_STATE["active_models"]) > 0:
            return {"status": "ready", "loaded_models": list(SYSTEM_STATE["active_models"].keys())}
        raise HTTPException(status_code=status.HTTP_503_SERVICE_UNAVAILABLE, detail="No models registered in engine pool")

    @app.get("/metrics", response_class=PlainTextResponse, tags=["Observability"], summary="Prometheus Metrics Endpoint")
    async def prometheus_metrics():
        """Exports OpenTelemetry & Prometheus-compatible metrics for Grafana scraping."""
        tel = SYSTEM_STATE["telemetry"]
        uptime = int(time.time() - SYSTEM_STATE["uptime_start"])
        gov_num = 1 if tel["governor_mode"] == "FP16" else 0

        lines = [
            "# HELP edge_scheduler_uptime_seconds Total runtime of the scheduler",
            "# TYPE edge_scheduler_uptime_seconds counter",
            f"edge_scheduler_uptime_seconds {uptime}",
            "# HELP edge_gpu_temp_celsius Tegra SoC die temperature",
            "# TYPE edge_gpu_temp_celsius gauge",
            f"edge_gpu_temp_celsius {tel['soc_temp_c']:.2f}",
            "# HELP edge_gpu_utilization_percent GPU SM engine load",
            "# TYPE edge_gpu_utilization_percent gauge",
            f"edge_gpu_utilization_percent {tel['gpu_utilization_pct']:.2f}",
            "# HELP edge_quantization_governor_mode 1 for FP16, 0 for INT8 throttle",
            "# TYPE edge_quantization_governor_mode gauge",
            f"edge_quantization_governor_mode {gov_num}",
            "# HELP edge_dropped_frames_total Low priority frames dropped for thermal protection",
            "# TYPE edge_dropped_frames_total counter",
            f"edge_dropped_frames_total {tel['dropped_frames_total']}"
        ]
        return "\n".join(lines) + "\n"

    @app.get("/api/v1/telemetry", tags=["Telemetry"], summary="Current Hardware & Arbiter State")
    async def get_telemetry():
        return {
            "timestamp": time.time(),
            "telemetry": SYSTEM_STATE["telemetry"],
            "queue_depths": {"HIGH": 0, "MEDIUM": 1, "LOW": 0}
        }

    @app.get("/api/v1/models", tags=["Registry"], summary="List Registered Perception Models")
    async def list_models():
        return {"count": len(SYSTEM_STATE["active_models"]), "models": SYSTEM_STATE["active_models"]}

    @app.post("/api/v1/models", tags=["Registry"], summary="Dynamically Register a Model")
    async def register_model(req: ModelRegisterRequest):
        if req.model_id in SYSTEM_STATE["active_models"]:
            raise HTTPException(status_code=400, detail=f"Model {req.model_id} already registered.")
        SYSTEM_STATE["active_models"][req.model_id] = {
            "engine_path": req.engine_path,
            "priority": req.priority_level,
            "slo_ms": req.latency_slo_ms,
            "allow_quantization": req.allow_quantization_fallback
        }
        return {"status": "registered", "model_id": req.model_id}

    @app.post("/api/v1/infer", response_model=InferenceResponse, tags=["Inference"], summary="Submit Inference Task")
    async def submit_inference(req: InferenceRequest):
        if req.model_id not in SYSTEM_STATE["active_models"]:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail=f"Model '{req.model_id}' is not registered in the edge engine pool."
            )
        
        cfg = SYSTEM_STATE["active_models"][req.model_id]
        
        # Simulate priority-aware dispatch latency
        start_ns = time.time_ns()
        queue_delay = 1.2 if req.priority == "HIGH" else 8.5
        exec_latency = 12.4 if cfg["priority"] == 1 else 24.8
        await asyncio.sleep(exec_latency / 1000.0) # non-blocking async
        total_time = (time.time_ns() - start_ns) / 1_000_000.0

        slo_met = total_time <= cfg["slo_ms"]

        return InferenceResponse(
            request_id=f"req_{int(time.time()*1000)}",
            model_id=req.model_id,
            status="SUCCESS",
            execution_time_ms=round(exec_latency, 2),
            precision_mode=SYSTEM_STATE["telemetry"]["governor_mode"],
            stream_id=0 if req.priority == "HIGH" else 1,
            queue_delay_ms=round(queue_delay, 2),
            slo_met=slo_met
        )

    @app.websocket("/api/v1/stream")
    async def websocket_telemetry_stream(websocket: WebSocket):
        """Streams real-time metrics and event updates at 10Hz to connected clients."""
        await websocket.accept()
        try:
            while True:
                data = {
                    "timestamp": time.time(),
                    "temp": SYSTEM_STATE["telemetry"]["soc_temp_c"],
                    "gpu": SYSTEM_STATE["telemetry"]["gpu_utilization_pct"],
                    "governor": SYSTEM_STATE["telemetry"]["governor_mode"]
                }
                await websocket.send_json(data)
                await asyncio.sleep(0.1)
        except WebSocketDisconnect:
            pass

def main():
    if not FastAPI:
        print("[EdgeAPI] Cannot start server without fastapi. Run: pip install fastapi uvicorn")
        sys.exit(1)
    port = int(os.environ.get("EDGE_API_PORT", 8080))
    print(f"[*] Starting EdgeModelScheduler API Server on 0.0.0.0:{port}...")
    uvicorn.run("edge_api_server:app", host="0.0.0.0", port=port, log_level="info")

if __name__ == "__main__":
    main()
