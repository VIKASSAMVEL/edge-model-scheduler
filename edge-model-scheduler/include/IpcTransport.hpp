#pragma once

#include "ShmRingBuffer.hpp"
#include "CudaIpcAllocator.hpp"
#include "TaskTypes.hpp"

#include <cstdint>
#include <cstring>
#include <string>
#include <chrono>
#include <functional>
#include <atomic>
#include <vector>

namespace edge_scheduler {

/**
 * IPC Message Protocol for Edge Scheduler Client→Daemon Communication
 *
 * Wire format transmitted through ShmRingBuffer slots:
 * ┌─────────────────────────────────────────────────────┐
 * │ IpcRequestHeader (fixed 128 bytes)                  │
 * │  - magic: 0xED5C (EdgeSCheduler)                    │
 * │  - version: protocol version                        │
 * │  - request_type: INFERENCE / CANCEL / HEALTH        │
 * │  - model_id: null-terminated string (64 bytes)      │
 * │  - priority: Priority enum                          │
 * │  - payload_type: CPU_BUFFER / GPU_IPC_HANDLE        │
 * │  - payload_size: size of following payload           │
 * │  - client_pid: sender process ID                    │
 * │  - sequence_id: monotonic per-client request ID     │
 * ├─────────────────────────────────────────────────────┤
 * │ Payload (variable size)                             │
 * │  - Raw frame bytes (CPU_BUFFER)                     │
 * │  - OR IpcGpuHandle struct (GPU_IPC_HANDLE, 80B)     │
 * └─────────────────────────────────────────────────────┘
 */

static constexpr uint16_t IPC_MAGIC = 0xED5C;
static constexpr uint16_t IPC_PROTOCOL_VERSION = 1;

enum class IpcRequestType : uint8_t {
    INFERENCE = 1,
    CANCEL    = 2,
    HEALTH    = 3,
    CONFIG    = 4,
};

enum class IpcPayloadType : uint8_t {
    CPU_BUFFER     = 1,  // Raw frame data follows header
    GPU_IPC_HANDLE = 2,  // IpcGpuHandle struct follows header
    NONE           = 0,
};

struct IpcRequestHeader {
    uint16_t magic{IPC_MAGIC};
    uint16_t version{IPC_PROTOCOL_VERSION};
    IpcRequestType request_type{IpcRequestType::INFERENCE};
    IpcPayloadType payload_type{IpcPayloadType::CPU_BUFFER};
    char model_id[64]{};
    uint32_t priority{static_cast<uint32_t>(Priority::MEDIUM)};
    uint32_t payload_size{0};
    uint32_t client_pid{0};
    uint64_t sequence_id{0};
    uint64_t deadline_ns{0}; // Absolute deadline (0 = no deadline)
    uint8_t reserved[16]{};
};

static_assert(sizeof(IpcRequestHeader) <= 128, "IpcRequestHeader must fit in 128 bytes");

/**
 * IpcResponse: Written back to a per-client response SHM segment.
 */
struct IpcResponse {
    uint16_t magic{IPC_MAGIC};
    uint64_t sequence_id{0};
    uint32_t status_code{0}; // 0=success, 1=timeout, 2=rejected, 3=error
    float inference_latency_ms{0.0f};
    float queue_delay_ms{0.0f};
    uint8_t precision_used{0}; // 0=FP16, 1=INT8
    uint8_t reserved[32]{};
};

/**
 * SchedulerIpcClient: Lightweight SDK for submitting inference requests
 *                     to the EdgeModelScheduler daemon via shared memory.
 *
 * Usage:
 *   SchedulerIpcClient client("/edge_scheduler_ipc");
 *   client.submit_inference("yolov8_safety", Priority::HIGH, frame_data, frame_size);
 */
class SchedulerIpcClient {
public:
    SchedulerIpcClient(const std::string& shm_name,
                       uint32_t slot_count = 16,
                       uint32_t slot_size = 1920 * 1080 * 3 + 128)
        : ring_(shm_name, slot_count, slot_size, ShmRole::PRODUCER),
          sequence_counter_(0) {
#ifdef __linux__
        client_pid_ = static_cast<uint32_t>(getpid());
#else
        client_pid_ = 0;
#endif
    }

