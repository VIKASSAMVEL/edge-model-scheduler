#pragma once

#include <string>
#include <vector>
#include <memory>
#include <iostream>

namespace edge_scheduler {

enum class PrecisionMode {
    FP16,   // High precision, baseline compute & memory bandwidth
    INT8    // Quantized precision: ~2x throughput, 50% memory bandwidth
};

/**
 * QuantizationGovernor:
 * 
 * Instead of completely shedding frames when thermal throttling occurs,
 * this governor swaps the active execution context to INT8 quantized engines.
 * This cuts tensor memory bandwidth consumption by 50% on Jetson UMA 
 * and drops ALU power draw without missing frames.
 */
class QuantizationGovernor {
public:
    QuantizationGovernor() : current_mode_(PrecisionMode::FP16) {}

    // Evaluate hardware telemetry and decide optimal precision
    PrecisionMode evaluate_telemetry(float gpu_temp_c, float gpu_util_pct) {
        // Hysteresis threshold: degrade to INT8 if temp > 80C or load > 90%
        if (gpu_temp_c > 80.0f || gpu_util_pct > 90.0f) {
            if (current_mode_ != PrecisionMode::INT8) {
                std::cout << "[QuantizationGovernor] HARDWARE PRESSURE DETECTED (Temp: " 
                          << gpu_temp_c << "C, GPU: " << gpu_util_pct 
                          << "%). Dynamic Precision Fallback: Switching to INT8." << std::endl;
                current_mode_ = PrecisionMode::INT8;
            }
        } 
        // Recover to FP16 when safe (Temp < 72C and load < 75%)
        else if (gpu_temp_c < 72.0f && gpu_util_pct < 75.0f) {
            if (current_mode_ != PrecisionMode::FP16) {
                std::cout << "[QuantizationGovernor] Headroom Restored (Temp: " 
                          << gpu_temp_c << "C). Restoring Precision to FP16." << std::endl;
                current_mode_ = PrecisionMode::FP16;
            }
        }
        return current_mode_;
    }

    PrecisionMode get_current_mode() const { return current_mode_; }

private:
    PrecisionMode current_mode_;
};

} // namespace edge_scheduler
