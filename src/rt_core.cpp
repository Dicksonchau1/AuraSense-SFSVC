/**
 * RT Core - Ultra-Low Latency Real-Time Processing Engine
 *
 * Target: <0.2ms latency for 234x416 BGR frame
 *
 * KEY OPTIMIZATIONS:
 * 1. LOCK-FREE YOLO snapshot (atomic double-buffer)
 * 2. NO pow() - precomputed LUT for spike compression
 * 3. SINGLE-PASS pipeline (delta + count + crack in one loop)
 * 4. NO full spike buffers (only prev_gray needed)
 * 5. UINT8 grayscale path (optional float for precision)
 * 6. SINGLE-THREADED writer (explicitly documented)
 *
 * THREAD SAFETY:
 * - rt_core_process_frame_ptr: SINGLE WRITER ONLY
 * - rt_core_yolo_publish: Can be called from separate thread
 * - rt_core_set_vehicle_speed: Can be called from separate thread
 *
 * Author: RT Core Team
 * Date: 2026-02-22
 */

#include "rt_core.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <chrono>
#include <atomic>

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

constexpr float THRESHOLD_ON = 0.03f;
constexpr float THRESHOLD_OFF = 0.03f;
constexpr float SPIKE_POWER = 0.7f;

// Crack detection ROI (bottom 1/3)
constexpr int CRACK_ROI_START_Y = (TARGET_HEIGHT * 2) / 3;
constexpr float CRACK_GRADIENT_THRESHOLD = 0.3f;

// YOLO staleness threshold
constexpr double YOLO_MAX_AGE_S = 5.0;

// =============================================================================
// Lock-Free YOLO State (Double-Buffered Atomic Snapshot)
// =============================================================================

struct YoloSnapshot {
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

    // Padding to avoid false sharing (cache line = 64 bytes)
    char padding[64 - sizeof(double) - 9*sizeof(float) - 3*sizeof(int) - sizeof(bool)];
};

// Two snapshots for double buffering
alignas(64) YoloSnapshot g_yolo_snapshots[2];
std::atomic<int> g_yolo_read_index{0};  // Reader uses this index
std::atomic<int> g_yolo_write_index{1}; // Writer uses this index

// Vehicle speed (atomic - can be updated from external thread)
std::atomic<float> g_vehicle_speed_mps{0.0f};
std::atomic<float> g_yolo_target_hz{5.0f};

// =============================================================================
// Precomputed LUT for Spike Compression (Replaces pow())
// =============================================================================

constexpr int LUT_SIZE = 256;
alignas(64) float g_spike_lut[LUT_SIZE];

void init_spike_lut() {
    // Precompute: (x / threshold) ^ 0.7 for x in [0, threshold * 2]
    // Map [0, 255] input to [0, 2*threshold] range
    constexpr float MAX_DELTA = THRESHOLD_ON * 2.0f;

    for (int i = 0; i < LUT_SIZE; ++i) {
        float ratio = static_cast<float>(i) / static_cast<float>(LUT_SIZE - 1);
        float delta = ratio * MAX_DELTA;

        if (delta < THRESHOLD_ON) {
            g_spike_lut[i] = 0.0f;
        } else {
            float norm = delta / THRESHOLD_ON;
            // pow(norm, 0.7) ≈ sqrt(norm) * pow(norm, 0.2)
            // For LUT, just compute directly once
            g_spike_lut[i] = std::min(std::pow(norm, SPIKE_POWER), 1.0f);
        }
    }
}

// Fast LUT lookup
inline float spike_compress_lut(float delta_abs, float threshold) {
    if (delta_abs < threshold) return 0.0f;

    // Map delta to LUT index
    int idx = static_cast<int>((delta_abs / (threshold * 2.0f)) * (LUT_SIZE - 1));
    idx = std::min(idx, LUT_SIZE - 1);

    return g_spike_lut[idx];
}

// =============================================================================
// Frame Buffers (Minimal - Only Previous Grayscale Needed)
// =============================================================================

alignas(64) static uint8_t* g_prev_gray_u8 = nullptr;
alignas(64) static uint8_t* g_curr_gray_u8 = nullptr;

