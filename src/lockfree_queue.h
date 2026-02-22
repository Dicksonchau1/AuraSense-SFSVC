// ============================================================================
// lockfree_queue.h  —  Production-grade SPSC Ring Buffer
// AuraSense MultiRateEngine  |  C++17  |  MSYS2 UCRT64
//
// Topology this file serves (engine.h):
//   Lane 1 (camera feeder)  ──▶  sig_queue_     ──▶  Lane 2 (signature)
//   Lane 1 (camera feeder)  ──▶  yolo_queue_    ──▶  Lane 3 (YOLO/crack)
//   Lane 1 (camera feeder)  ──▶  uplink_queue_  ──▶  Lane 4 (uplink/WS)
//
// Rules that MUST hold:
//   - Exactly ONE thread calls try_push / push_wait  (producer)
//   - Exactly ONE thread calls try_pop  / pop_wait   (consumer)
//   - N must be a power of 2  (64, 128, 256 …)
//
// Key features:
//   - Cached-index trick: avoids a full atomic load on every push/pop [web:63]
//   - Separate cache lines for head_, tail_, caches  (no false sharing) [web:57]
//   - Compile-time ring: fast bitmask modulo, zero heap allocation
//   - Blocking push_wait / pop_wait with timeout + exponential back-off
//   - Batch pop for bulk drain (useful in Lane 4 uplink batching)
//   - Stats struct readable by any thread (metric server)
//   - SPSCQueue<T,N> alias keeps engine.h unchanged
// ============================================================================

#pragma once

#include <atomic>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <new>
#include <thread>
#include <type_traits>
#include <utility>

// ---------------------------------------------------------------------------
// Cache-line size
// hardware_destructive_interference_size is defined by GCC/MinGW on C++17
// but may emit a -Winterference-size warning on some toolchains.
// We suppress it with a fallback to 64 bytes. [web:62]
// ---------------------------------------------------------------------------
namespace lfq_detail {

#if defined(__cpp_lib_hardware_interference_size)
    inline constexpr size_t CACHE_LINE = std::hardware_destructive_interference_size;
#else
    inline constexpr size_t CACHE_LINE = 64;
#endif

    constexpr bool is_pow2(size_t n) noexcept {
        return n >= 2 && (n & (n - 1)) == 0;
    }

    // Exponential back-off: yield → sleep(1µs) → sleep(10µs)
    // Used in blocking push_wait / pop_wait to be friendly to the OS. [web:61]
    struct Backoff {
        uint32_t count = 0;
        void spin() noexcept {
            if (count < 8) {
                // busy spin (L1 latency range)
            } else if (count < 16) {
                std::this_thread::yield();
            } else if (count < 32) {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            } else {
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
            ++count;
        }
        void reset() noexcept { count = 0; }
    };

} // namespace lfq_detail


// ============================================================================
//  LockFreeQueue<T, N>
// ============================================================================
template<typename T, size_t N>
class LockFreeQueue {

    static_assert(lfq_detail::is_pow2(N),
        "LockFreeQueue: N must be a power of 2 (e.g. 32, 64, 128, 256)");

    static_assert(std::is_nothrow_move_constructible_v<T> ||
                  std::is_nothrow_copy_constructible_v<T>,
        "LockFreeQueue: T must be nothrow move- or copy-constructible");

    static constexpr size_t MASK = N - 1;
    using Storage = std::aligned_storage_t<sizeof(T), alignof(T)>;

    // ─────────────────────────────────────────────────────────────────────────
    // Memory layout: each hot field lives on its own cache line to prevent
    // false sharing between the producer and consumer threads. [web:57][web:63]
    //
    //  ┌─ cache line 0 ─┐   tail_       (producer writes, consumer reads)
    //  ├─ cache line 1 ─┤   cached_head_ (producer private, never shared)
    //  ├─ cache line 2 ─┤   head_       (consumer writes, producer reads)
    //  ├─ cache line 3 ─┤   cached_tail_ (consumer private, never shared)
    //  └─ data buffer ──┘   buffer_[N]
    // ─────────────────────────────────────────────────────────────────────────

