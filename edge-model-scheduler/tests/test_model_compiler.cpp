#include <iostream>
#include <cassert>
#include <vector>
#include <fstream>

#include "ModelCompiler.hpp"

using namespace edge_scheduler;

void test_dla_layer_analysis() {
    std::cout << "[Test 1] Testing DLA layer compatibility analysis... ";

    ModelCompiler compiler;
    std::vector<std::string> unsupported;

    // Test 1: Pure CNN model (YOLO backbone)
    std::vector<std::string> cnn_layers = {
        "Conv2d", "BatchNorm", "Relu", "Conv2d", "BatchNorm", "Relu", "MaxPool", "Conv2d"
    };
    float score = compiler.analyze_dla_compatibility(cnn_layers, unsupported);
    assert(score == 100.0f);
    assert(unsupported.empty());

    // Test 2: Hybrid Vision Transformer model (BEVFormer / SegFormer)
    std::vector<std::string> transformer_layers = {
        "Conv2d", "BatchNorm", "Relu", "MultiHeadAttention", "DeformableConv", "LayerNorm", "Linear"
    };
    score = compiler.analyze_dla_compatibility(transformer_layers, unsupported);
    assert(score < 100.0f);
    assert(!unsupported.empty());
    assert(unsupported.size() == 3); // MultiHeadAttention, DeformableConv, LayerNorm

    std::cout << "PASSED. (CNN score: 100%, Transformer score: " << score << "%)\n";
}

void test_manifest_and_engine_generation() {
    std::cout << "[Test 2] Testing Dual Engine & Manifest generation... ";

    ModelCompilationConfig cfg;
    cfg.model_name = "yolov8_obstacle";
    cfg.output_dir = "/tmp";
    cfg.target_platform = "orin_agx";
    cfg.enable_dla = true;
    cfg.target_slo_ms = 35.0f;

    ModelCompiler compiler(cfg);

    std::vector<std::string> layers = {"Conv2d", "BatchNorm", "Relu", "Conv2d", "MaxPool"};
    std::vector<TensorSpec> inputs = {{"images", {1, 3, 640, 640}, "float32"}};
    std::vector<TensorSpec> outputs = {{"predictions", {1, 84, 8400}, "float32"}};

    auto manifest = compiler.compile(layers, inputs, outputs);

    assert(manifest.model_name == "yolov8_obstacle");
    assert(manifest.recommended_device == "DLA0");
    assert(manifest.expected_fp16_latency_ms > 0.0f);
    assert(manifest.expected_int8_latency_ms < manifest.expected_fp16_latency_ms);
    assert(manifest.inputs.size() == 1);
    assert(manifest.inputs[0].byte_size() == 1 * 3 * 640 * 640 * 4); // 4.9MB float32

    // Verify written manifest file
    std::ifstream file("/tmp/model_manifest.json");
    assert(file.is_open());
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    assert(content.find("\"model_name\": \"yolov8_obstacle\"") != std::string::npos);
    assert(content.find("\"recommended_device\": \"DLA0\"") != std::string::npos);

    std::cout << "PASSED. (Expected FP16: " << manifest.expected_fp16_latency_ms
              << "ms, INT8: " << manifest.expected_int8_latency_ms << "ms)\n";
}

void test_trtexec_command_synthesis() {
    std::cout << "[Test 3] Testing trtexec command synthesis... ";

    ModelCompilationConfig cfg;
    cfg.model_path = "/models/yolov8.onnx";
    cfg.model_name = "yolov8";
    cfg.output_dir = "/models/build";
    cfg.target_platform = "orin_agx";
    cfg.enable_dla = true;
    cfg.workspace_mb = 2048;

    ModelCompiler compiler(cfg);

    std::string fp16_cmd = compiler.generate_trtexec_command(false);
    assert(fp16_cmd.find("--onnx=/models/yolov8.onnx") != std::string::npos);
    assert(fp16_cmd.find("--fp16") != std::string::npos);
    assert(fp16_cmd.find("--useDLACore=0") != std::string::npos);

    std::string int8_cmd = compiler.generate_trtexec_command(true);
    assert(int8_cmd.find("--int8") != std::string::npos);

    std::cout << "PASSED.\n";
}

int main() {
    std::cout << "==================================================" << std::endl;
    std::cout << "🧪 Running ModelCompiler Unit Tests" << std::endl;
    std::cout << "==================================================" << std::endl;

    test_dla_layer_analysis();
    test_manifest_and_engine_generation();
    test_trtexec_command_synthesis();

    std::cout << "==================================================" << std::endl;
    std::cout << "🎉 ALL MODEL COMPILER TESTS PASSED!" << std::endl;
    std::cout << "==================================================" << std::endl;
    return 0;
}
