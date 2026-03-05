# 🔍 C++ Code Review Prompt Template

**Quick Reference for Principal C++ Architect Reviews**  
**Project:** AuraSense SFSVC Neuromorphic Codec  
**Context:** Real-time drone infrastructure inspection (crack detection)

---

## 📋 Copy-Paste Review Prompt

Use this in GitHub Copilot Chat or Claude with C++ files open:

```
You are a principal C++ architect and code reviewer.

Context:
- This repo is a neuromorphic codec / crack-detection project.
- Target: Real-time processing (<1ms P95 latency, 125+ fps throughput)
- Architecture: Multi-lane lock-free processing with SIMD optimization
- Hardware: CPU-only (AVX2), GPU-optional
- Reliability: 99.9% uptime, zero frame drops over 72hr runs

Review this C++ code for:

1. **Performance:**
   - Is the hot path optimized (SIMD, cache-friendly, minimal allocations)?
   - Are atomic operations used correctly (memory ordering)?
   - Will this meet <6ms control loop SLA?

2. **Correctness:**
   - Are all edge cases handled (overflow, underflow, zero-sized inputs)?
   - Is the code thread-safe (no data races, no deadlocks)?
   - Are error codes checked and propagated?

3. **Reliability:**
   - Does failure degrade gracefully (not crash)?
   - Are watchdog/health metrics in place?
   - Can this run for 72 hours without leaks?

4. **Maintainability:**
   - Is the code self-documenting (clear naming)?
   - Are complex algorithms explained?
   - Does it follow existing patterns?

Specific concerns for neuromorphic codec:
- Frame differencing must be SIMD-accelerated (AVX2)
- Lock-free data structures for frame ingestion
- Lane 1 (control) must never block on Lanes 2-6 (analytics)
- Memory pre-allocated (no new/delete in hot path)

Provide:
✅ What's good
⚠️ What needs attention
🔴 Critical issues (blocks merge)
💡 Optimization suggestions
```

---

## 🎯 Quick Checklist (30-Second Review)

**Critical Path (Must Pass):**
- [ ] No `new`/`malloc` in hot path (frame processing loop)
- [ ] No mutexes in control loop (use lock-free queues)
- [ ] SIMD intrinsics for frame differencing (AVX2 minimum)
- [ ] All error codes checked (`if (!result) { handle_error(); }`)
- [ ] Thread-safe (no unprotected shared state)

**Performance (Should Pass):**
- [ ] Memory aligned (32-byte for AVX2, 64-byte for cache lines)
- [ ] CPU affinity set for critical threads
- [ ] Pre-allocated frame buffers (circular buffer, no resize)
- [ ] Prefetching used for memory-bound loops
- [ ] Branch prediction hints (`__builtin_expect`) for rare paths

**Reliability (Nice to Have):**
- [ ] Graceful degradation on lane failure
- [ ] Watchdog timeout for hung threads
- [ ] Health metrics exported (latency, drop rate)
- [ ] Unit tests with >80% coverage

---

## 🚨 Anti-Patterns (Immediate Rejection)

❌ **Forbidden in Hot Path:**
```cpp
// ❌ DON'T: Dynamic allocation
std::vector<uint8_t> buffer(size);

// ✅ DO: Pre-allocated pool
static std::array<uint8_t[MAX_SIZE], 16> buffer_pool;

// ❌ DON'T: Mutex in control loop
std::lock_guard<std::mutex> lock(mutex_);

// ✅ DO: Lock-free queue
spsc_queue.push(frame);

// ❌ DON'T: Exceptions in critical section
if (error) throw std::runtime_error("...");

// ✅ DO: Error codes
if (error) return ErrorCode::INVALID_FRAME;

// ❌ DON'T: Virtual calls in hot loop
for (...) { processor->process(frame); }

// ✅ DO: Static dispatch or inline
process_frame_inline(frame);
```

---

## 🔥 Common Issues & Fixes

### Issue 1: False Sharing (Cache Line Bouncing)

```cpp
// ❌ PROBLEM: Adjacent atomics on same cache line (64 bytes)
struct LaneState {
    std::atomic<uint64_t> lane1_counter;  // Byte 0-7
    std::atomic<uint64_t> lane2_counter;  // Byte 8-15 (same cache line!)
};
// Writing to lane1_counter invalidates lane2's cache!

// ✅ FIX: Align to cache line
struct alignas(64) LaneState {
    std::atomic<uint64_t> counter;
    // Pad to 64 bytes
};
```

### Issue 2: Unaligned SIMD Loads

