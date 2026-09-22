#include <iostream>
#include <memory>

#include "AdvancedScheduler.hpp"
#include "ResourceMonitor.hpp"
#include "PinnedMemoryPool.hpp"
#include "QuantizationGovernor.hpp"
#include "ChaosEngine.hpp"

using namespace edge_scheduler;

int main() {
    std::cout << "\n╔═══════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  🔥 EdgeModelScheduler v3.0 Chaos Test Runner     ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════════════╝\n" << std::endl;

    auto monitor = std::make_shared<ResourceMonitor>();
    auto pool = std::make_shared<PinnedMemoryPool>(1024 * 64, 8);
    auto governor = std::make_shared<QuantizationGovernor>();

    ChaosEngine chaos(monitor, pool, governor);
    AdvancedScheduler scheduler(monitor, pool);

    auto results = chaos.run_all(scheduler);

    // Exit code: non-zero if any scenario failed
    for (const auto& r : results) {
        if (!r.passed) return 1;
    }

    return 0;
}
