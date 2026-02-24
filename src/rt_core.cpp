/**
 * RT Core - Ultra-Optimized Real-Time Processing Engine
 *
 * ALL ENHANCEMENTS INTEGRATED:
 * ✅ DAY 1: Temporal differencing with resolution safety (assert)
 * ✅ DAY 2: Integer grayscale conversion (BT.709 fixed-point)
 * ✅ DAY 2: Precomputed LUT for spike compression (no pow())
 * ✅ DAY 2: Single-pass pipeline (delta + count + crack in one loop)
 * ✅ DAY 2: Lock-free YOLO snapshot (double-buffer atomic)
 * ✅ DAY 5: Throttle LUT (optional, deterministic mapping)
 * ✅ Lateral inhibition (counts-only, scanline-causal)
 * ✅ Metrics tracking with nth_element percentiles
 *
 * TARGET: <0.2ms latency for 234×416 BGR frame
 *
 * Author: AuraSense RT Team
 * Date: 2026-02-22
 * Version: 2.0-fully-optimized + lateral-inhibition
 */

#include "rt_core_enhanced.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <chrono>
#include <atomic>
#include <cassert>
#include <vector>

#ifdef __linux__
#include <sched.h>
#include <pthread.h>
#endif

// =============================================================================
// Configuration
// =============================================================================

namespace {

constexpr int TARGET_WIDTH = 416;
constexpr int TARGET_HEIGHT = 234;
constexpr int TOTAL_PIXELS = TARGET_HEIGHT * TARGET_WIDTH;

// Event-based thresholds (uint8 domain: 0-255)
constexpr int THRESHOLD_ON_U8 = 8;   // ~0.03 * 255
constexpr int THRESHOLD_OFF_U8 = 8;

// Crack detection ROI (bottom 1/3)
constexpr int CRACK_ROI_START_Y = (TARGET_HEIGHT * 2) / 3;
constexpr int CRACK_GRADIENT_THRESHOLD_U8 = 77;  // ~0.3 * 255

// YOLO staleness threshold
constexpr double YOLO_MAX_AGE_S = 5.0;

// LUT size for spike compression
constexpr int SPIKE_LUT_SIZE = 256;

// Metrics tracking
constexpr int MAX_LATENCY_SAMPLES = 10000;

// Lateral inhibition (counts-only, scanline-causal)
constexpr int LATERAL_INHIBITION_RADIUS = 3; // pixels

// =============================================================================
// Lock-Free YOLO State (Double-Buffered Atomic Snapshot)
// =============================================================================

struct alignas(64) YoloSnapshot {
    double timestamp_s;
    float front_risk;
    float left_risk;
    float right_risk;
    float crack_risk;
    float min_distance_m;
    float max_confidence;
    int num_detections;
    int priority_detections;
    int num_filtered_out;
    bool valid;

