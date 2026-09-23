#include <iostream>
#include <cassert>
#include <vector>
#include <chrono>

#include "SafetyIslandGovernor.hpp"

using namespace edge_scheduler;

void test_hardware_crc32_bit_flip_detection() {
    std::cout << "[SafetyIsland Test 1] Testing Hardware CRC32 Memory Bit-Flip (SEU) Detection... " << std::flush;

    SafetyIslandGovernor governor(2);

    // 1. Create a 1KB buffer (simulating /dev/shm camera frame header)
    std::vector<uint8_t> buffer(1024, 0xAB);
    uint32_t expected_crc = SafetyIslandGovernor::compute_crc32(buffer.data(), buffer.size());

    // Valid buffer passes
    assert(governor.verify_frame_integrity(buffer.data(), buffer.size(), expected_crc) == true);
    assert(governor.get_state() == AsilSafetyState::NORMAL);

    // 2. Simulate Single-Event Upset (SEU): flip exactly 1 bit
    buffer[512] ^= 0x01; // Bit flip!

    // Verify governor detects hardware memory corruption
    bool passed = governor.verify_frame_integrity(buffer.data(), buffer.size(), expected_crc);
    assert(passed == false);
    assert(governor.bit_flips() == 1);
    assert(governor.get_state() == AsilSafetyState::DEGRADED_FAIL_OPERATIONAL);

    std::cout << "PASSED. (Single-Bit SEU Caught & Contained)\n";
}

void test_dual_core_lockstep_validation() {
    std::cout << "[SafetyIsland Test 2] Testing Dual-Core Lockstep Perception Validation... " << std::flush;

    SafetyIslandGovernor governor(2);

    // 1. Primary & Secondary agree
    std::vector<PerceptionBoundingBox> primary = {
        {0.2f, 0.3f, 0.5f, 0.7f, 0.95f, 0}, // Pedestrian
        {0.6f, 0.1f, 0.9f, 0.4f, 0.92f, 1}  // Vehicle
    };
    std::vector<PerceptionBoundingBox> secondary = primary; // Identical

    assert(governor.validate_dual_lockstep(primary, secondary) == true);
    assert(governor.get_state() == AsilSafetyState::NORMAL);

    // 2. Anomaly: Secondary engine mutates bounding box coordinates
    secondary[0].x1 = 0.45f; // Spatial divergence > 5% tolerance

    auto start_time = std::chrono::high_resolution_clock::now();
    bool passed = governor.validate_dual_lockstep(primary, secondary);
    assert(passed == false);
    assert(governor.get_state() == AsilSafetyState::DEGRADED_FAIL_OPERATIONAL);

    // 3. Second consecutive anomaly -> triggers EMERGENCY_SAFE_STOP
    governor.validate_dual_lockstep(primary, secondary);
    auto end_time = std::chrono::high_resolution_clock::now();

    double reaction_us = std::chrono::duration<double, std::micro>(end_time - start_time).count();

    assert(governor.get_state() == AsilSafetyState::EMERGENCY_SAFE_STOP);
    assert(governor.divergences() == 2);
    assert(reaction_us < 5000.0); // Less than 5.0ms reaction time!

    std::cout << "PASSED. (Fail-Operational Safe Stop Triggered in " << (reaction_us / 1000.0) << " ms)\n";
}

int main() {
    std::cout << "==================================================\n";
    std::cout << "🛡️ ISO 26262 ASIL-D Functional Safety Test Suite \n";
    std::cout << "==================================================\n";

    test_hardware_crc32_bit_flip_detection();
    test_dual_core_lockstep_validation();

    std::cout << "==================================================\n";
    std::cout << "🎉 ALL ASIL-D SAFETY ISLAND TESTS PASSED!\n";
    std::cout << "==================================================\n";
    return 0;
}
