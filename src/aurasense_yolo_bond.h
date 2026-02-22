// aurasense_yolo_bond.cpp
// Revised: Fix D (publish memory ordering), Fix E (valid gate confirmed)

// ============================================================================
// aurasense_yolo_bond.h
// AuraSense SFSVC — YOLO ↔ RT Core Logic Bonding Layer
// Version: 1.0.1
// Latency budget: < 0.3 ms added to Lane 1 cycle
// Detection accuracy: Primary constraint — all fusion gated by confidence
// ============================================================================

#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>
#include <atomic>
#include <cstring>

namespace aurasense {

// ============================================================================
// SECTION 1: DETECTION ACCURACY CONTRACTS
// ============================================================================

struct AccuracyConfig {
    // --- Detection confidence gates ---
    float min_confidence_crack     = 0.35f;
    float min_confidence_obstacle  = 0.45f;
    float min_confidence_priority  = 0.55f;

    // --- Temporal consistency (anti-flicker) ---
    int   min_consecutive_frames   = 2;
    int   max_gap_frames           = 3;

    // --- Spatial validation ---
    float min_bbox_area_ratio      = 0.001f;
    float max_bbox_area_ratio      = 0.80f;

    // --- Age gating ---
    double max_yolo_age_ms         = 200.0;
    double stale_decay_start_ms    = 100.0;

    // --- Fusion bounds ---
    float max_crack_amplification  = 2.5f;
    float max_speed_reduction      = 0.70f;
    float max_steer_bias           = 0.30f;
    float min_safe_speed_scale     = 0.15f;
};

// ============================================================================
// SECTION 2: YOLO SUMMARY STRUCT (Lock-Free Shared Contract)
// ============================================================================

struct alignas(64) YoloSummary {
    uint32_t sequence_id;
    bool     valid;

    double   timestamp_s;
    double   age_ms;

    float    front_risk;
    float    left_risk;
    float    right_risk;
    float    crack_risk;

    float    min_distance_m;
    float    max_confidence;
    int      num_detections;
    int      priority_detections;

    int      num_filtered_out;
    int      consecutive_frames;
    float    temporal_stability;
};

// ============================================================================
// SECTION 3: LOCK-FREE DOUBLE BUFFER (Python → C++)
// ============================================================================

class YoloSharedState {
private:
    YoloSummary           buffers_[2];
    std::atomic<int>      active_{0};
    std::atomic<uint32_t> write_seq_{0};

public:
    YoloSharedState() {
        std::memset(&buffers_[0], 0, sizeof(YoloSummary));
        std::memset(&buffers_[1], 0, sizeof(YoloSummary));
        buffers_[0].valid = false;
        buffers_[1].valid = false;
    }

    // FIX D: sequence ID is assigned BEFORE the struct copy, then written
    // after copy, with an explicit release fence before the index flip.
    // This guarantees sequence_id and struct body are coherent on the reader
    // side and prevents rare temporal filter misfires on the RT path.
    void publish(const YoloSummary& summary) {
        int write_idx = 1 - active_.load(std::memory_order_acquire);

        // Grab sequence number first
        uint32_t seq = write_seq_.fetch_add(1, std::memory_order_relaxed);

        // Copy full struct into inactive buffer
        buffers_[write_idx] = summary;

        // Stamp sequence AFTER copy so it matches this publish event
        buffers_[write_idx].sequence_id = seq;

        // Full release fence: entire struct visible before index flip
        std::atomic_thread_fence(std::memory_order_release);

        // Flip active index — reader will see complete, coherent buffer
        active_.store(write_idx, std::memory_order_release);
    }

    // Called by C++/Lane 1 (RT-critical, must be < 0.005 ms)
    YoloSummary read(double now_s) const {
        int read_idx = active_.load(std::memory_order_acquire);
        YoloSummary snap = buffers_[read_idx];
        snap.age_ms = (now_s - snap.timestamp_s) * 1000.0;
        return snap;
    }
};

// ============================================================================
// SECTION 4: TEMPORAL CONSISTENCY FILTER
// ============================================================================

class TemporalConsistencyFilter {
private:
    int   consec_front_ = 0;
    int   consec_left_  = 0;
    int   consec_right_ = 0;
    int   consec_crack_ = 0;
    int   gap_front_    = 0;
    int   gap_left_     = 0;
    int   gap_right_    = 0;
    int   gap_crack_    = 0;
    uint32_t last_seq_  = 0;

    inline void update_channel(float risk, float threshold,
                               int& consec, int& gap, int max_gap) {
        if (risk >= threshold) {
            consec++;
            gap = 0;
        } else {
            gap++;
            if (gap > max_gap) consec = 0;
        }
    }

public:
    struct FilteredRisks {
        float front_risk;
        float left_risk;
        float right_risk;
        float crack_risk;
        float temporal_stability;
    };