    alignas(lfq_detail::CACHE_LINE) std::atomic<size_t> tail_{0};
    alignas(lfq_detail::CACHE_LINE) size_t              cached_head_{0};   // producer local
    alignas(lfq_detail::CACHE_LINE) std::atomic<size_t> head_{0};
    alignas(lfq_detail::CACHE_LINE) size_t              cached_tail_{0};   // consumer local

    // Telemetry — relaxed loads/stores only, approximate but safe [web:28]
    alignas(lfq_detail::CACHE_LINE)
        std::atomic<uint64_t> push_count_{0};
    std::atomic<uint64_t>     pop_count_{0};
    std::atomic<uint64_t>     drop_count_{0};   // full-queue pushes silently dropped

    // Ring buffer — no per-slot padding needed for SPSC [web:28]
    alignas(lfq_detail::CACHE_LINE) Storage buffer_[N];

public:

    // ── Lifecycle ─────────────────────────────────────────────────────────────

    LockFreeQueue()  = default;

    ~LockFreeQueue() {
        // Properly destroy any live objects still in the ring.
        T discard;
        while (try_pop(discard)) {}
    }

    LockFreeQueue(const LockFreeQueue&)            = delete;
    LockFreeQueue& operator=(const LockFreeQueue&) = delete;
    LockFreeQueue(LockFreeQueue&&)                 = delete;
    LockFreeQueue& operator=(LockFreeQueue&&)      = delete;


    // ── Push  (PRODUCER THREAD ONLY) ─────────────────────────────────────────

    // Non-blocking copy push.  Returns false if queue is full.
    bool try_push(const T& item)
        noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
        return push_impl(item);
    }

    // Non-blocking move push.  Returns false if queue is full.
    bool try_push(T&& item)
        noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        return push_impl(std::move(item));
    }

    // In-place construction push (avoids extra move).  Returns false if full.
    template<typename... Args>
    bool try_emplace(Args&&... args)
        noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        const size_t tail = tail_.load(std::memory_order_relaxed);
        if (tail - cached_head_ >= N) {
            cached_head_ = head_.load(std::memory_order_acquire);
            if (tail - cached_head_ >= N) {
                drop_count_.fetch_add(1, std::memory_order_relaxed);
                return false;
            }
        }
        ::new (static_cast<void*>(&buffer_[tail & MASK]))
            T(std::forward<Args>(args)...);
        tail_.store(tail + 1, std::memory_order_release);
        push_count_.fetch_add(1, std::memory_order_relaxed);
        return true;
    }

    // Blocking move push with timeout.  Spins with back-off.
    // Returns false on timeout (item is left unmodified). [web:61]
    template<typename Rep, typename Period>
    bool push_wait(T&& item,
                   std::chrono::duration<Rep, Period> timeout) noexcept
    {
        auto deadline = std::chrono::steady_clock::now() + timeout;
        lfq_detail::Backoff bo;
        while (!try_push(std::move(item))) {
            if (std::chrono::steady_clock::now() >= deadline) {
                drop_count_.fetch_add(1, std::memory_order_relaxed);
                return false;
            }
            bo.spin();
        }
        return true;
    }


    // ── Pop  (CONSUMER THREAD ONLY) ──────────────────────────────────────────

    // Non-blocking pop.  Returns false if queue is empty.
    [[nodiscard]]
    bool try_pop(T& out) noexcept {
        const size_t head = head_.load(std::memory_order_relaxed);
        if (head == cached_tail_) {
            cached_tail_ = tail_.load(std::memory_order_acquire);
            if (head == cached_tail_) return false;   // truly empty
        }
        T* elem = reinterpret_cast<T*>(&buffer_[head & MASK]);
        out = std::move(*elem);
        elem->~T();
        head_.store(head + 1, std::memory_order_release);
        pop_count_.fetch_add(1, std::memory_order_relaxed);
        return true;
    }

    // Blocking pop with timeout.  Spins with back-off. [web:61]
    template<typename Rep, typename Period>
    [[nodiscard]]
    bool pop_wait(T& out,
                  std::chrono::duration<Rep, Period> timeout) noexcept
    {
        auto deadline = std::chrono::steady_clock::now() + timeout;
        lfq_detail::Backoff bo;
        while (!try_pop(out)) {
            if (std::chrono::steady_clock::now() >= deadline) return false;
            bo.spin();
        }
        return true;
    }

    // Batch pop: drains up to max_count items into dst[].
    // Returns number of items actually popped.
    // Useful for Lane 4 uplink batching — one syscall per batch. [web:28]
    size_t try_pop_batch(T* dst, size_t max_count) noexcept {
        size_t popped = 0;
        while (popped < max_count && try_pop(dst[popped])) {
            ++popped;
        }
        return popped;
    }


    // ── Status queries  (approximate — callable from any thread) ─────────────

    [[nodiscard]] bool   empty()       const noexcept {
        return head_.load(std::memory_order_acquire) ==
               tail_.load(std::memory_order_acquire);
    }

    [[nodiscard]] bool   full()        const noexcept {
        return size_approx() >= N;
    }

    [[nodiscard]] size_t size_approx() const noexcept {
        const size_t t = tail_.load(std::memory_order_acquire);
        const size_t h = head_.load(std::memory_order_acquire);
        return t - h;
    }

    static constexpr size_t capacity() noexcept { return N; }


    // ── Telemetry — read by metric server / HardwareProfiler ─────────────────

    struct Stats {
        uint64_t push_count;
        uint64_t pop_count;
        uint64_t drop_count;   // frames silently dropped (queue was full)
        size_t   current_size;
        size_t   queue_capacity;
        float    fill_ratio;   // 0.0 – 1.0
    };

    [[nodiscard]] Stats stats() const noexcept {
        const uint64_t p  = push_count_.load(std::memory_order_relaxed);
        const uint64_t c  = pop_count_.load(std::memory_order_relaxed);
        const uint64_t d  = drop_count_.load(std::memory_order_relaxed);
        const size_t   sz = size_approx();
        return {p, c, d, sz, N, static_cast<float>(sz) / static_cast<float>(N)};
    }

    void reset_stats() noexcept {
        push_count_.store(0, std::memory_order_relaxed);
        pop_count_.store(0,  std::memory_order_relaxed);
        drop_count_.store(0, std::memory_order_relaxed);
    }