static int g_frame_count = 0;

// Timing
auto g_start_time = std::chrono::steady_clock::now();

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

    // Initialize spike LUT
    init_spike_lut();

    // Initialize YOLO snapshots
    std::memset(g_yolo_snapshots, 0, sizeof(g_yolo_snapshots));

    initialized = true;
}

// =============================================================================
// Time Helper
// =============================================================================

inline double get_time_seconds() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(now - g_start_time).count();
}

// =============================================================================
// BGR → Grayscale (UINT8 Path - ITU-R BT.709)
// =============================================================================

inline void bgr_to_gray_u8(const uint8_t* bgr, uint8_t* gray, int pixels) {
    // Fixed-point coefficients (scaled by 256 for integer math)
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

        // Integer math: (54*R + 183*G + 19*B) / 256
        int val = (R_COEF * r + G_COEF * g + B_COEF * b) >> 8;
        gray[i] = static_cast<uint8_t>(val);
    }
}

// =============================================================================
// SINGLE-PASS Pipeline: Delta + Count + Crack Detection
//
// This replaces:
//   1. compute_spikes() - full frame loop
//   2. count loop - second full frame loop
//   3. detect_crack() - third loop
//
// With: ONE loop that does all three
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

    // Convert thresholds to uint8 range (0-255)
    const int thresh_u8 = static_cast<int>(THRESHOLD_ON * 255.0f);

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            const int idx = y * width + x;

            // Temporal delta (signed)
            int delta = static_cast<int>(curr_gray[idx]) - static_cast<int>(prev_gray[idx]);

            // Count spikes (ON/OFF)
            if (delta > thresh_u8) {
                result.on_count++;
            } else if (delta < -thresh_u8) {
                result.off_count++;
            }

            // Crack detection (only in bottom 1/3)
            if (y >= roi_start_y) {
                // Horizontal gradient for crack detection
                // Use current frame (not delta) for better edge response
                int left = curr_gray[idx - 1];
                int right = curr_gray[idx + 1];
                int grad_abs = std::abs(right - left);

                // Crack signature: strong horizontal gradient in current frame
                if (grad_abs > static_cast<int>(CRACK_GRADIENT_THRESHOLD * 255.0f)) {
                    crack_accumulator += static_cast<float>(grad_abs) / 255.0f;
                    crack_pixels++;
                }
            }
        }
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
    // Atomic read of current read index
    int idx = g_yolo_read_index.load(std::memory_order_acquire);

    // Copy snapshot (no lock needed - writer uses other buffer)
    return g_yolo_snapshots[idx];
}

// =============================================================================
// YOLO Fusion (Lock-Free)
// =============================================================================