    // Padding to cache line boundary
    char padding[64 - sizeof(double) - 9*sizeof(float) - 3*sizeof(int) - sizeof(bool)];
};

static_assert(sizeof(YoloSnapshot) == 64, "YoloSnapshot must be exactly 64 bytes");

// Double buffer for lock-free read/write
alignas(64) YoloSnapshot g_yolo_snapshots[2];
std::atomic<int> g_yolo_read_index{0};
std::atomic<int> g_yolo_write_index{1};

// Vehicle speed (atomic)
std::atomic<float> g_vehicle_speed_mps{0.0f};
std::atomic<float> g_yolo_target_hz{5.0f};

// =============================================================================
// Precomputed LUT for Spike Compression (Replaces pow())
// =============================================================================

alignas(64) float g_spike_lut[SPIKE_LUT_SIZE];

void init_spike_lut() {
    // Precompute: pow((delta - threshold) / threshold, 0.7) for uint8 range
    // Map uint8 delta [0, 255] → spike strength [0, 1]

    constexpr float SPIKE_POWER = 0.7f;
    constexpr float THRESHOLD_F = static_cast<float>(THRESHOLD_ON_U8);

    for (int i = 0; i < SPIKE_LUT_SIZE; ++i) {
        float delta = static_cast<float>(i);

        if (delta <= THRESHOLD_F) {
            g_spike_lut[i] = 0.0f;
        } else {
            float ratio = (delta - THRESHOLD_F) / THRESHOLD_F;
            g_spike_lut[i] = std::min(std::pow(ratio, SPIKE_POWER), 1.0f);
        }
    }
}

inline float spike_compress_lut(int delta_u8) {
    return g_spike_lut[std::clamp(delta_u8, 0, SPIKE_LUT_SIZE - 1)];
}

// =============================================================================
// Throttle LUT (Deterministic Mapping: crack → throttle)
// =============================================================================

alignas(64) float g_throttle_lut[256];

void init_throttle_lut() {
    // Map crack score [0.0, 1.0] → throttle [1.0, 0.3]
    // Thresholds: >0.5 → 0.3, >0.2 → 0.7, else 1.0

    for (int i = 0; i < 256; ++i) {
        float crack_norm = static_cast<float>(i) / 255.0f;

        if (crack_norm > 0.5f) {
            g_throttle_lut[i] = 0.3f;
        } else if (crack_norm > 0.2f) {
            g_throttle_lut[i] = 0.7f;
        } else {
            g_throttle_lut[i] = 1.0f;
        }
    }
}

inline float throttle_from_crack_lut(float crack_score) {
    int idx = static_cast<int>(crack_score * 255.0f);
    idx = std::clamp(idx, 0, 255);
    return g_throttle_lut[idx];
}

// =============================================================================
// Frame Buffers (Minimal - Only Previous Grayscale Needed)
// =============================================================================

alignas(64) static uint8_t* g_prev_gray_u8 = nullptr;
alignas(64) static uint8_t* g_curr_gray_u8 = nullptr;

// Lateral inhibition buffers (counts-only; rolling rows)
alignas(64) static uint8_t* g_on_history = nullptr;   // [radius][width]
alignas(64) static uint8_t* g_off_history = nullptr;  // [radius][width]
alignas(64) static uint8_t* g_on_curr = nullptr;      // [width]
alignas(64) static uint8_t* g_off_curr = nullptr;     // [width]

static int g_frame_count = 0;

// =============================================================================
// Metrics Tracking
// =============================================================================

struct MetricsTracker {
    std::vector<float> latencies;
    std::atomic<uint64_t> total_frames{0};
    std::atomic<uint64_t> spike_events{0};

    MetricsTracker() {
        latencies.reserve(MAX_LATENCY_SAMPLES);
    }

    void record_latency(float ms) {
        if (latencies.size() < MAX_LATENCY_SAMPLES) {
            latencies.push_back(ms);
        } else {
            // Ring buffer behavior
            latencies[total_frames.load() % MAX_LATENCY_SAMPLES] = ms;
        }
        total_frames.fetch_add(1, std::memory_order_relaxed);
    }

    // Fast percentile using nth_element (O(n), not O(n log n))
    float get_percentile(float p) {
        if (latencies.empty()) return 0.0f;

        std::vector<float> tmp = latencies;
        size_t n = tmp.size();
        size_t k = static_cast<size_t>(p * n);

        if (k >= n) k = n - 1;

        std::nth_element(tmp.begin(), tmp.begin() + k, tmp.end());
        return tmp[k];
    }
};

static MetricsTracker g_metrics;

// =============================================================================
// Timing
// =============================================================================

auto g_start_time = std::chrono::steady_clock::now();

inline double get_time_seconds() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(now - g_start_time).count();
}

inline uint64_t get_time_ns() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        now - g_start_time).count();
}

// =============================================================================
// Initialization (Called Once)
// =============================================================================

void ensure_initialized() {
    static bool initialized = false;
    if (initialized) return;

    // Allocate grayscale buffers (uint8, not float!)
    g_prev_gray_u8 = new uint8_t[TOTAL_PIXELS];
    g_curr_gray_u8 = new uint8_t[TOTAL_PIXELS];

    std::memset(g_prev_gray_u8, 0, TOTAL_PIXELS);
    std::memset(g_curr_gray_u8, 0, TOTAL_PIXELS);

    // Lateral inhibition buffers
    if (LATERAL_INHIBITION_RADIUS > 0) {
        const size_t row_bytes = static_cast<size_t>(TARGET_WIDTH);
        const size_t hist_bytes = row_bytes * static_cast<size_t>(LATERAL_INHIBITION_RADIUS);

        g_on_history = new uint8_t[hist_bytes];
        g_off_history = new uint8_t[hist_bytes];
        g_on_curr = new uint8_t[row_bytes];
        g_off_curr = new uint8_t[row_bytes];

        std::memset(g_on_history, 0, hist_bytes);
        std::memset(g_off_history, 0, hist_bytes);
        std::memset(g_on_curr, 0, row_bytes);
        std::memset(g_off_curr, 0, row_bytes);
    }

    // Initialize LUTs
    init_spike_lut();
    init_throttle_lut();

    // Initialize YOLO snapshots
    std::memset(g_yolo_snapshots, 0, sizeof(g_yolo_snapshots));

    initialized = true;
}