```cpp
// ❌ PROBLEM: Unaligned buffer (segfault or slow path)
uint8_t* buffer = new uint8_t[1920 * 1080];
__m256i vec = _mm256_load_si256((__m256i*)buffer); // May crash!

// ✅ FIX: Aligned allocation
uint8_t* buffer = static_cast<uint8_t*>(
    aligned_alloc(32, 1920 * 1080));
// Or use _mm256_loadu_si256 (slower but safe)
```

### Issue 3: Memory Ordering Bugs

```cpp
// ❌ PROBLEM: Relaxed ordering allows reordering (data race)
data_ready.store(true, std::memory_order_relaxed);
shared_data = new_value;  // May be reordered before store!

// ✅ FIX: Release-acquire semantics
shared_data = new_value;
data_ready.store(true, std::memory_order_release);
// Reader uses: data_ready.load(std::memory_order_acquire)
```

---

## 📐 Architecture Patterns (Reference)

### Pattern 1: Lock-Free Frame Ingestion

```cpp
class RTCore {
    boost::lockfree::spsc_queue<Frame, 
        boost::lockfree::capacity<256>> frame_queue_;
    
    bool push_frame(const Frame& frame) {
        return frame_queue_.push(frame);  // Non-blocking
    }
    
    void process_loop() {
        Frame frame;
        while (frame_queue_.pop(frame)) {
            process_frame_internal(frame);
        }
    }
};
```

### Pattern 2: SIMD Frame Differencing

```cpp
void compute_diff_simd(const uint8_t* prev, const uint8_t* curr,
                       uint8_t* diff, size_t num_pixels) {
    for (size_t i = 0; i + 32 <= num_pixels; i += 32) {
        __m256i prev_vec = _mm256_loadu_si256((__m256i*)(prev + i));
        __m256i curr_vec = _mm256_loadu_si256((__m256i*)(curr + i));
        __m256i diff_vec = _mm256_abs_epi8(
            _mm256_sub_epi8(curr_vec, prev_vec));
        _mm256_storeu_si256((__m256i*)(diff + i), diff_vec);
    }
}
```

### Pattern 3: Multi-Lane Isolation

```cpp
class MultiLaneEngine {
    std::unique_ptr<ControlLane> control_lane_;  // Lane 1
    std::vector<std::unique_ptr<AnalyticsLane>> analytics_lanes_;
    
    void process_frame(const Frame& frame) {
        // Lane 1: Blocking (hard real-time)
        ControlDecision decision = control_lane_->process(frame);
        
        // Lanes 2-6: Non-blocking (soft real-time)
        for (auto& lane : analytics_lanes_) {
            lane->enqueue_async(frame);
        }
        
        emit_decision(decision);  // Don't wait for analytics
    }
};
```

---

## 🧪 Testing Requirements

### Latency Benchmark (Must Pass)

```bash
# Run 10,000 frames, report percentiles
./sfsvc_benchmark --frames 10000 --resolution 720p

# Expected output:
# P50: < 0.20 ms ✅
# P95: < 0.50 ms ✅
# P99: < 0.80 ms ✅
```

### Memory Leak Test (Must Pass)

```bash
# Valgrind with 1000 frames
valgrind --leak-check=full ./sfsvc_engine --frames 1000

# Expected: "no leaks are possible"
```

### Stress Test (Should Pass)

```bash
# 72-hour continuous run
./sfsvc_engine --input stress_test.mp4 --duration 72h

# Expected:
# - Zero crashes
# - Flat memory usage
# - P95 latency stable (±5%)
# - Drop rate < 0.01%
```

---

## 📞 Escalation & Resources

**Before Merging:**
1. Run automated checks (linters, static analysis)
2. Run latency benchmarks (must meet SLA)
3. Get architect approval for new patterns
4. Update `CPP_ARCHITECTURE_REVIEW.md` if architecture changes

**Need Help?**
- Latency regression → Profile with `perf record -g`
- Memory leak → Debug with `valgrind --leak-check=full`
- Data race → Test with `tsan` (ThreadSanitizer)
- Architecture questions → Email DicksonChau@aurasensehk.com

**References:**
- Full architecture doc: [`docs/CPP_ARCHITECTURE_REVIEW.md`](./CPP_ARCHITECTURE_REVIEW.md)
- Performance datasheet: [`docs/SFSVC_DATASHEET.md`](./SFSVC_DATASHEET.md)
- Intel intrinsics guide: https://www.intel.com/content/www/us/en/docs/intrinsics-guide/

---

## 🎯 TL;DR (5-Second Summary)

**For this neuromorphic codec:**
- ✅ Lock-free, SIMD-accelerated, zero-copy
- ✅ <1ms latency, 125+ fps, 94% compression
- ✅ CPU-only, 99.9% uptime, 72hr stress-tested
- ❌ No mutexes, no allocations, no exceptions in hot path

**Review focus:** Performance → Correctness → Reliability → Readability

