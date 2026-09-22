#pragma once

#include <iostream>
#include <vector>

#ifdef HAS_CUDA
#include <cuda_runtime.h>
#endif

namespace edge_scheduler {

/**
 * DualCudaStreamDispatcher:
 * 
 * Hardware Asynchronous Multi-Stream Isolation.
 * Standard Triton/PyTorch inference blocks the default CUDA stream (Stream 0).
 * By explicitly binding High-Priority tasks to high_priority_stream_ 
 * with cudaStreamCreateWithPriority(-1), NVIDIA hardware schedulers preempt lower 
 * priority kernels on the streaming multiprocessors (SMs).
 */
class DualCudaStreamDispatcher {
public:
    DualCudaStreamDispatcher() {
#ifdef HAS_CUDA
        int leastPriority = 0, greatestPriority = 0;
        cudaDeviceGetStreamPriorityRange(&leastPriority, &greatestPriority);
        
        // greatestPriority has numerical lower value (e.g. -1 or -5)
        cudaStreamCreateWithPriority(&high_priority_stream_, cudaStreamNonBlocking, greatestPriority);
        cudaStreamCreateWithPriority(&low_priority_stream_, cudaStreamNonBlocking, leastPriority);
        std::cout << "[DualCudaStreamDispatcher] Hardware streams initialized (High: " 
                  << greatestPriority << ", Low: " << leastPriority << ")" << std::endl;
#else
        high_priority_stream_ = nullptr;
        low_priority_stream_ = nullptr;
#endif
    }

    ~DualCudaStreamDispatcher() {
#ifdef HAS_CUDA
        if (high_priority_stream_) cudaStreamDestroy(high_priority_stream_);
        if (low_priority_stream_) cudaStreamDestroy(low_priority_stream_);
#endif
    }

    void* get_high_priority_stream() const {
#ifdef HAS_CUDA
        return reinterpret_cast<void*>(high_priority_stream_);
#else
        return nullptr;
#endif
    }

    void* get_low_priority_stream() const {
#ifdef HAS_CUDA
        return reinterpret_cast<void*>(low_priority_stream_);
#else
        return nullptr;
#endif
    }

    void synchronize_all() {
#ifdef HAS_CUDA
        cudaStreamSynchronize(high_priority_stream_);
        cudaStreamSynchronize(low_priority_stream_);
#endif
    }

private:
#ifdef HAS_CUDA
    cudaStream_t high_priority_stream_;
    cudaStream_t low_priority_stream_;
#else
    void* high_priority_stream_;
    void* low_priority_stream_;
#endif
};

} // namespace edge_scheduler
