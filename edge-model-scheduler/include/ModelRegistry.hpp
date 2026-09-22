#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>

namespace edge_scheduler {

struct ModelConfig {
    std::string model_id;
    std::string engine_path;
    int priority_level;        // 1 = High, 2 = Medium, 3 = Low
    float latency_slo_ms;      // Maximum permissible latency deadline
    size_t input_buffer_bytes; // Buffer size for zero-copy memory allocation
    bool allow_quantization_fallback; // Whether INT8 fallback is allowed under heat
};

class ModelRegistry {
public:
    ModelRegistry() = default;

    bool register_model(const ModelConfig& config) {
        if (config.model_id.empty() || config.latency_slo_ms <= 0.0f) {
            return false;
        }
        models_[config.model_id] = config;
        return true;
    }

    const ModelConfig* get_model(const std::string& model_id) const {
        auto it = models_.find(model_id);
        if (it != models_.end()) {
            return &(it->second);
        }
        return nullptr;
    }

    bool has_model(const std::string& model_id) const {
        return models_.find(model_id) != models_.end();
    }

    size_t count() const {
        return models_.size();
    }

    std::vector<std::string> get_all_model_ids() const {
        std::vector<std::string> ids;
        ids.reserve(models_.size());
        for (const auto& pair : models_) {
            ids.push_back(pair.first);
        }
        return ids;
    }

    // Load simple configuration file format: model_id,engine_path,priority,slo_ms,buffer_bytes,fallback
    bool load_from_csv(const std::string& csv_path) {
        std::ifstream file(csv_path);
        if (!file.is_open()) {
            std::cerr << "[ModelRegistry] Failed to open config file: " << csv_path << std::endl;
            return false;
        }

        std::string line;
        // Skip header if present
        if (std::getline(file, line)) {
            if (line.find("model_id") == std::string::npos) {
                // Not a header line, process it
                parse_and_add_line(line);
            }
        }

        while (std::getline(file, line)) {
            if (!line.empty() && line[0] != '#') {
                parse_and_add_line(line);
            }
        }
        return true;
    }

private:
    void parse_and_add_line(const std::string& line) {
        std::stringstream ss(line);
        std::string item;
        std::vector<std::string> tokens;
        while (std::getline(ss, item, ',')) {
            tokens.push_back(item);
        }

        if (tokens.size() >= 6) {
            ModelConfig cfg;
            cfg.model_id = tokens[0];
            cfg.engine_path = tokens[1];
            cfg.priority_level = std::stoi(tokens[2]);
            cfg.latency_slo_ms = std::stof(tokens[3]);
            cfg.input_buffer_bytes = std::stoul(tokens[4]);
            cfg.allow_quantization_fallback = (tokens[5] == "1" || tokens[5] == "true");
            register_model(cfg);
        }
    }

    std::unordered_map<std::string, ModelConfig> models_;
};

} // namespace edge_scheduler
