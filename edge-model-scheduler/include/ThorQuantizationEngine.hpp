#pragma once

#include <iostream>
#include <string>
#include <atomic>
#include <algorithm>

namespace edge_scheduler {

/**
 * ═══════════════════════════════════════════════════════════════
 * ThorQuantizationEngine: 4-Tier Blackwell & Thor Precision Governor
 * 
 * Supports next-generation NVIDIA DRIVE Thor / Blackwell silicon (2025-2027):
 *   - Tier 1: FP16     (Nominal, < 70°C)  -> 1.0x memory bandwidth baseline
 *   - Tier 2: FP8_E4M3 (Warm, 70°C-78°C)  -> 0.50x bandwidth (2x compute throughput)
 *   - Tier 3: INT8     (Hot, 78°C-84°C)   -> 0.50x bandwidth (calibrated integer)
 *   - Tier 4: NVFP4    (Extreme, > 84°C)  -> 0.25x bandwidth (4x compute throughput)
 * 
 * Prevents catastrophic DVFS clock throttling by dynamically shedding
 * memory controller and ALU load before the silicon hits 85°C trip limits.
 * ═══════════════════════════════════════════════════════════════
 */

enum class ThorPrecisionTier {
    FP16 = 0,
    FP8_E4M3 = 1,
    INT8 = 2,
    NVFP4 = 3
};

class ThorQuantizationEngine {
public:
    ThorQuantizationEngine(float fp8_trip_c = 70.0f,
                           float int8_trip_c = 78.0f,
                           float fp4_trip_c = 84.0f)
        : fp8_trip_c_(fp8_trip_c),
          int8_trip_c_(int8_trip_c),
          fp4_trip_c_(fp4_trip_c),
          current_tier_(ThorPrecisionTier::FP16) {}

    // Multi-band hysteresis telemetry evaluation
    ThorPrecisionTier evaluate_telemetry(float temp_c, float gpu_util_pct = 50.0f) {
        ThorPrecisionTier prev = current_tier_;

        // Emergency Thermal Trip: NVFP4 (>84°C or GPU util > 95%)
        if (temp_c >= fp4_trip_c_ || gpu_util_pct > 95.0f) {
            current_tier_ = ThorPrecisionTier::NVFP4;
        }
        // Hot Trip: INT8 (78°C - 84°C)
        else if (temp_c >= int8_trip_c_) {
            if (prev == ThorPrecisionTier::NVFP4) {
                // Hysteresis: only recover from NVFP4 to INT8 if temp drops below 80°C
                if (temp_c < fp4_trip_c_ - 4.0f) {
                    current_tier_ = ThorPrecisionTier::INT8;
                }
            } else {
                current_tier_ = ThorPrecisionTier::INT8;
            }
        }
        // Warm Trip: FP8 (70°C - 78°C)
        else if (temp_c >= fp8_trip_c_) {
            if (prev == ThorPrecisionTier::INT8 || prev == ThorPrecisionTier::NVFP4) {
                // Hysteresis: only recover from INT8 to FP8 if temp drops below 74°C
                if (temp_c < int8_trip_c_ - 4.0f) {
                    current_tier_ = ThorPrecisionTier::FP8_E4M3;
                }
            } else {
                current_tier_ = ThorPrecisionTier::FP8_E4M3;
            }
        }
        // Headroom Restored: FP16 (<70°C with 4°C deadband recovery at <66°C)
        else {
            if (prev != ThorPrecisionTier::FP16) {
                if (temp_c < fp8_trip_c_ - 4.0f) {
                    current_tier_ = ThorPrecisionTier::FP16;
                }
            } else {
                current_tier_ = ThorPrecisionTier::FP16;
            }
        }

        return current_tier_;
    }

    float get_bandwidth_scaling() const {
        switch (current_tier_) {
            case ThorPrecisionTier::FP16:     return 1.0f;
            case ThorPrecisionTier::FP8_E4M3: return 0.50f;
            case ThorPrecisionTier::INT8:     return 0.50f;
            case ThorPrecisionTier::NVFP4:    return 0.25f;
            default:                          return 1.0f;
        }
    }

    std::string tier_to_string() const {
        switch (current_tier_) {
            case ThorPrecisionTier::FP16:     return "FP16";
            case ThorPrecisionTier::FP8_E4M3: return "FP8_E4M3";
            case ThorPrecisionTier::INT8:     return "INT8";
            case ThorPrecisionTier::NVFP4:    return "NVFP4";
            default:                          return "UNKNOWN";
        }
    }

    ThorPrecisionTier get_current_tier() const { return current_tier_; }
    void force_tier(ThorPrecisionTier tier) { current_tier_ = tier; }

private:
    float fp8_trip_c_;
    float int8_trip_c_;
    float fp4_trip_c_;
    ThorPrecisionTier current_tier_;
};

} // namespace edge_scheduler