// =============================================================================
// DAY 2: Integer Grayscale Conversion (BT.709 Fixed-Point)
// =============================================================================

inline void bgr_to_gray_u8(const uint8_t* bgr, uint8_t* gray, int pixels) {
    // ITU-R BT.709 coefficients in fixed-point (scaled by 256)
    // R: 0.2126 * 256 = 54.4 ≈ 54
    // G: 0.7152 * 256 = 183.1 ≈ 183
    // B: 0.0722 * 256 = 18.5 ≈ 19
    // Total = 256 (perfect)

    constexpr int R_COEF = 54;
    constexpr int G_COEF = 183;
    constexpr int B_COEF = 19;

    for (int i = 0; i < pixels; ++i) {
        int b = bgr[i * 3 + 0];
        int g = bgr[i * 3 + 1];
        int r = bgr[i * 3 + 2];

        // Integer math: (54*R + 183*G + 19*B) >> 8
        int val = (R_COEF * r + G_COEF * g + B_COEF * b) >> 8;
        gray[i] = static_cast<uint8_t>(val);
    }
}

// =============================================================================
// Lateral Inhibition (Counts-only, scanline-causal)
// =============================================================================

inline bool inhibited_causal(
    const uint8_t* curr_row,
    const uint8_t* history_rows,
    int width,
    int x,
    int radius,
    int rows_valid,
    int rows_processed
) {
    if (radius <= 0) return false;

    int x0 = x - radius;
    if (x0 < 0) x0 = 0;

    // Check left neighbors in current row
    for (int xi = x0; xi < x; ++xi) {
        if (curr_row[xi]) return true;
    }

    // Check previous rows (rolling buffer)
    for (int k = 1; k <= rows_valid; ++k) {
        int idx = rows_processed - k;
        int slot = idx % radius;
        const uint8_t* row = history_rows + static_cast<size_t>(slot) * static_cast<size_t>(width);

        int x1 = x + radius;
        if (x1 >= width) x1 = width - 1;

        for (int xi = x0; xi <= x1; ++xi) {
            if (row[xi]) return true;
        }
    }

    return false;
}

// =============================================================================
// DAY 2: Single-Pass Pipeline (Delta + Count + Crack in ONE Loop)
// =============================================================================

struct PipelineResult {
    int on_count;
    int off_count;
    float crack_score;
};

