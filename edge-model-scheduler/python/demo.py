import time
import threading

# Assuming edge_scheduler_py is compiled and in PYTHONPATH
try:
    import edge_scheduler_py as es
except ImportError:
    print("Warning: edge_scheduler_py not found. Please compile the C++ bindings first.")
    exit(1)

def run_inference_high_priority():
    print("Executing HIGH priority inference logic...")
    time.sleep(0.02) # Mock inference time (20ms)

def run_inference_low_priority():
    print("Executing LOW priority inference logic...")
    time.sleep(0.05) # Mock inference time (50ms)

def main():
    # 1. Start Resource Monitor
    monitor = es.ResourceMonitor()
    monitor.start(polling_interval_ms=100)

    # 2. Start Scheduler
    scheduler = es.Scheduler(monitor)
    scheduler.start()

    # 3. Load TRT Engines (Mock)
    trt_runner = es.TensorRTRunner()
    trt_runner.load_engine("yolov8_model.engine")

    print("Submitting tasks...")

    # Submit a bunch of tasks to simulate concurrency
    # We submit a low priority first, then high priority. 
    # High priority should execute first if they hit the queue together.
    for i in range(5):
        scheduler.submit_task("log_model", es.Priority.LOW, run_inference_low_priority)
        scheduler.submit_task("obstacle_model", es.Priority.HIGH, lambda: trt_runner.infer())

    time.sleep(1) # Let the scheduler run

    print("Stopping system...")
    scheduler.stop()
    monitor.stop()

if __name__ == "__main__":
    main()
