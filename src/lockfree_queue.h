// ============================================================================
// lockfree_queue.h — Production-Grade SPSC Ring Buffer (ABI-Stable)
// C++17 | Lock-Free | Cache-Line Safe | No ABI Warnings
// ============================================================================

#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <new>
#include <thread>
#include <type_traits>
#include <utility>

// ============================================================================
// Stable cache-line size (avoid hardware_destructive_interference_size ABI)
// ============================================================================

namespace lfq_detail {

    // ✅ Stabilized constant to avoid compiler ABI warnings
    inline constexpr size_t CACHE_LINE = 64;

    constexpr bool is_pow2(size_t n) noexcept {
        return n >= 2 && (n & (n - 1)) == 0;
    }

    // Exponential backoff
    struct Backoff {
        uint32_t count = 0;

        void spin() noexcept {
            if (count < 8) {
                // busy spin
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
// LockFreeQueue<T, N>  (Single Producer, Single Consumer)
// ============================================================================

template<typename T, size_t N>
class LockFreeQueue {

    static_assert(lfq_detail::is_pow2(N),
        "LockFreeQueue: N must be power-of-two");

    static_assert(std::is_nothrow_move_constructible_v<T> ||
                  std::is_nothrow_copy_constructible_v<T>,
        "LockFreeQueue: T must be nothrow move- or copy-constructible");

    static constexpr size_t MASK = N - 1;
    using Storage = std::aligned_storage_t<sizeof(T), alignof(T)>;

    // ------------------------------------------------------------------------
    // Cache-line isolated hot fields
    // ------------------------------------------------------------------------

    alignas(lfq_detail::CACHE_LINE)
    std::atomic<size_t> tail_{0};

    alignas(lfq_detail::CACHE_LINE)
    size_t cached_head_{0};

    alignas(lfq_detail::CACHE_LINE)
    std::atomic<size_t> head_{0};

    alignas(lfq_detail::CACHE_LINE)
    size_t cached_tail_{0};

    alignas(lfq_detail::CACHE_LINE)
    std::atomic<uint64_t> push_count_{0};

    std::atomic<uint64_t> pop_count_{0};
    std::atomic<uint64_t> drop_count_{0};

    alignas(lfq_detail::CACHE_LINE)
    Storage buffer_[N];

public:

    LockFreeQueue() = default;

    ~LockFreeQueue() {
        T tmp;
        while (try_pop(tmp)) {}
    }

    LockFreeQueue(const LockFreeQueue&) = delete;
    LockFreeQueue& operator=(const LockFreeQueue&) = delete;

    // =========================================================================
    // Producer
    // =========================================================================

    bool try_push(const T& item)
        noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
        return push_impl(item);
    }

    bool try_push(T&& item)
        noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        return push_impl(std::move(item));
    }

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

    // =========================================================================
    // Consumer
    // =========================================================================

    [[nodiscard]]
    bool try_pop(T& out) noexcept
    {
        const size_t head = head_.load(std::memory_order_relaxed);

        if (head == cached_tail_) {
            cached_tail_ = tail_.load(std::memory_order_acquire);
            if (head == cached_tail_)
                return false;
        }

        T* elem = reinterpret_cast<T*>(&buffer_[head & MASK]);
        out = std::move(*elem);
        elem->~T();

        head_.store(head + 1, std::memory_order_release);
        pop_count_.fetch_add(1, std::memory_order_relaxed);
        return true;
    }

    template<typename Rep, typename Period>
    [[nodiscard]]
    bool pop_wait(T& out,
                  std::chrono::duration<Rep, Period> timeout) noexcept
    {
        auto deadline = std::chrono::steady_clock::now() + timeout;
        lfq_detail::Backoff bo;

        while (!try_pop(out)) {
            if (std::chrono::steady_clock::now() >= deadline)
                return false;
            bo.spin();
        }
        return true;
    }

    size_t try_pop_batch(T* dst, size_t max_count) noexcept
    {
        size_t popped = 0;
        while (popped < max_count && try_pop(dst[popped])) {
            ++popped;
        }
        return popped;
    }

    // =========================================================================
    // Status
    // =========================================================================

    [[nodiscard]] bool empty() const noexcept {
        return head_.load(std::memory_order_acquire) ==
               tail_.load(std::memory_order_acquire);
    }

    [[nodiscard]] size_t size_approx() const noexcept {
        const size_t t = tail_.load(std::memory_order_acquire);
        const size_t h = head_.load(std::memory_order_acquire);
        return t - h;
    }

    [[nodiscard]] bool full() const noexcept {
        return size_approx() >= N;
    }

    static constexpr size_t capacity() noexcept {
        return N;
    }

    // =========================================================================
    // Telemetry
    // =========================================================================

    struct Stats {
        uint64_t push_count;
        uint64_t pop_count;
        uint64_t drop_count;
        size_t current_size;
        size_t queue_capacity;
        float fill_ratio;
    };

    [[nodiscard]]
    Stats stats() const noexcept
    {
        const uint64_t p = push_count_.load(std::memory_order_relaxed);
        const uint64_t c = pop_count_.load(std::memory_order_relaxed);
        const uint64_t d = drop_count_.load(std::memory_order_relaxed);
        const size_t sz  = size_approx();

        return {p, c, d, sz, N,
                static_cast<float>(sz) / static_cast<float>(N)};
    }

    void reset_stats() noexcept
    {
        push_count_.store(0, std::memory_order_relaxed);
        pop_count_.store(0,  std::memory_order_relaxed);
        drop_count_.store(0, std::memory_order_relaxed);
    }

private:

    template<typename U>
    bool push_impl(U&& item)
        noexcept(std::is_nothrow_constructible_v<T, U&&>)
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
            T(std::forward<U>(item));

        tail_.store(tail + 1, std::memory_order_release);
        push_count_.fetch_add(1, std::memory_order_relaxed);
        return true;
    }
};


// Alias
template<typename T, size_t N>
using SPSCQueue = LockFreeQueue<T, N>;