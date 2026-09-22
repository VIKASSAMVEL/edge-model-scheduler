#pragma once

#include <atomic>
#include <cstdint>
#include <cstring>
#include <string>
#include <stdexcept>

#ifdef __linux__
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include <thread>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

// Portable micro-architectural CPU pause/relax helper for spin-loops
inline void cpu_relax() noexcept {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    #if defined(_MSC_VER)
        _mm_pause();
    #else
        __builtin_ia32_pause();
    #endif
#elif defined(__aarch64__) || defined(__arm__)
    asm volatile("yield" ::: "memory");
#else
    std::this_thread::yield();
#endif
}

namespace edge_scheduler {

/**
 * ShmRingBuffer: Lock-Free SPSC Ring Buffer over POSIX Shared Memory
 *
 * Architecture:
 *   Memory Layout in /dev/shm:
 *   ┌──────────────────────────────────────────────────────────────────┐
 *   │  ControlBlock (cache-line aligned)                               │
 *   │  [write_seq: u64] [pad: 56B] [read_seq: u64] [pad: 56B]        │
 *   │  [slot_count: u32] [slot_size: u32] [version: u32] [flags: u32] │
 *   ├──────────────────────────────────────────────────────────────────┤
 *   │  Slot Headers [N × SlotHeader]                                   │
 *   │  [sequence: u64] [payload_size: u32] [timestamp_ns: u64]        │
 *   ├──────────────────────────────────────────────────────────────────┤
 *   │  Payload Arena [N × slot_payload_size bytes]                     │
 *   └──────────────────────────────────────────────────────────────────┘
 *
 * Design rationale:
 * - write_seq and read_seq are on separate cache lines to prevent false sharing.
 * - Sequence numbers use acquire/release memory ordering (no memory barriers needed).
 * - Power-of-two slot count enables bitwise masking instead of modulo.
 * - Zero mutex, zero syscall in the publish/consume hot path.
 */

static constexpr size_t CACHE_LINE_SIZE = 64;
static constexpr uint32_t SHM_RING_VERSION = 0x00030001; // v3.0.1

struct alignas(CACHE_LINE_SIZE) ShmControlBlock {
    // Producer-owned: cache-line isolated
    alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> write_seq{0};

    // Consumer-owned: cache-line isolated
    alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> read_seq{0};