PipelineResult single_pass_pipeline(
    const uint8_t* curr_gray,
    const uint8_t* prev_gray,
    int width,
    int height
) {
    PipelineResult result = {0, 0, 0.0f};

    float crack_accumulator = 0.0f;
    int crack_pixels = 0;

    const int roi_start_y = CRACK_ROI_START_Y;

    // Reset inhibition buffers per frame
    if (LATERAL_INHIBITION_RADIUS > 0) {
        const size_t row_bytes = static_cast<size_t>(width);
        const size_t hist_bytes = row_bytes * static_cast<size_t>(LATERAL_INHIBITION_RADIUS);
        std::memset(g_on_history, 0, hist_bytes);
        std::memset(g_off_history, 0, hist_bytes);
        std::memset(g_on_curr, 0, row_bytes);
        std::memset(g_off_curr, 0, row_bytes);
    }

    int rows_processed = 0;

    for (int y = 1; y < height - 1; ++y) {
        const int rows_valid = (rows_processed < LATERAL_INHIBITION_RADIUS)
            ? rows_processed
            : LATERAL_INHIBITION_RADIUS;

        if (LATERAL_INHIBITION_RADIUS > 0) {
            std::memset(g_on_curr, 0, static_cast<size_t>(width));
            std::memset(g_off_curr, 0, static_cast<size_t>(width));
        }

        for (int x = 1; x < width - 1; ++x) {
            const int idx = y * width + x;

            // ──────────────────────────────────────────────────────────
            // Temporal delta (signed)
            // ──────────────────────────────────────────────────────────
            int delta = static_cast<int>(curr_gray[idx]) - static_cast<int>(prev_gray[idx]);

            // ──────────────────────────────────────────────────────────
            // Count spikes (ON/OFF) with lateral inhibition
            // ──────────────────────────────────────────────────────────
            if (delta > THRESHOLD_ON_U8) {
                bool inhibited = false;
                if (LATERAL_INHIBITION_RADIUS > 0) {
                    inhibited = inhibited_causal(
                        g_on_curr, g_on_history, width, x,
                        LATERAL_INHIBITION_RADIUS, rows_valid, rows_processed
                    );
                }
                if (!inhibited) {
                    result.on_count++;
                    g_metrics.spike_events.fetch_add(1, std::memory_order_relaxed);
                    if (LATERAL_INHIBITION_RADIUS > 0) g_on_curr[x] = 1;
                }
            } else if (delta < -THRESHOLD_OFF_U8) {
                bool inhibited = false;
                if (LATERAL_INHIBITION_RADIUS > 0) {
                    inhibited = inhibited_causal(
                        g_off_curr, g_off_history, width, x,
                        LATERAL_INHIBITION_RADIUS, rows_valid, rows_processed
                    );
                }
                if (!inhibited) {
                    result.off_count++;
                    g_metrics.spike_events.fetch_add(1, std::memory_order_relaxed);
                    if (LATERAL_INHIBITION_RADIUS > 0) g_off_curr[x] = 1;
                }
            }

            // ──────────────────────────────────────────────────────────
            // Crack detection (only in bottom 1/3)
            // ──────────────────────────────────────────────────────────
            if (y >= roi_start_y) {
                int left = curr_gray[idx - 1];
                int right = curr_gray[idx + 1];
                int grad_abs = std::abs(right - left);

                if (grad_abs > CRACK_GRADIENT_THRESHOLD_U8) {
                    crack_accumulator += static_cast<float>(grad_abs) / 255.0f;
                    crack_pixels++;
                }
            }
        }

        // Store current row into rolling history
        if (LATERAL_INHIBITION_RADIUS > 0) {
            int slot = rows_processed % LATERAL_INHIBITION_RADIUS;
            uint8_t* on_row = g_on_history + static_cast<size_t>(slot) * static_cast<size_t>(width);
            uint8_t* off_row = g_off_history + static_cast<size_t>(slot) * static_cast<size_t>(width);
            std::memcpy(on_row, g_on_curr, static_cast<size_t>(width));
            std::memcpy(off_row, g_off_curr, static_cast<size_t>(width));
        }

        rows_processed++;
    }

    // Normalize crack score
    const int roi_pixels = (height - roi_start_y) * (width - 2);
    if (roi_pixels > 0) {
        result.crack_score = crack_accumulator / static_cast<float>(roi_pixels);
    }

    return result;
}

// =============================================================================
// Lock-Free YOLO Snapshot Read
// =============================================================================

inline YoloSnapshot read_yolo_snapshot() {
    int idx = g_yolo_read_index.load(std::memory_order_acquire);
    return g_yolo_snapshots[idx];
}

// =============================================================================
// YOLO Fusion (Lock-Free)
// =============================================================================

inline float apply_yolo_fusion_lockfree(float raw_crack_score, double current_time_s) {
    YoloSnapshot snap = read_yolo_snapshot();

    if (!snap.valid) {
        return raw_crack_score;
    }

    double age = current_time_s - snap.timestamp_s;
    if (age > YOLO_MAX_AGE_S) {
        return raw_crack_score;
    }

    // Fusion: boost crack if YOLO detects crack-related objects
    float boost = 1.0f + (snap.crack_risk * 0.5f);

    // Attenuate if high collision risk (prioritize obstacles)
    float obstacle_risk = std::max({snap.front_risk, snap.left_risk, snap.right_risk});

    if (obstacle_risk > 0.6f) {
        boost *= 0.7f;
    }

    return std::min(1.0f, raw_crack_score * boost);
}

// =============================================================================
// Adaptive YOLO Hz Update
// =============================================================================

