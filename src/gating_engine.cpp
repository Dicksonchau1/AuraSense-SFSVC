#include "gating_engine.hpp"
#include <algorithm>

// =============================================================================
// GatingEngine Implementation - Deterministic Inference Gating Logic
// =============================================================================

GatingEngine::GatingEngine(
    float confidence_threshold,
    int   max_skip_frames,
    float max_skip_time_ms,
    float critical_crack_threshold
)
    : confidence_threshold_(confidence_threshold)
    , max_skip_frames_(max_skip_frames)
    , max_skip_time_ms_(max_skip_time_ms)
    , critical_crack_threshold_(critical_crack_threshold)
    , frames_since_last_infer_(0)
    , last_infer_time_ms_(0.0)
    , current_skip_streak_(0)
    , max_skip_streak_(0)
{
}

// =============================================================================
// Main Decision Logic (HOT PATH - must be deterministic and fast)
// =============================================================================

GatingDecision GatingEngine::decide(
    const SignatureMatch& sig_match,
    int                   frame_id,
    double                current_time_ms,
    float                 crack_score,
    bool                  force_infer
) {
    // Compute time since last inference
    double time_since_last_ms = 0.0;

    if (last_infer_time_ms_ > 0.0) {
        time_since_last_ms = current_time_ms - last_infer_time_ms_;
    } else {
        // First frame: simulate time overflow to force initial inference
        time_since_last_ms = max_skip_time_ms_.load() + 1.0;
    }

    // Increment frame counter
    frames_since_last_infer_++;

    // Load atomic config (once per call)
    const float conf_thresh = confidence_threshold_.load();
    const int   max_frames  = max_skip_frames_.load();
    const float max_time    = max_skip_time_ms_.load();
    const float crit_crack  = critical_crack_threshold_.load();

    // -------------------------------------------------------------------------
    // Decision Cascade (Priority Order - DO NOT REORDER)
    // -------------------------------------------------------------------------

    // 1. FORCED INFERENCE (highest priority)
    if (force_infer) {
        return make_decision(
            true,                          // should_infer
            1.0f,                          // confidence (forced = 100%)
            GatingReason::ForcedInfer,
            sig_match,
            time_since_last_ms,
            current_time_ms
        );
    }

    // 2. CRITICAL CRACK (safety override)
    if (crack_score >= crit_crack) {
        return make_decision(
            true,
            1.0f,
            GatingReason::CriticalCrack,
            sig_match,
            time_since_last_ms,
            current_time_ms
        );
    }

    // 3. MAX SKIP FRAMES (temporal guarantee)
    if (frames_since_last_infer_ >= max_frames) {
        return make_decision(
            true,
            0.5f,
            GatingReason::MaxSkipFrames,
            sig_match,
            time_since_last_ms,
            current_time_ms
        );
    }

    // 4. MAX SKIP TIME (real-time guarantee)
    if (time_since_last_ms >= max_time) {
        return make_decision(
            true,
            0.5f,
            GatingReason::MaxSkipTime,
            sig_match,
            time_since_last_ms,
            current_time_ms
        );
    }

    // 5. NOVEL SCENE (signature not matched)
    if (!sig_match.matched) {
        return make_decision(
            true,
            0.3f,
            GatingReason::NovelScene,
            sig_match,
            time_since_last_ms,
            current_time_ms
        );
    }

    // 6. LOW CONFIDENCE (signature matched but confidence below threshold)
    if (sig_match.confidence < conf_thresh) {
        return make_decision(
            true,
            sig_match.confidence,
            GatingReason::LowConfidence,
            sig_match,
            time_since_last_ms,
            current_time_ms
        );
    }

    // 7. HIGH CONFIDENCE SKIP (default: signature good, skip inference)
    return make_decision(
        false,                           // should_infer = false
        sig_match.confidence,
        GatingReason::HighConfidenceSkip,
        sig_match,
        time_since_last_ms,
        current_time_ms
    );
}