    /**
     * Submit a CPU-resident frame buffer for inference.
     * Returns true if the request was accepted (ring not full).
     */
    bool submit_inference(const std::string& model_id, Priority priority,
                          const void* frame_data, uint32_t frame_size) {
        uint64_t seq;
        void* slot = ring_.try_claim_write_slot(seq);
        if (!slot) return false; // Back-pressure: ring full

        // Write header
        IpcRequestHeader hdr;
        hdr.request_type = IpcRequestType::INFERENCE;
        hdr.payload_type = IpcPayloadType::CPU_BUFFER;
        std::strncpy(hdr.model_id, model_id.c_str(), sizeof(hdr.model_id) - 1);
        hdr.priority = static_cast<uint32_t>(priority);
        hdr.payload_size = frame_size;
        hdr.client_pid = client_pid_;
        hdr.sequence_id = sequence_counter_.fetch_add(1, std::memory_order_relaxed);

        auto* dst = static_cast<uint8_t*>(slot);
        std::memcpy(dst, &hdr, sizeof(hdr));

        // Write frame payload directly after header
        if (frame_data && frame_size > 0) {
            std::memcpy(dst + sizeof(hdr), frame_data, frame_size);
        }

        uint64_t ts = static_cast<uint64_t>(
            std::chrono::steady_clock::now().time_since_epoch().count());
        ring_.commit_publish(seq, sizeof(hdr) + frame_size, ts);
        return true;
    }

    /**
     * Submit a GPU-resident tensor via CUDA IPC handle (zero memcpy).
     */
    bool submit_gpu_inference(const std::string& model_id, Priority priority,
                              const IpcGpuHandle& gpu_handle) {
        uint64_t seq;
        void* slot = ring_.try_claim_write_slot(seq);
        if (!slot) return false;

        IpcRequestHeader hdr;
        hdr.request_type = IpcRequestType::INFERENCE;
        hdr.payload_type = IpcPayloadType::GPU_IPC_HANDLE;
        std::strncpy(hdr.model_id, model_id.c_str(), sizeof(hdr.model_id) - 1);
        hdr.priority = static_cast<uint32_t>(priority);
        hdr.payload_size = sizeof(IpcGpuHandle);
        hdr.client_pid = client_pid_;
        hdr.sequence_id = sequence_counter_.fetch_add(1, std::memory_order_relaxed);

        auto* dst = static_cast<uint8_t*>(slot);
        std::memcpy(dst, &hdr, sizeof(hdr));
        std::memcpy(dst + sizeof(hdr), &gpu_handle, sizeof(IpcGpuHandle));

        uint64_t ts = static_cast<uint64_t>(
            std::chrono::steady_clock::now().time_since_epoch().count());
        ring_.commit_publish(seq, sizeof(hdr) + sizeof(IpcGpuHandle), ts);
        return true;
    }

    /**
     * Send a health check ping to the daemon.
     */
    bool ping() {
        uint64_t seq;
        void* slot = ring_.try_claim_write_slot(seq);
        if (!slot) return false;

        IpcRequestHeader hdr;
        hdr.request_type = IpcRequestType::HEALTH;
        hdr.payload_type = IpcPayloadType::NONE;
        hdr.client_pid = client_pid_;
        hdr.sequence_id = sequence_counter_.fetch_add(1, std::memory_order_relaxed);

        std::memcpy(slot, &hdr, sizeof(hdr));
        ring_.commit_publish(seq, sizeof(hdr), 0);
        return true;
    }

    uint64_t requests_sent() const {
        return sequence_counter_.load(std::memory_order_relaxed);
    }

    uint32_t ring_pending() const { return ring_.pending_count(); }

private:
    ShmRingBuffer ring_;
    uint32_t client_pid_;
    std::atomic<uint64_t> sequence_counter_;
};

/**
 * SchedulerIpcServer: Daemon-side consumer that drains the IPC ring
 *                     and dispatches inference tasks into the AdvancedScheduler.
 */
class SchedulerIpcServer {
public:
    using InferenceCallback = std::function<void(
        const IpcRequestHeader& header,
        const void* payload_data)>;

    SchedulerIpcServer(const std::string& shm_name,
                       uint32_t slot_count = 16,
                       uint32_t slot_size = 1920 * 1080 * 3 + 128)
        : ring_(shm_name, slot_count, slot_size, ShmRole::CONSUMER) {}

    /**
     * Poll for incoming requests. Non-blocking.
     * Returns the number of requests processed in this call.
     */
    uint32_t poll(InferenceCallback callback, uint32_t max_batch = 8) {
        uint32_t processed = 0;
        while (processed < max_batch) {
            uint64_t seq;
            uint32_t size;
            uint64_t ts;
            const void* data = ring_.try_consume(seq, size, ts);
            if (!data) break;

            if (size >= sizeof(IpcRequestHeader)) {
                const auto* hdr = static_cast<const IpcRequestHeader*>(data);
                if (hdr->magic == IPC_MAGIC && hdr->version == IPC_PROTOCOL_VERSION) {
                    const void* payload = static_cast<const uint8_t*>(data) + sizeof(IpcRequestHeader);
                    if (callback) {
                        callback(*hdr, payload);
                    }
                    total_processed_++;
                }
            }

            ring_.commit_consume(seq);
            processed++;
        }
        return processed;
    }

    uint64_t total_processed() const { return total_processed_; }
    uint32_t ring_pending() const { return ring_.pending_count(); }

private:
    ShmRingBuffer ring_;
    uint64_t total_processed_{0};
};

} // namespace edge_scheduler