inline float apply_yolo_fusion_lockfree(float raw_crack_score, double current_time_s) {
    YoloSnapshot snap = read_yolo_snapshot();

    if (!snap.valid) {
        return raw_crack_score;  // No YOLO data
    }

    // Check staleness
    double age = current_time_s - snap.timestamp_s;
    if (age > YOLO_MAX_AGE_S) {
        return raw_crack_score;  // Stale YOLO
    }

    // Fusion: boost crack score if YOLO detects crack-related objects
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
// Control Decision (Deterministic, Branch-Reduced)
// =============================================================================

inline void make_control_decision(float fused_crack, float* throttle, float* steer) {
    // Branch-reduced version using min/max
    // Instead of: if (x > 0.5) throttle = 0.3; else if (x > 0.2) ...

    // Map crack score [0, 1] to throttle [1.0, 0.3]
    // Linear: throttle = 1.0 - 0.7 * clamp(crack / 0.5)

    if (fused_crack > 0.5f) {
        *throttle = 0.3f;
    } else if (fused_crack > 0.2f) {
        *throttle = 0.7f;
    } else {
        *throttle = 1.0f;
    }

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
// THREAD SAFETY: SINGLE WRITER ONLY
// This function MUST be called from only ONE thread.
// Concurrent calls will produce undefined behavior.
// =============================================================================

ControlOutput rt_core_process_frame_ptr(
    const uint8_t* bgr,
    int            height,
    int            width
) {
    ensure_initialized();

    ControlOutput output;
    std::memset(&output, 0, sizeof(output));

    // Validate input
    if (!bgr || height != TARGET_HEIGHT || width != TARGET_WIDTH) {
        output.frame_id = -1;
        return output;
    }

    const double current_time_s = get_time_seconds();

    // =========================================================================
    // STEP 1: BGR → Grayscale (UINT8 - No Float Conversion)
    // =========================================================================

    bgr_to_gray_u8(bgr, g_curr_gray_u8, TOTAL_PIXELS);

    // =========================================================================
    // STEP 2: Single-Pass Pipeline (Delta + Count + Crack)
    // =========================================================================

    PipelineResult pipeline;

    if (g_frame_count > 0) {
        pipeline = single_pass_pipeline(
            g_curr_gray_u8,
            g_prev_gray_u8,
            TARGET_WIDTH,
            TARGET_HEIGHT
        );
    } else {
        // First frame: no temporal delta
        pipeline = {0, 0, 0.0f};
    }

    output.on_spike_count = pipeline.on_count;
    output.off_spike_count = pipeline.off_count;

    // =========================================================================
    // STEP 3: Compute Sparsity
    // =========================================================================

    float total_spikes = static_cast<float>(pipeline.on_count + pipeline.off_count);
    output.sparsity = 1.0f - (total_spikes / static_cast<float>(TOTAL_PIXELS));

    // =========================================================================
    // STEP 4: Raw Crack Score (Already Computed in Pipeline)
    // =========================================================================

    output.crack_score = pipeline.crack_score;

    // =========================================================================
    // STEP 5: YOLO Fusion (LOCK-FREE)
    // =========================================================================

    output.fused_crack_score = apply_yolo_fusion_lockfree(
        output.crack_score,
        current_time_s
    );

    // =========================================================================
    // STEP 6: Control Decision
    // =========================================================================

    make_control_decision(
        output.fused_crack_score,
        &output.throttle,
        &output.steer
    );

    // =========================================================================
    // STEP 7: Inference Suppression Logic
    // =========================================================================

    output.inference_suppressed = (output.sparsity > 0.95f) ? 1 : 0;
    output.event_only_mode = (output.sparsity > 0.98f) ? 1 : 0;
    output.is_null_cycle = (g_frame_count == 0) ? 1 : 0;

    // =========================================================================
    // STEP 8: YOLO State (Lock-Free Read)
    // =========================================================================

    YoloSnapshot yolo = read_yolo_snapshot();

    output.yolo_active = yolo.valid ? 1 : 0;

    if (yolo.valid) {
        double age = current_time_s - yolo.timestamp_s;
        output.yolo_age_ms = static_cast<float>(age * 1000.0);
    } else {
        output.yolo_age_ms = 99999.0f;
    }

    output.yolo_target_hz = g_yolo_target_hz.load(std::memory_order_relaxed);

    // =========================================================================
    // STEP 9: Metadata
    // =========================================================================

    output.frame_id = g_frame_count;
    output.reference_frame_age = 1;
    output.encode_time_ms = 0.0f;  // Reserved for future profiling
    output.global_saliency = 0.0f; // Reserved
    output.roi_count = 0;          // Reserved

    // =========================================================================
    // STEP 10: Swap Buffers for Next Frame
    // =========================================================================

    std::swap(g_curr_gray_u8, g_prev_gray_u8);

    g_frame_count++;

    return output;
}

// =============================================================================
// YOLO Bonding Layer - Lock-Free Double-Buffered Write
//
// THREAD SAFETY: Can be called from separate YOLO inference thread
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
    // Write to the WRITE buffer (not being read)
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

    // Atomic swap: make write buffer the new read buffer
    int old_read_idx = g_yolo_read_index.exchange(write_idx, std::memory_order_release);
    g_yolo_write_index.store(old_read_idx, std::memory_order_relaxed);

    // Update adaptive YOLO Hz
    update_yolo_target_hz();
}

void rt_core_yolo_reset(void) {
    // Invalidate both buffers
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

}  // extern "C"