// =============================================================================
// Helper: Make Decision and Update State
// =============================================================================

GatingDecision GatingEngine::make_decision(
    bool                  should_infer,
    float                 confidence,
    GatingReason          reason,
    const SignatureMatch& sig_match,
    double                time_since_last_ms,
    double                current_time_ms
) {
    GatingDecision decision;

    decision.should_infer = should_infer;
    decision.confidence = confidence;
    decision.reason = reason;
    decision.signature_matched = sig_match.matched;
    decision.signature_confidence = sig_match.confidence;
    decision.frames_since_last_infer = frames_since_last_infer_;
    decision.time_since_last_infer_ms = static_cast<float>(time_since_last_ms);

    // Update state
    total_decisions_.fetch_add(1, std::memory_order_relaxed);

    if (should_infer) {
        // Reset counters on inference
        frames_since_last_infer_ = 0;
        last_infer_time_ms_ = current_time_ms;
        current_skip_streak_ = 0;

        infer_count_.fetch_add(1, std::memory_order_relaxed);
    } else {
        // Increment skip streak
        current_skip_streak_++;
        if (current_skip_streak_ > max_skip_streak_) {
            max_skip_streak_ = current_skip_streak_;
        }

        skip_count_.fetch_add(1, std::memory_order_relaxed);
    }

    // Update atomic snapshots for external readers
    current_skip_streak_snapshot_.store(current_skip_streak_,
                                         std::memory_order_relaxed);
    max_skip_streak_snapshot_.store(max_skip_streak_,
                                     std::memory_order_relaxed);

    return decision;
}

// =============================================================================
// Statistics (Thread-Safe Read)
// =============================================================================

GatingEngine::Stats GatingEngine::get_stats() const {
    Stats s;

    s.total_decisions = total_decisions_.load(std::memory_order_relaxed);
    s.infer_count = infer_count_.load(std::memory_order_relaxed);
    s.skip_count = skip_count_.load(std::memory_order_relaxed);
    s.current_skip_streak = current_skip_streak_snapshot_.load(
        std::memory_order_relaxed);
    s.max_skip_streak = max_skip_streak_snapshot_.load(
        std::memory_order_relaxed);

    if (s.total_decisions > 0) {
        s.suppression_rate = static_cast<float>(s.skip_count) /
                             static_cast<float>(s.total_decisions);
    }

    if (s.infer_count > 0) {
        s.avg_skip_streak = static_cast<float>(s.skip_count) /
                            static_cast<float>(s.infer_count);
    }

    return s;
}

// =============================================================================
// Reset (Single-Writer Only)
// =============================================================================

void GatingEngine::reset() {
    frames_since_last_infer_ = 0;
    last_infer_time_ms_ = 0.0;
    current_skip_streak_ = 0;
    max_skip_streak_ = 0;

    total_decisions_.store(0, std::memory_order_relaxed);
    infer_count_.store(0, std::memory_order_relaxed);
    skip_count_.store(0, std::memory_order_relaxed);
    current_skip_streak_snapshot_.store(0, std::memory_order_relaxed);
    max_skip_streak_snapshot_.store(0, std::memory_order_relaxed);
}

// =============================================================================
// Configuration Update (Thread-Safe)
// =============================================================================

void GatingEngine::update_config(
    float confidence_threshold,
    int   max_skip_frames,
    float max_skip_time_ms,
    float critical_crack_threshold
) {
    confidence_threshold_.store(
        std::clamp(confidence_threshold, 0.0f, 1.0f),
        std::memory_order_relaxed);

    max_skip_frames_.store(
        std::max(1, max_skip_frames),
        std::memory_order_relaxed);

    max_skip_time_ms_.store(
        std::max(1.0f, max_skip_time_ms),
        std::memory_order_relaxed);

    critical_crack_threshold_.store(
        std::clamp(critical_crack_threshold, 0.0f, 1.0f),
        std::memory_order_relaxed);
}