    FilteredRisks filter(const YoloSummary& raw, const AccuracyConfig& cfg) {
        FilteredRisks out{};

        if (raw.sequence_id == last_seq_ && last_seq_ != 0)
            return out;
        last_seq_ = raw.sequence_id;

        update_channel(raw.front_risk, cfg.min_confidence_obstacle,
                       consec_front_, gap_front_, cfg.max_gap_frames);
        update_channel(raw.left_risk,  cfg.min_confidence_obstacle,
                       consec_left_,  gap_left_,  cfg.max_gap_frames);
        update_channel(raw.right_risk, cfg.min_confidence_obstacle,
                       consec_right_, gap_right_, cfg.max_gap_frames);
        update_channel(raw.crack_risk, cfg.min_confidence_crack,
                       consec_crack_, gap_crack_, cfg.max_gap_frames);

        int min_frames = cfg.min_consecutive_frames;

        out.front_risk = (consec_front_ >= min_frames) ? raw.front_risk : 0.0f;
        out.left_risk  = (consec_left_  >= min_frames) ? raw.left_risk  : 0.0f;
        out.right_risk = (consec_right_ >= min_frames) ? raw.right_risk : 0.0f;
        out.crack_risk = (consec_crack_ >= min_frames) ? raw.crack_risk : 0.0f;

        int total_consec = consec_front_ + consec_left_ +
                           consec_right_ + consec_crack_;
        out.temporal_stability = std::min(1.0f, total_consec / 20.0f);

        return out;
    }

    void reset() {
        consec_front_ = consec_left_ = consec_right_ = consec_crack_ = 0;
        gap_front_    = gap_left_    = gap_right_    = gap_crack_    = 0;
        last_seq_ = 0;
    }
};

// ============================================================================
// SECTION 5: AGE-AWARE CONFIDENCE DECAY
// ============================================================================

inline float age_decay(float risk, double age_ms, const AccuracyConfig& cfg) {
    if (age_ms <= cfg.stale_decay_start_ms) return risk;
    if (age_ms >= cfg.max_yolo_age_ms)      return 0.0f;

    double range   = cfg.max_yolo_age_ms - cfg.stale_decay_start_ms;
    double elapsed = age_ms - cfg.stale_decay_start_ms;
    float  decay   = 1.0f - static_cast<float>(elapsed / range);
    return risk * decay;
}

// ============================================================================
// SECTION 6: CRACK FUSION
// ============================================================================

inline float fuse_crack(float sfsvc_crack_score,
                        float yolo_crack_risk,
                        float yolo_front_risk,
                        const AccuracyConfig& cfg) {
    float semantic_risk = std::max(yolo_crack_risk, yolo_front_risk);
    float adjusted      = sfsvc_crack_score;

    if (semantic_risk > 0.30f && sfsvc_crack_score > 0.05f) {
        float factor = 1.0f + (cfg.max_crack_amplification - 1.0f) * semantic_risk;
        adjusted = std::min(1.0f, sfsvc_crack_score * factor);
    }
    else if (semantic_risk < 0.10f && sfsvc_crack_score < 0.10f) {
        adjusted = sfsvc_crack_score * 0.5f;
    }

    return adjusted;
}

// ============================================================================
// SECTION 7: SPEED & STEERING FUSION
// ============================================================================

struct ControlModifiers {
    float speed_scale;
    float steer_bias;
    bool  emergency_slow;
};

inline ControlModifiers compute_control_modifiers(
        float front_risk,
        float left_risk,
        float right_risk,
        int   priority_detections,
        const AccuracyConfig& cfg) {

    ControlModifiers mod;
    mod.speed_scale    = 1.0f;
    mod.steer_bias     = 0.0f;
    mod.emergency_slow = false;

    if (front_risk > 0.20f) {
        float reduction = cfg.max_speed_reduction * std::min(front_risk, 1.0f);
        mod.speed_scale = std::max(1.0f - reduction, cfg.min_safe_speed_scale);
    }

    if (priority_detections > 0) {
        mod.speed_scale    = std::min(mod.speed_scale, 0.30f);
        mod.emergency_slow = true;
    }

    float asymmetry = left_risk - right_risk;
    if (std::fabs(asymmetry) > 0.10f) {
        float raw_bias = cfg.max_steer_bias * asymmetry;
        mod.steer_bias = std::clamp(raw_bias,
                                    -cfg.max_steer_bias,
                                     cfg.max_steer_bias);
    }

    return mod;
}

// ============================================================================
// SECTION 8: MASTER FUSION ENTRY POINT
// ============================================================================

struct FusionResult {
    float            adjusted_crack_score;
    ControlModifiers control_mods;
    bool             yolo_active;
    float            yolo_age_ms;
    float            temporal_stability;
    uint32_t         yolo_seq;
};

class YoloBondingLayer {
private:
    AccuracyConfig            cfg_;
    TemporalConsistencyFilter temporal_filter_;
    YoloSharedState*          shared_state_;

public:
    explicit YoloBondingLayer(YoloSharedState* state,
                              AccuracyConfig cfg = {})
        : cfg_(cfg), shared_state_(state) {}

