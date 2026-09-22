#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <iostream>

#ifdef HAS_CUDA
#include <cuda_runtime.h>
#endif

namespace edge_scheduler {

/**
 * PinnedMemoryPool: Zero-Copy Page-Locked (Pinned) Memory Allocator.
 * 
 * On NVIDIA Jetson's Unified Memory Architecture (UMA), regular host malloc() requires
 * an expensive cudaMemcpy Host-to-Device transfer across the SoC bus.
 * By using cudaHostAlloc with cudaHostAllocMapped, memory is page-locked and directly 
 * accessible by both CPU and GPU without bus copies, saving 3-8ms per 1080p frame.
 */
class PinnedMemoryPool {
public:
    PinnedMemoryPool(size_t buffer_size_bytes, size_t pool_capacity = 4)
        : buffer_size_(buffer_size_bytes), pool_capacity_(pool_capacity) {
        
        for (size_t i = 0; i < pool_capacity_; ++i) {
            void* host_ptr = nullptr;
#ifdef HAS_CUDA
            cudaError_t err = cudaHostAlloc(&host_ptr, buffer_size_, cudaHostAllocMapped);
            if (err != cudaSuccess) {
                std::cerr << "[PinnedMemoryPool] Failed to allocate pinned memory: " 
                          << cudaGetErrorString(err) << std::endl;
                host_ptr = malloc(buffer_size_);
            }
#else
            host_ptr = malloc(buffer_size_);
#endif
            available_buffers_.push_back(host_ptr);
        }
    }

    ~PinnedMemoryPool() {
        std::lock_guard<std::mutex> lock(pool_mutex_);
        for (void* ptr : available_buffers_) {
#ifdef HAS_CUDA
            cudaFreeHost(ptr);
#else
            free(ptr);
#endif
        }
        available_buffers_.clear();
    }

    void* acquire() {
        std::lock_guard<std::mutex> lock(pool_mutex_);
        if (available_buffers_.empty()) {
            // Fallback emergency allocation if pool is drained
            void* fallback_ptr = nullptr;
#ifdef HAS_CUDA
            cudaHostAlloc(&fallback_ptr, buffer_size_, cudaHostAllocMapped);
#else
            fallback_ptr = malloc(buffer_size_);
#endif
            return fallback_ptr;
        }
        void* ptr = available_buffers_.back();
        available_buffers_.pop_back();
        return ptr;
    }

    void release(void* ptr) {
        std::lock_guard<std::mutex> lock(pool_mutex_);
        if (available_buffers_.size() < pool_capacity_) {
            available_buffers_.push_back(ptr);
        } else {
#ifdef HAS_CUDA
            cudaFreeHost(ptr);
#else
            free(ptr);
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