    // Immutable after init
    alignas(CACHE_LINE_SIZE) uint32_t slot_count{0};
    uint32_t slot_payload_size{0};
    uint32_t version{SHM_RING_VERSION};
    std::atomic<uint32_t> flags{0}; // bit 0: initialized
};

struct SlotHeader {
    std::atomic<uint64_t> sequence{0};
    uint32_t payload_size{0};
    uint32_t reserved{0};
    uint64_t timestamp_ns{0};
};

enum class ShmRole { PRODUCER, CONSUMER };

class ShmRingBuffer {
public:
    /**
     * @param name       POSIX SHM name (e.g. "/edge_scheduler_ipc")
     * @param slot_count Must be power of two (e.g. 16, 32)
     * @param slot_size  Max payload per slot in bytes (e.g. 1920*1080*3 for RGB)
     * @param role       PRODUCER creates & initializes; CONSUMER opens existing
     */
    ShmRingBuffer(const std::string& name, uint32_t slot_count,
                  uint32_t slot_size, ShmRole role)
        : shm_name_(name), role_(role) {

        if ((slot_count & (slot_count - 1)) != 0) {
            throw std::invalid_argument("slot_count must be power of two");
        }
        slot_mask_ = slot_count - 1;

#ifdef __linux__
        size_t total = compute_total_size(slot_count, slot_size);

        if (role == ShmRole::PRODUCER) {
            shm_unlink(name.c_str()); // clean stale
            fd_ = shm_open(name.c_str(), O_CREAT | O_RDWR, 0660);
        } else {
            fd_ = shm_open(name.c_str(), O_RDWR, 0660);
        }

        if (fd_ < 0) {
            throw std::runtime_error("shm_open failed: " + name);
        }

        if (role == ShmRole::PRODUCER) {
            if (ftruncate(fd_, static_cast<off_t>(total)) != 0) {
                close(fd_);
                throw std::runtime_error("ftruncate failed");
            }
        }

        base_ = static_cast<uint8_t*>(
            mmap(nullptr, total, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0));
        if (base_ == MAP_FAILED) {
            close(fd_);
            throw std::runtime_error("mmap failed");
        }

        mapped_size_ = total;
        ctrl_ = reinterpret_cast<ShmControlBlock*>(base_);

        if (role == ShmRole::PRODUCER) {
            new (ctrl_) ShmControlBlock();
            ctrl_->slot_count = slot_count;
            ctrl_->slot_payload_size = slot_size;
            ctrl_->version = SHM_RING_VERSION;

            // Initialize all slot headers
            for (uint32_t i = 0; i < slot_count; ++i) {
                auto* hdr = get_slot_header(i);
                new (hdr) SlotHeader();
                hdr->sequence.store(i, std::memory_order_relaxed);
            }

            ctrl_->flags.store(1, std::memory_order_release); // mark initialized
        } else {
            // Spin until producer has initialized
            while (ctrl_->flags.load(std::memory_order_acquire) == 0) {
                cpu_relax();
            }
            if (ctrl_->version != SHM_RING_VERSION) {
                throw std::runtime_error("SHM version mismatch");
            }
        }
#else
        // Non-Linux: in-process fallback for development/testing
        slot_count_ = slot_count;
        slot_payload_size_ = slot_size;
        size_t total = compute_total_size(slot_count, slot_size);
        heap_buf_.resize(total, 0);
        base_ = heap_buf_.data();
        mapped_size_ = total;
        ctrl_ = reinterpret_cast<ShmControlBlock*>(base_);
        new (ctrl_) ShmControlBlock();
        ctrl_->slot_count = slot_count;
        ctrl_->slot_payload_size = slot_size;
        ctrl_->version = SHM_RING_VERSION;
        for (uint32_t i = 0; i < slot_count; ++i) {
            auto* hdr = get_slot_header(i);
            new (hdr) SlotHeader();
            hdr->sequence.store(i, std::memory_order_relaxed);
        }
        ctrl_->flags.store(1, std::memory_order_release);
#endif
    }

    ~ShmRingBuffer() {
#ifdef __linux__
        if (base_ && base_ != MAP_FAILED) {
            munmap(base_, mapped_size_);
        }
        if (fd_ >= 0) {
            close(fd_);
        }
        if (role_ == ShmRole::PRODUCER) {
            shm_unlink(shm_name_.c_str());
        }
#endif
    }

    // Non-copyable, non-movable
    ShmRingBuffer(const ShmRingBuffer&) = delete;
    ShmRingBuffer& operator=(const ShmRingBuffer&) = delete;

    /**
     * try_publish: Zero-copy publish into the ring buffer.
     * Returns pointer to the slot's payload region for direct writes.
     * Caller fills the buffer, then calls commit_publish().
     *
     * Returns nullptr if ring is full (back-pressure).
     */
    void* try_claim_write_slot(uint64_t& out_seq) {
        out_seq = 0;
        uint64_t seq = ctrl_->write_seq.load(std::memory_order_relaxed);
        for (;;) {
            uint32_t idx = static_cast<uint32_t>(seq) & slot_mask_;
            auto* hdr = get_slot_header(idx);
            uint64_t hdr_seq = hdr->sequence.load(std::memory_order_acquire);

            int64_t diff = static_cast<int64_t>(hdr_seq) - static_cast<int64_t>(seq);
            if (diff == 0) {
                // Slot available — try to claim it
                if (ctrl_->write_seq.compare_exchange_weak(
                        seq, seq + 1, std::memory_order_relaxed)) {
                    out_seq = seq;
                    return get_payload_ptr(idx);
                }
            } else if (diff < 0) {
                // Ring full — consumer hasn't caught up
                return nullptr;
            } else {
                // Another producer advanced — relax and reload
                cpu_relax();
                seq = ctrl_->write_seq.load(std::memory_order_relaxed);
            }
        }
    }

    void commit_publish(uint64_t seq, uint32_t payload_size, uint64_t timestamp_ns = 0) {
        uint32_t idx = static_cast<uint32_t>(seq) & slot_mask_;
        auto* hdr = get_slot_header(idx);
        hdr->payload_size = payload_size;
        hdr->timestamp_ns = timestamp_ns;
        // Release fence: all payload writes visible before sequence advance
        hdr->sequence.store(seq + 1, std::memory_order_release);
    }