inline void update_yolo_target_hz() {
    float speed = g_vehicle_speed_mps.load(std::memory_order_relaxed);

    float target_hz;
    if (speed > 1.0f) {
        target_hz = 20.0f;
    } else if (speed > 0.5f) {
        target_hz = 10.0f + (speed - 0.5f) * 20.0f;
    } else {
        target_hz = 5.0f + speed * 10.0f;
    }

    g_yolo_target_hz.store(std::clamp(target_hz, 5.0f, 20.0f), std::memory_order_relaxed);
}

// =============================================================================
// DAY 5: Control Decision (Throttle LUT)
// =============================================================================

inline void make_control_decision(float fused_crack, float* throttle, float* steer) {
    *throttle = throttle_from_crack_lut(fused_crack);
    *steer = 0.0f;
}

}  // anonymous namespace

// =============================================================================
// Public C API
// =============================================================================

extern "C" {

int rt_core_target_width(void) {
    return TARGET_WIDTH;
}

int rt_core_target_height(void) {
    return TARGET_HEIGHT;
}

int rt_core_pin_thread(int core_id) {
#ifdef __linux__
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    pthread_t thread = pthread_self();
    int result = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);

    return (result == 0) ? 1 : 0;
#else
    (void)core_id;
    return 0;
#endif
}

// =============================================================================
// HOT PATH - Main Frame Processing
//
// DAY 1: Resolution safety with assert
// DAY 2: Integer pipeline, single-pass, lock-free YOLO
// DAY 5: Throttle LUT
//
// THREAD SAFETY: SINGLE WRITER ONLY
// =============================================================================

ControlOutput rt_core_process_frame_ptr(
    const uint8_t* bgr,
    int            height,
    int            width
) {
    ensure_initialized();

    const uint64_t t_start_ns = get_time_ns();

    ControlOutput output;
    std::memset(&output, 0, sizeof(output));

    // ═════════════════════════════════════════════════════════════════════
    // DAY 1: Resolution Safety (Assert + Early Return)
    // ═════════════════════════════════════════════════════════════════════

    if (!bgr) {
        output.frame_id = -1;
        return output;
    }

    // Critical: Enforce fixed resolution (event-based encoding requires this)
    if (height != TARGET_HEIGHT || width != TARGET_WIDTH) {
        assert(false && "Resolution mismatch - event encoder requires fixed size!");
        output.frame_id = -1;
        return output;
    }

    const double current_time_s = get_time_seconds();

    // ═════════════════════════════════════════════════════════════════════
    // DAY 2: Integer Grayscale Conversion (BT.709 Fixed-Point)
    // ═════════════════════════════════════════════════════════════════════

    bgr_to_gray_u8(bgr, g_curr_gray_u8, TOTAL_PIXELS);

    // ═════════════════════════════════════════════════════════════════════
    // DAY 2: Single-Pass Pipeline (Delta + Count + Crack)
    // ═════════════════════════════════════════════════════════════════════

    PipelineResult pipeline;

    if (g_frame_count > 0) {
        pipeline = single_pass_pipeline(
            g_curr_gray_u8,
            g_prev_gray_u8,
            TARGET_WIDTH,
            TARGET_HEIGHT
        );
    } else {
        // First frame: no temporal reference
        pipeline = {0, 0, 0.0f};
    }

    output.on_spike_count = pipeline.on_count;
    output.off_spike_count = pipeline.off_count;

    // Compute sparsity
    float total_spikes = static_cast<float>(pipeline.on_count + pipeline.off_count);
    output.sparsity = 1.0f - (total_spikes / static_cast<float>(TOTAL_PIXELS));

    // Raw crack score
    output.crack_score = pipeline.crack_score;

    // ═════════════════════════════════════════════════════════════════════
    // DAY 2: YOLO Fusion (Lock-Free)
    // ═════════════════════════════════════════════════════════════════════

    output.fused_crack_score = apply_yolo_fusion_lockfree(
        output.crack_score,
        current_time_s
    );

    // ═════════════════════════════════════════════════════════════════════
    // DAY 5: Control Decision (Throttle LUT)
    // ═════════════════════════════════════════════════════════════════════

    make_control_decision(
        output.fused_crack_score,
        &output.throttle,
        &output.steer
    );

    // Inference suppression logic
    output.inference_suppressed = (output.sparsity > 0.95f) ? 1 : 0;
    output.event_only_mode = (output.sparsity > 0.98f) ? 1 : 0;
    output.is_null_cycle = (g_frame_count == 0) ? 1 : 0;

    // ═════════════════════════════════════════════════════════════════════
    // YOLO State (Lock-Free Read)
    // ═════════════════════════════════════════════════════════════════════

    YoloSnapshot yolo = read_yolo_snapshot();

    output.yolo_active = yolo.valid ? 1 : 0;

    if (yolo.valid) {
        double age = current_time_s - yolo.timestamp_s;
        output.yolo_age_ms = static_cast<float>(age * 1000.0);
    } else {
        output.yolo_age_ms = 99999.0f;
    }

    output.yolo_target_hz = g_yolo_target_hz.load(std::memory_order_relaxed);

    // Metadata
    output.frame_id = g_frame_count;
    output.reference_frame_age = 1;

    // ═════════════════════════════════════════════════════════════════════
    // Latency Measurement
    // ═════════════════════════════════════════════════════════════════════

    const uint64_t t_end_ns = get_time_ns();
    const float latency_ms = static_cast<float>(t_end_ns - t_start_ns) / 1e6f;

    output.encode_time_ms = latency_ms;
    g_metrics.record_latency(latency_ms);

    // Buffer swap for next frame
    std::swap(g_curr_gray_u8, g_prev_gray_u8);

    g_frame_count++;

    return output;
}

