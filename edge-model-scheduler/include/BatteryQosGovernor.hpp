#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <mutex>
#include <atomic>
#include <fstream>
#include <cmath>
#include <unordered_map>
#include <algorithm>

#ifndef NOMINMAX
#define NOMINMAX
#endif

namespace edge_scheduler {

enum class BatteryQosMode {
    EXTERNAL_POWER = 0, // Tethered / Charging / Max compute
    NORMAL = 1,         // SoC > 40%: All pipelines 100% rate
    ECO = 2,            // SoC 20%-40%: Background pipelines duty-cycled 50%
    CRITICAL_RTH = 3    // SoC < 20%: Return to Home, non-critical models paused
};

struct BatteryTelemetry {
    float soc_percent{100.0f};      // State of Charge (0.0 - 100.0)
    float voltage_v{16.8f};         // 4S LiPo nominal
    float current_a{2.5f};          // Total system draw
    float power_w{42.0f};           // Calculated power in Watts
    bool is_charging{false};
    uint64_t timestamp_ms{0};
};

class BatteryQosGovernor {
public:
    explicit BatteryQosGovernor(float eco_threshold = 40.0f, float critical_threshold = 20.0f)
        : eco_threshold_(eco_threshold), critical_threshold_(critical_threshold) {}

    // Update battery state directly (from ROS 2 sensor_msgs/BatteryState or daemon poll)
    void update_telemetry(float soc, bool is_charging = false, float voltage = 16.8f, float current = 2.5f) {
        std::lock_guard<std::mutex> lock(mutex_);
        telemetry_.soc_percent = (std::max)(0.0f, (std::min)(100.0f, soc));
        telemetry_.is_charging = is_charging;
        telemetry_.voltage_v = voltage;
        telemetry_.current_a = current;
        telemetry_.power_w = voltage * current;
        telemetry_.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();

        reevaluate_mode();
    }

    // Try reading Linux sysfs battery supply (/sys/class/power_supply/BAT0/capacity)
    bool poll_sysfs_battery(const std::string& path = "/sys/class/power_supply/BAT0") {
        std::ifstream cap_file(path + "/capacity");
        if (!cap_file.is_open()) {
            return false;
        }
        float cap = 100.0f;
        cap_file >> cap;

        std::ifstream stat_file(path + "/status");
        std::string status = "Discharging";
        if (stat_file.is_open()) {
            stat_file >> status;
        }
        bool charging = (status == "Charging" || status == "Full");
        update_telemetry(cap, charging);
        return true;
    }

    BatteryQosMode current_mode() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return current_mode_;
    }

    std::string mode_to_string() const {
        switch (current_mode()) {
            case BatteryQosMode::EXTERNAL_POWER: return "EXTERNAL_POWER";
            case BatteryQosMode::NORMAL: return "NORMAL";
            case BatteryQosMode::ECO: return "ECO_CONSERVING";
            case BatteryQosMode::CRITICAL_RTH: return "CRITICAL_RTH";
        }
        return "UNKNOWN";
    }

    BatteryTelemetry telemetry() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return telemetry_;
    }

    // Task admission filter with duty-cycling for lower priority workloads
    // Priority 0: Safety/Emergency Obstacle -> ALWAYS admitted (100%)
    // Priority 1: Primary perception -> Admitted in NORMAL, ECO, duty-cycled in RTH
    // Priority 2: Background SLAM / Mapping -> 100% in NORMAL, 50% in ECO, Paused in RTH
    // Priority 3: Telemetry / Mesh -> 100% in NORMAL, 25% in ECO, Paused in RTH
    bool should_admit_task(const std::string& model_name, uint32_t priority) {
        if (priority == 0) return true; // Safety invariant: never throttle obstacle detection!

        std::lock_guard<std::mutex> lock(mutex_);
        uint64_t count = ++task_counters_[model_name];

        switch (current_mode_) {
            case BatteryQosMode::EXTERNAL_POWER:
            case BatteryQosMode::NORMAL:
                return true; // All admitted

            case BatteryQosMode::ECO:
                if (priority == 1) return true;
                if (priority == 2) return (count % 2 == 0); // 50% duty cycle
                if (priority >= 3) return (count % 4 == 0); // 25% duty cycle
                break;

            case BatteryQosMode::CRITICAL_RTH:
                if (priority == 1) return (count % 3 == 0); // Throttle primary perception to 33%
                if (priority >= 2) return false;            // Drop all background/non-critical!
                break;
        }
        return true;
    }

    // Suggested inference precision under battery QoS
    // In ECO or RTH, non-critical models should switch to INT8 to save energy
    bool should_force_int8(uint32_t priority) const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (current_mode_ == BatteryQosMode::CRITICAL_RTH) return true;
        if (current_mode_ == BatteryQosMode::ECO && priority >= 2) return true;
        return false;
    }

private:
    void reevaluate_mode() {
        if (telemetry_.is_charging) {
            current_mode_ = BatteryQosMode::EXTERNAL_POWER;
            return;
        }

        float soc = telemetry_.soc_percent;
        const float hysteresis = 2.0f; // 2% deadband to avoid oscillation

        if (current_mode_ == BatteryQosMode::CRITICAL_RTH) {
            if (soc > critical_threshold_ + hysteresis) {
                current_mode_ = BatteryQosMode::ECO;
            }
        } else if (current_mode_ == BatteryQosMode::ECO) {
            if (soc <= critical_threshold_) {
                current_mode_ = BatteryQosMode::CRITICAL_RTH;
            } else if (soc > eco_threshold_ + hysteresis) {
                current_mode_ = BatteryQosMode::NORMAL;
            }
        } else { // NORMAL or EXTERNAL_POWER
            if (soc <= critical_threshold_) {
                current_mode_ = BatteryQosMode::CRITICAL_RTH;
            } else if (soc <= eco_threshold_) {
                current_mode_ = BatteryQosMode::ECO;
            } else {
                current_mode_ = BatteryQosMode::NORMAL;
            }
        }
    }

    float eco_threshold_{40.0f};
    float critical_threshold_{20.0f};
    mutable std::mutex mutex_;
    BatteryTelemetry telemetry_;
    BatteryQosMode current_mode_{BatteryQosMode::NORMAL};
    std::unordered_map<std::string, uint64_t> task_counters_;
};

} // namespace edge_scheduler