private:

    // ── Internal push helper ──────────────────────────────────────────────────
    // Uses cached_head_ to avoid hitting the consumer's cache line on every
    // push.  Only refreshes from the shared atomic when the local cache says
    // the queue is full — the same trick used in rigtorp/SPSCQueue. [web:63]

    template<typename U>
    bool push_impl(U&& item)
        noexcept(std::is_nothrow_constructible_v<T, U&&>)
    {
        const size_t tail = tail_.load(std::memory_order_relaxed);

        // Fast path: use cached head, avoids atomic read of head_. [web:57]
        if (tail - cached_head_ >= N) {
            // Slow path: refresh cache from shared head_.
            cached_head_ = head_.load(std::memory_order_acquire);
            if (tail - cached_head_ >= N) {
                drop_count_.fetch_add(1, std::memory_order_relaxed);
                return false;   // genuinely full
            }
        }

        // Placement-new into the aligned slot.
        ::new (static_cast<void*>(&buffer_[tail & MASK]))
            T(std::forward<U>(item));

        // Publish: release so consumer sees the constructed object. [web:57]
        tail_.store(tail + 1, std::memory_order_release);
        push_count_.fetch_add(1, std::memory_order_relaxed);
        return true;
    }
};


// ============================================================================
//  SPSCQueue<T, N>  — alias that matches your existing engine.h declarations
//
//  engine.h uses:
//    SPSCQueue<Frame,         CAMERA_QUEUE_CAP>  camera_queue_;
//    SPSCQueue<SignatureMatch*,SIG_QUEUE_CAP>    sig_queue_;
//    SPSCQueue<Frame,         YOLO_QUEUE_CAP>    yolo_queue_;
//    SPSCQueue<UplinkPayload, UPLINK_QUEUE_CAP>  uplink_queue_;
// ============================================================================
template<typename T, size_t N>
using SPSCQueue = LockFreeQueue<T, N>;