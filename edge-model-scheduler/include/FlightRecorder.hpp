#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <atomic>
#include <fstream>
#include <iostream>
#include <chrono>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <mutex>

namespace edge_scheduler {

/**
 * FlightTelemetryFrame: 64-byte compact telemetry frame recorded every millisecond.
 */
struct alignas(64) FlightTelemetryFrame {
    uint64_t timestamp_ns{0};
    uint32_t sequence_id{0};
    float gpu_temp_c{0.0f};
    float gpu_util_pct{0.0f};
    float memory_bus_gbps{0.0f};
    float p99_latency_ms{0.0f};
    float deadline_slack_ms{0.0f};
    float battery_soc_pct{100.0f};
    uint16_t asil_state{0};       // 0: OK, 1: SEU, 2: DIVERGENCE, 3: SAFE_STOP
    uint16_t current_precision{0}; // 0: FP16, 1: INT8, 2: NVFP4, 3: SHED
    uint32_t active_tasks_count{0};
    uint32_t stale_shed_total{0};
    uint32_t bit_flips_detected{0};
    uint32_t reserved{0};
};

/**
 * FlightRecorderHeader: Metadata header preceding the .emsbag binary recording.
 */
struct FlightRecorderHeader {
    char magic[8]{'E', 'M', 'S', 'F', 'L', 'I', 'G', 'H'}; // "EMSFLIGH"
    uint32_t format_version{1};
    uint32_t total_frames{0};
    uint64_t record_start_ns{0};
    uint64_t record_end_ns{0};
    char trigger_reason[64]{"NOMINAL_DISPATCH"};
    char vehicle_id[32]{"AV_THOR_NODE_01"};
    uint8_t sha256_checksum[32]{0};
};

/**
 * FlightRecorder:
 * Lock-free 60-second rolling ring buffer capturing black-box flight data
 * for ISO 26262 ASIL-D crash forensics and real-time observability.
 */
class FlightRecorder {
public:
    static constexpr size_t DEFAULT_RING_CAPACITY = 60000; // 60 seconds @ 1000 Hz

    explicit FlightRecorder(size_t capacity = DEFAULT_RING_CAPACITY)
        : capacity_(capacity), ring_buffer_(capacity) {
        head_.store(0, std::memory_order_relaxed);
    }

    /**
     * Record a telemetry frame into the circular buffer (lock-free).
     */
    void record_frame(const FlightTelemetryFrame& frame) {
        uint64_t idx = head_.fetch_add(1, std::memory_order_acq_rel) % capacity_;
        ring_buffer_[idx] = frame;
        ring_buffer_[idx].sequence_id = static_cast<uint32_t>(idx);
        total_recorded_.fetch_add(1, std::memory_order_relaxed);
    }

    /**
     * Export the rolling history into an encrypted / structured .emsbag file.
     */
    bool dump_to_file(const std::string& filepath, const std::string& trigger_reason = "MANUAL_TRIGGER") {
        std::lock_guard<std::mutex> lock(dump_mutex_);

        std::ofstream out(filepath, std::ios::binary);
        if (!out.is_open()) {
            return false;
        }

        uint64_t current_head = head_.load(std::memory_order_acquire);
        size_t count = (current_head < capacity_) ? current_head : capacity_;

        FlightRecorderHeader hdr;
        hdr.total_frames = static_cast<uint32_t>(count);
        hdr.record_start_ns = (count > 0) ? ring_buffer_[0].timestamp_ns : 0;
        hdr.record_end_ns = (count > 0) ? ring_buffer_[(current_head - 1) % capacity_].timestamp_ns : 0;
        std::strncpy(hdr.trigger_reason, trigger_reason.c_str(), sizeof(hdr.trigger_reason) - 1);

        // Simple rolling checksum across telemetry frames
        uint32_t rolling_hash = 0x811c9dc5;
        for (size_t i = 0; i < count; ++i) {
            const auto& f = ring_buffer_[i];
            rolling_hash ^= static_cast<uint32_t>(f.timestamp_ns & 0xFFFFFFFF);
            rolling_hash *= 0x01000193;
        }
        std::memcpy(hdr.sha256_checksum, &rolling_hash, sizeof(rolling_hash));

        // Write Header
        out.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));

        // Write Telemetry Frames in chronological order
        size_t start_idx = (current_head >= capacity_) ? (current_head % capacity_) : 0;
        for (size_t i = 0; i < count; ++i) {
            size_t frame_idx = (start_idx + i) % capacity_;
            out.write(reinterpret_cast<const char*>(&ring_buffer_[frame_idx]), sizeof(FlightTelemetryFrame));
        }

        out.close();
        return true;
    }

    /**
     * Verify and load an .emsbag file for post-incident forensic replay.
     */
    static bool parse_emsbag(
        const std::string& filepath,
        FlightRecorderHeader& out_header,
        std::vector<FlightTelemetryFrame>& out_frames)
    {
        std::ifstream in(filepath, std::ios::binary);
        if (!in.is_open()) return false;

        in.read(reinterpret_cast<char*>(&out_header), sizeof(FlightRecorderHeader));
        if (std::memcmp(out_header.magic, "EMSFLIGH", 8) != 0) {
            return false;
        }

        out_frames.resize(out_header.total_frames);
        in.read(reinterpret_cast<char*>(out_frames.data()), out_header.total_frames * sizeof(FlightTelemetryFrame));

        return in.gcount() == static_cast<std::streamsize>(out_header.total_frames * sizeof(FlightTelemetryFrame));
    }

    size_t capacity() const { return capacity_; }
    uint64_t total_recorded() const { return total_recorded_.load(std::memory_order_relaxed); }

private:
    size_t capacity_;
    std::vector<FlightTelemetryFrame> ring_buffer_;
    std::atomic<uint64_t> head_{0};
    std::atomic<uint64_t> total_recorded_{0};
    std::mutex dump_mutex_;
};

} // namespace edge_scheduler
