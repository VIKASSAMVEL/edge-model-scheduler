#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace nvinfer1 {
    class IRuntime;
    class ICudaEngine;
    class IExecutionContext;
}

namespace edge_scheduler {

struct TensorBindingInfo {
    std::string name;
    bool is_input{true};
    size_t size_bytes{0};
    std::vector<int64_t> dims;
};

class TensorRTRunner {
public:
    TensorRTRunner();
    ~TensorRTRunner();

    // Load a TensorRT engine from serialized .engine / .plan binary file
    bool load_engine(const std::string& engine_path);

    // Run inference synchronously (using internal device buffers or mock)
    bool infer();

    // Full-featured hardware inference: pass explicit device pointers and CUDA stream
    bool infer(void** device_buffers, void* cuda_stream = nullptr);

    // Asynchronous dispatch on a specific CUDA stream (e.g. from DualCudaStreamDispatcher)
    bool infer_async(void** device_buffers, void* cuda_stream);

    // Inspect loaded engine metadata
    bool is_loaded() const { return is_loaded_; }
    size_t num_bindings() const { return bindings_.size(); }
    const std::vector<TensorBindingInfo>& bindings() const { return bindings_; }
    size_t input_size_bytes() const { return input_size_bytes_; }
    size_t output_size_bytes() const { return output_size_bytes_; }
    const std::string& engine_path() const { return engine_path_; }

    // Allocate internal GPU device scratch memory if caller does not manage IPC memory
    bool allocate_internal_buffers();
    void** get_internal_bindings() { return device_ptrs_.data(); }

private:
    void cleanup_internal_buffers();

    std::shared_ptr<nvinfer1::IRuntime> runtime_;
    std::shared_ptr<nvinfer1::ICudaEngine> engine_;
    std::shared_ptr<nvinfer1::IExecutionContext> context_;

    bool is_loaded_{false};
    std::string engine_path_;
    std::vector<TensorBindingInfo> bindings_;
    size_t input_size_bytes_{0};
    size_t output_size_bytes_{0};

    std::vector<void*> device_ptrs_;
    bool internal_allocated_{false};
};

} // namespace edge_scheduler

