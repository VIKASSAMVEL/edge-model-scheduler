#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <iostream>
#include <cstdlib>

#ifdef HAS_CUDA
#include <cuda_runtime.h>
#endif

namespace edge_scheduler {

class PinnedMemoryPool {
public:
    PinnedMemoryPool(size_t buffer_size_bytes = 1024 * 1024, size_t pool_capacity = 4)
        : buffer_size_(buffer_size_bytes), pool_capacity_(pool_capacity) {
        
        for (size_t i = 0; i < pool_capacity_; ++i) {
            void* host_ptr = nullptr;
#ifdef HAS_CUDA
            cudaError_t err = cudaHostAlloc(&host_ptr, buffer_size_, cudaHostAllocMapped);
            if (err != cudaSuccess || !host_ptr) {
                host_ptr = std::malloc(buffer_size_);
            }
#else
            host_ptr = std::malloc(buffer_size_);
#endif
            if (host_ptr) {
                available_buffers_.push_back(host_ptr);
            }
        }
    }

    ~PinnedMemoryPool() {
        std::lock_guard<std::mutex> lock(pool_mutex_);
        for (void* ptr : available_buffers_) {
            if (!ptr) continue;
#ifdef HAS_CUDA
            cudaFreeHost(ptr);
#else
            std::free(ptr);
#endif
        }
        available_buffers_.clear();
    }

    void* acquire() {
        std::lock_guard<std::mutex> lock(pool_mutex_);
        if (available_buffers_.empty()) {
            void* fallback_ptr = nullptr;
#ifdef HAS_CUDA
            cudaError_t err = cudaHostAlloc(&fallback_ptr, buffer_size_, cudaHostAllocMapped);
            if (err != cudaSuccess || !fallback_ptr) {
                fallback_ptr = std::malloc(buffer_size_);
            }
#else
            fallback_ptr = std::malloc(buffer_size_);
#endif
            return fallback_ptr;
        }
        void* ptr = available_buffers_.back();
        available_buffers_.pop_back();
        return ptr;
    }

    void release(void* ptr) {
        if (!ptr) return;
        std::lock_guard<std::mutex> lock(pool_mutex_);
        if (available_buffers_.size() < pool_capacity_) {
            available_buffers_.push_back(ptr);
        } else {
#ifdef HAS_CUDA
            cudaFreeHost(ptr);
#else
            std::free(ptr);
#endif
        }
    }

    size_t buffer_size() const { return buffer_size_; }

private:
    size_t buffer_size_;
    size_t pool_capacity_;
    std::vector<void*> available_buffers_;
    std::mutex pool_mutex_;
};

} // namespace edge_scheduler