    /**
     * try_consume: Zero-copy read from the ring buffer.
     * Returns pointer to payload and fills out metadata.
     * Caller reads the buffer, then calls commit_consume().
     */
    const void* try_consume(uint64_t& out_seq, uint32_t& out_size, uint64_t& out_ts) {
        out_seq = 0;
        out_size = 0;
        out_ts = 0;
        uint64_t seq = ctrl_->read_seq.load(std::memory_order_relaxed);
        for (;;) {
            uint32_t idx = static_cast<uint32_t>(seq) & slot_mask_;
            auto* hdr = get_slot_header(idx);
            uint64_t hdr_seq = hdr->sequence.load(std::memory_order_acquire);

            int64_t diff = static_cast<int64_t>(hdr_seq) - static_cast<int64_t>(seq + 1);
            if (diff == 0) {
                if (ctrl_->read_seq.compare_exchange_weak(
                        seq, seq + 1, std::memory_order_relaxed)) {
                    out_seq = seq;
                    out_size = hdr->payload_size;
                    out_ts = hdr->timestamp_ns;
                    return get_payload_ptr(idx);
                }
            } else if (diff < 0) {
                // Empty — nothing to consume
                return nullptr;
            } else {
                cpu_relax();
                seq = ctrl_->read_seq.load(std::memory_order_relaxed);
            }
        }
    }

    void commit_consume(uint64_t seq) {
        uint32_t idx = static_cast<uint32_t>(seq) & slot_mask_;
        auto* hdr = get_slot_header(idx);
        // Advance slot sequence by full ring size to make it available for producers
        hdr->sequence.store(seq + ctrl_->slot_count, std::memory_order_release);
    }

    // Diagnostics
    uint64_t write_position() const { return ctrl_->write_seq.load(std::memory_order_relaxed); }
    uint64_t read_position() const { return ctrl_->read_seq.load(std::memory_order_relaxed); }
    uint32_t pending_count() const {
        uint64_t w = write_position();
        uint64_t r = read_position();
        return static_cast<uint32_t>(w > r ? w - r : 0);
    }
    uint32_t slot_count() const { return ctrl_->slot_count; }
    uint32_t slot_payload_size() const { return ctrl_->slot_payload_size; }

private:
    static size_t compute_total_size(uint32_t slots, uint32_t payload_size) {
        size_t ctrl_sz = sizeof(ShmControlBlock);
        // Align to cache line
        ctrl_sz = (ctrl_sz + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1);
        size_t headers_sz = slots * sizeof(SlotHeader);
        headers_sz = (headers_sz + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1);
        size_t payload_arena = static_cast<size_t>(slots) * payload_size;
        return ctrl_sz + headers_sz + payload_arena;
    }

    SlotHeader* get_slot_header(uint32_t idx) {
        size_t ctrl_sz = sizeof(ShmControlBlock);
        ctrl_sz = (ctrl_sz + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1);
        auto* headers_base = base_ + ctrl_sz;
        return reinterpret_cast<SlotHeader*>(headers_base + idx * sizeof(SlotHeader));
    }

    void* get_payload_ptr(uint32_t idx) {
        size_t ctrl_sz = sizeof(ShmControlBlock);
        ctrl_sz = (ctrl_sz + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1);
        size_t headers_sz = ctrl_->slot_count * sizeof(SlotHeader);
        headers_sz = (headers_sz + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1);
        auto* payload_base = base_ + ctrl_sz + headers_sz;
        return payload_base + static_cast<size_t>(idx) * ctrl_->slot_payload_size;
    }

    std::string shm_name_;
    ShmRole role_;
    uint32_t slot_mask_{0};
    uint8_t* base_{nullptr};
    size_t mapped_size_{0};
    ShmControlBlock* ctrl_{nullptr};

#ifdef __linux__
    int fd_{-1};
#else
    uint32_t slot_count_{0};
    uint32_t slot_payload_size_{0};
    std::vector<uint8_t> heap_buf_;
#endif
};

} // namespace edge_scheduler
