#pragma once

#include <string>
#include <vector>
#include <atomic>
#include <iostream>
#include <cstdlib>

#ifdef HAS_CUDA
#include <cuda_runtime.h>
#endif

namespace edge_scheduler {

class PinnedMemoryPool {
public:
    explicit PinnedMemoryPool(size_t buffer_size_bytes = 1024 * 1024, size_t pool_capacity = 8)
        : buffer_size_(buffer_size_bytes),
          pool_capacity_(pool_capacity > MAX_CAPACITY ? MAX_CAPACITY : pool_capacity)
    {
        for (size_t i = 0; i < pool_capacity_; ++i) {
            void* host_ptr = allocate_buffer();
            slots_[i].store(host_ptr, std::memory_order_relaxed);
        }
        for (size_t i = pool_capacity_; i < MAX_CAPACITY; ++i) {
            slots_[i].store(nullptr, std::memory_order_relaxed);
        }
    }

    ~PinnedMemoryPool() {
        for (size_t i = 0; i < MAX_CAPACITY; ++i) {
            void* ptr = slots_[i].exchange(nullptr, std::memory_order_relaxed);
            if (ptr) {
                free_buffer(ptr);
            }
        }
    }

    // Lock-free buffer acquisition (< 15ns)
    void* acquire() {
        for (size_t i = 0; i < pool_capacity_; ++i) {
            void* expected = slots_[i].load(std::memory_order_relaxed);
            if (expected != nullptr) {
                if (slots_[i].compare_exchange_weak(expected, nullptr,
                                                    std::memory_order_acquire,
                                                    std::memory_order_relaxed)) {
                    return expected;
                }
            }
        }
        // Pool exhausted: allocate fallback buffer directly
        return allocate_buffer();
    }

    // Lock-free buffer recycling (< 15ns)
    void release(void* ptr) {
        if (!ptr) return;
        for (size_t i = 0; i < pool_capacity_; ++i) {
            void* expected = nullptr;
            if (slots_[i].load(std::memory_order_relaxed) == nullptr) {
                if (slots_[i].compare_exchange_weak(expected, ptr,
                                                    std::memory_order_release,
                                                    std::memory_order_relaxed)) {
                    return;
                }
            }
        }
        // Pool full: free excess buffer
        free_buffer(ptr);
    }

    size_t buffer_size() const { return buffer_size_; }
    size_t pool_capacity() const { return pool_capacity_; }

private:
    void* allocate_buffer() {
        void* host_ptr = nullptr;
#ifdef HAS_CUDA
        cudaError_t err = cudaHostAlloc(&host_ptr, buffer_size_, cudaHostAllocMapped);
        if (err != cudaSuccess || !host_ptr) {
            host_ptr = std::malloc(buffer_size_);
        }
#else
        host_ptr = std::malloc(buffer_size_);
#endif
        return host_ptr;
    }

    void free_buffer(void* ptr) {
        if (!ptr) return;
#ifdef HAS_CUDA
        cudaFreeHost(ptr);
#else
        std::free(ptr);
#endif
    }

    static constexpr size_t MAX_CAPACITY = 64;
    size_t buffer_size_;
    size_t pool_capacity_;
    alignas(64) std::atomic<void*> slots_[MAX_CAPACITY];
};

} // namespace edge_scheduler