// =============================================================================
// YOLO Bonding Layer - Lock-Free Double-Buffered Write
// =============================================================================

void rt_core_yolo_publish(
    double timestamp_s,
    float  front_risk,
    float  left_risk,
    float  right_risk,
    float  crack_risk,
    float  min_distance_m,
    float  max_confidence,
    int    num_detections,
    int    priority_detections,
    int    num_filtered_out
) {
    int write_idx = g_yolo_write_index.load(std::memory_order_relaxed);

    YoloSnapshot* write_snap = &g_yolo_snapshots[write_idx];

    write_snap->timestamp_s = timestamp_s;
    write_snap->front_risk = front_risk;
    write_snap->left_risk = left_risk;
    write_snap->right_risk = right_risk;
    write_snap->crack_risk = crack_risk;
    write_snap->min_distance_m = min_distance_m;
    write_snap->max_confidence = max_confidence;
    write_snap->num_detections = num_detections;
    write_snap->priority_detections = priority_detections;
    write_snap->num_filtered_out = num_filtered_out;
    write_snap->valid = true;

    // Atomic swap
    int old_read_idx = g_yolo_read_index.exchange(write_idx, std::memory_order_release);
    g_yolo_write_index.store(old_read_idx, std::memory_order_relaxed);

    update_yolo_target_hz();
}

void rt_core_yolo_reset(void) {
    for (int i = 0; i < 2; ++i) {
        g_yolo_snapshots[i].valid = false;
        g_yolo_snapshots[i].timestamp_s = 0.0;
    }
}

void rt_core_set_vehicle_speed(float speed_mps) {
    g_vehicle_speed_mps.store(std::max(0.0f, speed_mps), std::memory_order_relaxed);
    update_yolo_target_hz();
}

float rt_core_yolo_get_target_hz(void) {
    return g_yolo_target_hz.load(std::memory_order_relaxed);
}

float rt_core_yolo_get_period_ms(void) {
    float hz = g_yolo_target_hz.load(std::memory_order_relaxed);
    return (hz > 0.001f) ? (1000.0f / hz) : 200.0f;
}

double rt_core_get_time_s(void) {
    return get_time_seconds();
}

// =============================================================================
// Metrics API (nth_element percentiles)
// =============================================================================

float rt_core_get_p50_latency_ms(void) {
    return g_metrics.get_percentile(0.50f);
}

float rt_core_get_p95_latency_ms(void) {
    return g_metrics.get_percentile(0.95f);
}

float rt_core_get_p99_latency_ms(void) {
    return g_metrics.get_percentile(0.99f);
}

uint64_t rt_core_get_total_frames(void) {
    return g_metrics.total_frames.load(std::memory_order_relaxed);
}

uint64_t rt_core_get_total_spike_events(void) {
    return g_metrics.spike_events.load(std::memory_order_relaxed);
}

}  // extern "C"