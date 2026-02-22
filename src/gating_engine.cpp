#include "gating_engine.hpp"
#include <algorithm>

// =====================================================
// CONSTRUCTOR
// =====================================================

GatingEngine::GatingEngine(float confidence_threshold,
                           int   max_skip_frames,
                           float max_skip_time_ms,
                           float critical_crack_threshold)
    : confidence_threshold_(confidence_threshold)
    , max_skip_frames_(max_skip_frames)
    , max_skip_time_ms_(max_skip_time_ms)
    , critical_crack_threshold_(critical_crack_threshold)
{
}

// =====================================================
// HOT PATH — SINGLE WRITER
// =====================================================

GatingDecision GatingEngine::decide(const SignatureMatch& sig_match,
                                    int                   frame_id,
                                    double                current_time_ms,
                                    float                 crack_score,
                                    bool                  force_infer)
{
    (void)frame_id; // reserved for future frame-based policies

    total_decisions_.fetch_add(1, std::memory_order_relaxed);

    const float conf_thr     = confidence_threshold_.load(std::memory_order_relaxed);
    const int   max_frames   = max_skip_frames_.load(std::memory_order_relaxed);
    const float max_time_ms  = max_skip_time_ms_.load(std::memory_order_relaxed);
    const float critical_thr = critical_crack_threshold_.load(std::memory_order_relaxed);

    // Initial overflow behavior ensures first valid frame runs inference.
    double time_since_last_ms =
        (last_infer_time_ms_ <= 0.0)
        ? max_time_ms + 1.0
        : std::max(0.0, current_time_ms - last_infer_time_ms_);

    if (force_infer)
        return make_decision(true, 1.0f, GatingReason::ForcedInfer,
                             sig_match, time_since_last_ms, current_time_ms);

    if (crack_score > critical_thr)
        return make_decision(true, 0.9f, GatingReason::CriticalCrack,
                             sig_match, time_since_last_ms, current_time_ms);

    if (frames_since_last_infer_ >= max_frames)
        return make_decision(true, 1.0f, GatingReason::MaxSkipFrames,
                             sig_match, time_since_last_ms, current_time_ms);

    if (time_since_last_ms >= max_time_ms)
        return make_decision(true, 1.0f, GatingReason::MaxSkipTime,
                             sig_match, time_since_last_ms, current_time_ms);

    if (!sig_match.matched)
        return make_decision(true, 0.8f, GatingReason::NovelScene,
                             sig_match, time_since_last_ms, current_time_ms);

    if (sig_match.confidence < conf_thr)
        return make_decision(true, 0.7f, GatingReason::LowConfidence,
                             sig_match, time_since_last_ms, current_time_ms);

    return make_decision(false,
                         sig_match.confidence,
                         GatingReason::HighConfidenceSkip,
                         sig_match,
                         time_since_last_ms,
                         current_time_ms);
}

// =====================================================
// DECISION FINALIZER
// =====================================================

GatingDecision GatingEngine::make_decision(
    bool                  should_infer,
    float                 confidence,
    GatingReason          reason,
    const SignatureMatch& sig_match,
    double                time_since_last_ms,
    double                current_time_ms)
{
    if (should_infer) {
        infer_count_.fetch_add(1, std::memory_order_relaxed);
        frames_since_last_infer_ = 0;
        last_infer_time_ms_      = current_time_ms;
        current_skip_streak_     = 0;
    } else {
        skip_count_.fetch_add(1, std::memory_order_relaxed);
        ++frames_since_last_infer_;
        ++current_skip_streak_;
        max_skip_streak_ =
            std::max(max_skip_streak_, current_skip_streak_);
    }

    // Update atomic snapshots for safe concurrent reads
    current_skip_streak_snapshot_.store(current_skip_streak_,
                                        std::memory_order_relaxed);
    max_skip_streak_snapshot_.store(max_skip_streak_,
                                    std::memory_order_relaxed);

    GatingDecision d;
    d.should_infer             = should_infer;
    d.confidence               = confidence;
    d.reason                   = reason;
    d.signature_matched        = sig_match.matched;
    d.signature_confidence     = sig_match.matched
                                   ? sig_match.confidence
                                   : 0.0f;
    d.frames_since_last_infer  = frames_since_last_infer_;
    d.time_since_last_infer_ms =
        static_cast<float>(time_since_last_ms);

    return d;
}

// =====================================================
// STATS SNAPSHOT (race-free)
// =====================================================

GatingEngine::Stats GatingEngine::get_stats() const
{
    Stats s;

    s.total_decisions = total_decisions_.load(std::memory_order_relaxed);
    s.infer_count     = infer_count_.load(std::memory_order_relaxed);
    s.skip_count      = skip_count_.load(std::memory_order_relaxed);

    s.current_skip_streak =
        current_skip_streak_snapshot_.load(std::memory_order_relaxed);
    s.max_skip_streak =
        max_skip_streak_snapshot_.load(std::memory_order_relaxed);

    if (s.total_decisions > 0) {
        s.suppression_rate =
            static_cast<float>(s.skip_count) /
            static_cast<float>(s.total_decisions);
    }

    if (s.infer_count > 0) {
        s.avg_skip_streak =
            static_cast<float>(s.skip_count) /
            static_cast<float>(s.infer_count);
    }

    return s;
}

// =====================================================
// RESET
// =====================================================

void GatingEngine::reset()
{
    frames_since_last_infer_ = 0;
    last_infer_time_ms_      = 0.0;
    current_skip_streak_     = 0;
    max_skip_streak_         = 0;

    total_decisions_.store(0);
    infer_count_.store(0);
    skip_count_.store(0);

    current_skip_streak_snapshot_.store(0);
    max_skip_streak_snapshot_.store(0);
}

// =====================================================
// CONFIG UPDATE
// =====================================================

void GatingEngine::update_config(float confidence_threshold,
                                 int   max_skip_frames,
                                 float max_skip_time_ms,
                                 float critical_crack_threshold)
{
    confidence_threshold_.store(confidence_threshold);
    max_skip_frames_.store(max_skip_frames);
    max_skip_time_ms_.store(max_skip_time_ms);
    critical_crack_threshold_.store(critical_crack_threshold);
}