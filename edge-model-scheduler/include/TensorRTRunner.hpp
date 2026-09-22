#pragma once

#include <string>
#include <vector>
#include <memory>
// Forward declarations for TensorRT to avoid heavy includes in headers if possible
namespace nvinfer1 {
    class IRuntime;
    class ICudaEngine;
    class IExecutionContext;
}

namespace edge_scheduler {

class TensorRTRunner {
public:
    TensorRTRunner();
    ~TensorRTRunner();

    // Load a TensorRT engine from a serialized file
    bool load_engine(const std::string& engine_path);

    // Run inference (synchronous)
    // In a real system, you'd pass input/output buffer pointers here
    bool infer();

private:
    std::shared_ptr<nvinfer1::IRuntime> runtime_;
    std::shared_ptr<nvinfer1::ICudaEngine> engine_;
    std::shared_ptr<nvinfer1::IExecutionContext> context_;
};

} // namespace edge_scheduler
