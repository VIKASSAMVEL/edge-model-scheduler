#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <stdexcept>
#include <iostream>

#ifdef HAS_CUDA
#include <cuda_runtime.h>
#endif

namespace edge_scheduler {

/**
 * CudaIpcAllocator: Cross-Process GPU Memory Transport
 *
 * On NVIDIA Jetson (UMA architecture), CPU and GPU share the same physical
 * DRAM. However, CUDA virtual addresses are still process-private. To pass
 * GPU tensors between an external camera process and the scheduler daemon
 * without D2H→H2D round-trips, we use CUDA IPC:
 *
 *   Producer Process:
 *     1. cudaMalloc(&dev_ptr, size)
 *     2. cudaIpcGetMemHandle(&handle, dev_ptr)
 *     3. Send handle (64 bytes) over SHM ring buffer
 *
 *   Consumer (Daemon):
 *     1. Receive handle from ring buffer
 *     2. cudaIpcOpenMemHandle(&dev_ptr, handle, cudaIpcMemLazyEnablePeerAccess)
 *     3. Execute TensorRT inference directly on dev_ptr
 *     4. cudaIpcCloseMemHandle(dev_ptr) when done
 *
 * This eliminates ALL memcpy for GPU-resident data (camera → inference).
 */

struct IpcGpuHandle {
#if defined(HAS_CUDA) && defined(__linux__)
    cudaIpcMemHandle_t handle;
#else
    uint8_t handle[64]; // Placeholder for cross-platform compilation
#endif
    size_t size_bytes{0};
    uint32_t device_ordinal{0};
    bool valid{false};
};

class CudaIpcAllocator {
public:
    CudaIpcAllocator(uint32_t device_id = 0) : device_id_(device_id) {
#if defined(HAS_CUDA) && defined(__linux__)
        cudaError_t err = cudaSetDevice(device_id);
        if (err != cudaSuccess) {
            throw std::runtime_error(
                std::string("cudaSetDevice failed: ") + cudaGetErrorString(err));
        }
#endif
    }

    ~CudaIpcAllocator() = default;

    /**
     * Allocate device memory and export an IPC handle.
     * The producer calls this and sends the handle to the consumer via SHM.
     */
    IpcGpuHandle allocate_and_export(size_t size_bytes) {
        IpcGpuHandle result;
        result.size_bytes = size_bytes;
        result.device_ordinal = device_id_;

#if defined(HAS_CUDA) && defined(__linux__)
        void* dev_ptr = nullptr;
        cudaError_t err = cudaMalloc(&dev_ptr, size_bytes);
        if (err != cudaSuccess || !dev_ptr) {
            throw std::runtime_error("cudaMalloc failed for IPC export");
        }

        err = cudaIpcGetMemHandle(&result.handle, dev_ptr);
        if (err != cudaSuccess) {
            cudaFree(dev_ptr);
            throw std::runtime_error("cudaIpcGetMemHandle failed");
        }

        result.valid = true;
        owned_ptrs_.push_back(dev_ptr);
#else
        // Non-CUDA fallback: simulate with heap allocation
        void* ptr = std::malloc(size_bytes);
        if (!ptr) throw std::runtime_error("malloc fallback failed");
        std::memcpy(result.handle, &ptr, sizeof(void*));
        result.valid = true;
        owned_ptrs_.push_back(ptr);
#endif
        return result;
    }

    /**
     * Import a foreign GPU handle received from another process.
     * Returns a device pointer usable in this process's CUDA context.
     */
    static void* import_handle(const IpcGpuHandle& ipc_handle) {
        if (!ipc_handle.valid) {
            throw std::invalid_argument("Invalid IPC handle");
        }

#if defined(HAS_CUDA) && defined(__linux__)
        void* dev_ptr = nullptr;
        cudaError_t err = cudaIpcOpenMemHandle(
            &dev_ptr, ipc_handle.handle, cudaIpcMemLazyEnablePeerAccess);
        if (err != cudaSuccess) {
            throw std::runtime_error(
                std::string("cudaIpcOpenMemHandle failed: ") + cudaGetErrorString(err));
        }
        return dev_ptr;
#else
        void* ptr = nullptr;
        std::memcpy(&ptr, ipc_handle.handle, sizeof(void*));
        return ptr;
#endif
    }

    /**
     * Release an imported handle (consumer side).
     */
    static void release_imported(void* dev_ptr) {
        if (!dev_ptr) return;
#if defined(HAS_CUDA) && defined(__linux__)
        cudaIpcCloseMemHandle(dev_ptr);
#endif
        // Non-CUDA: don't free — the producer owns the allocation
    }

    /**
     * Free all owned allocations (producer side cleanup).
     */
    void free_all() {
        for (void* ptr : owned_ptrs_) {
#if defined(HAS_CUDA) && defined(__linux__)
            cudaFree(ptr);
#else
            std::free(ptr);
#endif
        }
        owned_ptrs_.clear();
    }

    size_t owned_count() const { return owned_ptrs_.size(); }

private:
    uint32_t device_id_;
    std::vector<void*> owned_ptrs_;
};

} // namespace edge_scheduler
