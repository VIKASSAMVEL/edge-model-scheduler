#include <iostream>
#include <cassert>
#include "ThorQuantizationEngine.hpp"

using namespace edge_scheduler;

void test_thor_precision_transitions() {
    std::cout << "[Thor Test 1] Testing 4-Tier Precision State Transitions... " << std::flush;

    ThorQuantizationEngine engine(70.0f, 78.0f, 84.0f);

    // 1. Initial State (<70°C) -> FP16
    assert(engine.evaluate_telemetry(55.0f) == ThorPrecisionTier::FP16);
    assert(engine.get_bandwidth_scaling() == 1.0f);
    assert(engine.tier_to_string() == "FP16");

    // 2. Heat to 72°C -> FP8
    assert(engine.evaluate_telemetry(72.0f) == ThorPrecisionTier::FP8_E4M3);
    assert(engine.get_bandwidth_scaling() == 0.50f);
    assert(engine.tier_to_string() == "FP8_E4M3");

    // 3. Heat to 80°C -> INT8
    assert(engine.evaluate_telemetry(80.0f) == ThorPrecisionTier::INT8);
    assert(engine.get_bandwidth_scaling() == 0.50f);

    // 4. Heat to 86°C -> NVFP4
    assert(engine.evaluate_telemetry(86.0f) == ThorPrecisionTier::NVFP4);
    assert(engine.get_bandwidth_scaling() == 0.25f);
    assert(engine.tier_to_string() == "NVFP4");

    std::cout << "PASSED. (All 4 Tiers Scaled: FP16->FP8->INT8->NVFP4)\n";
}

void test_thor_hysteresis_deadband() {
    std::cout << "[Thor Test 2] Testing Multi-Band Hysteresis Deadband Stability... " << std::flush;

    ThorQuantizationEngine engine(70.0f, 78.0f, 84.0f);
    engine.evaluate_telemetry(86.0f); // Trip to NVFP4

    // Cool down slightly to 82°C (above recovery deadband of 80°C) -> Must STAY NVFP4
    assert(engine.evaluate_telemetry(82.0f) == ThorPrecisionTier::NVFP4);

    // Cool down to 79°C (below deadband) -> Transitions to INT8
    assert(engine.evaluate_telemetry(79.0f) == ThorPrecisionTier::INT8);

    // Cool down to 69°C (above FP16 recovery threshold of 66°C) -> Must STAY FP8
    engine.evaluate_telemetry(75.0f); // In FP8
    assert(engine.evaluate_telemetry(68.0f) == ThorPrecisionTier::FP8_E4M3);

    // Cool down to 65°C -> Recovers to FP16
    assert(engine.evaluate_telemetry(65.0f) == ThorPrecisionTier::FP16);

    std::cout << "PASSED. (Hysteresis Deadbands Eliminating Jitter)\n";
}

int main() {
    std::cout << "==================================================\n";
    std::cout << "⚡ NVIDIA Thor & Blackwell Quantization Test Suite\n";
    std::cout << "==================================================\n";

    test_thor_precision_transitions();
    test_thor_hysteresis_deadband();

    std::cout << "==================================================\n";
    std::cout << "🎉 ALL THOR QUANTIZATION TESTS PASSED!\n";
    std::cout << "==================================================\n";
    return 0;
}
