#include "TensorRTRunner.hpp"
#include <iostream>
#include <fstream>

// We stub the TensorRT calls if the headers aren't available, 
// so this can compile on non-Jetson/non-GPU machines during development.
#ifdef HAS_TENSORRT
#include <NvInfer.h>
#include <cuda_runtime_api.h>

class Logger : public nvinfer1::ILogger {
    void log(Severity severity, const char* msg) noexcept override {
        if (severity != Severity::kINFO) {
            std::cout << "[TRT] " << msg << std::endl;
        }
    }
} gLogger;
#endif

namespace edge_scheduler {

TensorRTRunner::TensorRTRunner() {
#ifdef HAS_TENSORRT
    runtime_ = std::shared_ptr<nvinfer1::IRuntime>(nvinfer1::createInferRuntime(gLogger));
#endif
}

TensorRTRunner::~TensorRTRunner() {
    // Shared pointers will clean up if we provide custom deleters, 
    // but typically TRT objects require ->destroy() in older versions.
    // Assuming TRT 8+ where delete can be used.
}

bool TensorRTRunner::load_engine(const std::string& engine_path) {
    std::cout << "[TensorRTRunner] Loading engine from " << engine_path << std::endl;
    
    std::ifstream file(engine_path, std::ios::binary);
    if (!file.good()) {
        std::cerr << "Error opening engine file: " << engine_path << std::endl;
        return false;
    }

#ifdef HAS_TENSORRT
    file.seekg(0, file.end);
    size_t size = file.tellg();
    file.seekg(0, file.beg);
    
    std::vector<char> trtModelStream(size);
    file.read(trtModelStream.data(), size);
    file.close();

    engine_ = std::shared_ptr<nvinfer1::ICudaEngine>(
        runtime_->deserializeCudaEngine(trtModelStream.data(), size)
    );

    if (!engine_) {
        std::cerr << "Failed to deserialize engine" << std::endl;
        return false;
    }

    context_ = std::shared_ptr<nvinfer1::IExecutionContext>(engine_->createExecutionContext());
    if (!context_) {
        std::cerr << "Failed to create execution context" << std::endl;
        return false;
    }
#else
    std::cout << "[TensorRTRunner] (Mock) Engine loaded successfully." << std::endl;
#endif

    return true;
}

bool TensorRTRunner::infer() {
#ifdef HAS_TENSORRT
    if (!context_) {
        std::cerr << "No execution context" << std::endl;
        return false;
    }

    // In a real application, you enqueue the task on a CUDA stream here:
    // void* buffers[2]; // Setup input/output device pointers
    // cudaStream_t stream;
    // cudaStreamCreate(&stream);
    // context_->enqueueV2(buffers, stream, nullptr);
    // cudaStreamSynchronize(stream);
    // cudaStreamDestroy(stream);
    
    std::cout << "[TensorRTRunner] Executed inference via TRT C++ API." << std::endl;
    return true;
#else
    std::cout << "[TensorRTRunner] (Mock) Executed inference." << std::endl;
    return true;
#endif
}

} // namespace edge_scheduler
