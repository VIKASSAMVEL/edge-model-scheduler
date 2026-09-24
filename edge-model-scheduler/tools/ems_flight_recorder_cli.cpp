#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>

#include "FlightRecorder.hpp"

using namespace edge_scheduler;

void print_usage(const char* prog) {
    std::cout << "EdgeModelScheduler v4.0 Flight Recorder Forensics CLI\n"
              << "Usage: " << prog << " <incident_bag.emsbag> [--export-csv <out.csv>] [--summary]\n"
              << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string bag_path = argv[1];
    FlightRecorderHeader header;
    std::vector<FlightTelemetryFrame> frames;

    if (!FlightRecorder::parse_emsbag(bag_path, header, frames)) {
        std::cerr << "[ERROR] Failed to parse .emsbag file: " << bag_path << " (Corrupt header or invalid magic)" << std::endl;
        return 2;
    }

    std::cout << "===============================================================\n"
              << "🚁 EDGEMODELSCHEDULER ASIL-D BLACK BOX FORENSIC REPLAY REPORT\n"
              << "===============================================================\n"
              << "  Format Version   : " << header.format_version << "\n"
              << "  Vehicle ID       : " << header.vehicle_id << "\n"
              << "  Trigger Reason   : " << header.trigger_reason << "\n"
              << "  Total Frames     : " << header.total_frames << "\n"
              << "  Recording Window : " << (header.record_end_ns - header.record_start_ns) / 1e6 << " ms\n"
              << "===============================================================\n";

    if (frames.empty()) {
        std::cout << "[INFO] Bag contains zero telemetry frames." << std::endl;
        return 0;
    }

    // Compute Forensics Metrics
    float max_temp = 0.0f;
    float max_p99 = 0.0f;
    float min_slack = 9999.0f;
    uint32_t seu_events = 0;
    uint32_t total_shed = frames.back().stale_shed_total;

    for (const auto& f : frames) {
        if (f.gpu_temp_c > max_temp) max_temp = f.gpu_temp_c;
        if (f.p99_latency_ms > max_p99) max_p99 = f.p99_latency_ms;
        if (f.deadline_slack_ms < min_slack) min_slack = f.deadline_slack_ms;
        if (f.asil_state > 0) seu_events++;
    }

    std::cout << "📈 FORENSIC TELEMETRY SUMMARY:\n"
              << "  Peak GPU Temperature   : " << std::fixed << std::setprecision(1) << max_temp << " °C\n"
              << "  Peak P99 Latency       : " << max_p99 << " ms\n"
              << "  Worst-Case Slack (WCET): " << min_slack << " ms\n"
              << "  Stale Frames Shed      : " << total_shed << " sweeps\n"
              << "  Safety Island Events   : " << seu_events << " anomalies\n"
              << "===============================================================\n";

    // Check for CSV export argument
    for (int i = 2; i < argc; ++i) {
        if (std::string(argv[i]) == "--export-csv" && i + 1 < argc) {
            std::string csv_path = argv[i + 1];
            std::ofstream csv(csv_path);
            if (csv.is_open()) {
                csv << "timestamp_ns,sequence_id,gpu_temp_c,gpu_util_pct,p99_latency_ms,slack_ms,battery_soc,asil_state\n";
                for (const auto& f : frames) {
                    csv << f.timestamp_ns << "," << f.sequence_id << "," << f.gpu_temp_c << ","
                        << f.gpu_util_pct << "," << f.p99_latency_ms << "," << f.deadline_slack_ms << ","
                        << f.battery_soc_pct << "," << f.asil_state << "\n";
                }
                csv.close();
                std::cout << "✅ Telemetry successfully exported to: " << csv_path << std::endl;
            }
        }
    }

    return 0;
}