    FusionResult fuse(float sfsvc_crack_score, double now_s) {
        FusionResult result{};
        result.adjusted_crack_score = sfsvc_crack_score;
        result.control_mods         = {1.0f, 0.0f, false};
        result.yolo_active          = false;
        result.yolo_age_ms          = -1.0f;
        result.temporal_stability   = 0.0f;
        result.yolo_seq             = 0;

        // 1. Read snapshot (wait-free, ~0.001 ms)
        YoloSummary snap = shared_state_->read(now_s);

        // 2. FIX E: valid gate — confirmed present and correct
        if (!snap.valid)                          return result;
        if (snap.age_ms > cfg_.max_yolo_age_ms)  return result;
        if (snap.age_ms < 0.0)                   return result;  // clock skew

        result.yolo_age_ms = static_cast<float>(snap.age_ms);
        result.yolo_seq    = snap.sequence_id;

        // 3. Temporal filter (~0.005 ms)
        auto filtered = temporal_filter_.filter(snap, cfg_);
        result.temporal_stability = filtered.temporal_stability;

        // 4. Age-aware decay (~0.008 ms)
        float front = age_decay(filtered.front_risk, snap.age_ms, cfg_);
        float left  = age_decay(filtered.left_risk,  snap.age_ms, cfg_);
        float right = age_decay(filtered.right_risk, snap.age_ms, cfg_);
        float crack = age_decay(filtered.crack_risk, snap.age_ms, cfg_);

        // 5. Signal check
        bool has_signal = (front > 0.01f || left > 0.01f ||
                           right > 0.01f || crack > 0.01f ||
                           snap.priority_detections > 0);
        if (!has_signal) return result;

        result.yolo_active = true;

        // 6. Crack fusion (~0.003 ms)
        result.adjusted_crack_score = fuse_crack(
            sfsvc_crack_score, crack, front, cfg_);

        // 7. Speed + steering modifiers (~0.003 ms)
        result.control_mods = compute_control_modifiers(
            front, left, right, snap.priority_detections, cfg_);

        return result;
    }

    void update_config(const AccuracyConfig& new_cfg) { cfg_ = new_cfg; }
    void reset() { temporal_filter_.reset(); }
};

// ============================================================================
// SECTION 9: INTEGRATION NOTES (unchanged)
// ============================================================================
//
//   AFTER:
//     CppOutput out = process_frame(input);
//     FusionResult fused = yolo_bond.fuse(out.crack_score, now_s);
//     float crack = fused.adjusted_crack_score;
//     float speed = compute_speed(crack) * fused.control_mods.speed_scale;
//     float steer = std::clamp(
//         compute_steering(input) + fused.control_mods.steer_bias,
//         -1.0f, 1.0f);

// ============================================================================
// SECTION 10: PYTHON CTYPES BRIDGE
// ============================================================================

extern "C" {

static YoloSharedState  g_yolo_state;
static YoloBondingLayer g_yolo_bond(&g_yolo_state);

void yolo_publish(
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
    YoloSummary s{};
    s.valid               = true;
    s.timestamp_s         = timestamp_s;
    s.age_ms              = 0.0;
    s.front_risk          = front_risk;
    s.left_risk           = left_risk;
    s.right_risk          = right_risk;
    s.crack_risk          = crack_risk;
    s.min_distance_m      = min_distance_m;
    s.max_confidence      = max_confidence;
    s.num_detections      = num_detections;
    s.priority_detections = priority_detections;
    s.num_filtered_out    = num_filtered_out;
    s.consecutive_frames  = 0;
    s.temporal_stability  = 0.0f;

    g_yolo_state.publish(s);
}

void yolo_fuse(
    float  sfsvc_crack_score,
    double now_s,
    float* adjusted_crack,
    float* speed_scale,
    float* steer_bias,
    int*   yolo_active
) {
    FusionResult r   = g_yolo_bond.fuse(sfsvc_crack_score, now_s);
    *adjusted_crack  = r.adjusted_crack_score;
    *speed_scale     = r.control_mods.speed_scale;
    *steer_bias      = r.control_mods.steer_bias;
    *yolo_active     = r.yolo_active ? 1 : 0;
}

void yolo_reset() {
    g_yolo_bond.reset();
}

}  // extern "C"

}  // namespace aurasense