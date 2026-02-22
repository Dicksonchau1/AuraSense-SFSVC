#pragma once

#include <cstdint>
#include <atomic>
#include "types.h"

// =====================================================
// Decision Reason (strongly typed, zero allocation)
// =====================================================

enum class GatingReason : uint8_t {
    ForcedInfer,
    CriticalCrack,
    MaxSkipFrames,
    MaxSkipTime,
    NovelScene,
    LowConfidence,
    HighConfidenceSkip
};

// =====================================================
// OUTPUT STRUCT
// No heap allocations.
// =====================================================

struct GatingDecision {
    bool          should_infer             = false;
    float         confidence               = 0.0f;
    GatingReason  reason                   = GatingReason::HighConfidenceSkip;
    bool          signature_matched        = false;
    float         signature_confidence     = 0.0f;
    int           frames_since_last_infer  = 0;
    float         time_since_last_infer_ms = 0.0f;
};

// =====================================================
// GATING ENGINE
//
// Thread Contract:
// - decide() MUST be called from exactly one thread per instance.
// - get_stats() may be called concurrently.
// - Config updates are atomic.
// - No heap allocations in hot path.
// =====================================================

class GatingEngine {
public:

    struct Stats {
        uint64_t total_decisions     = 0;
        uint64_t infer_count         = 0;
        uint64_t skip_count          = 0;
        float    suppression_rate    = 0.0f;
        float    avg_skip_streak     = 0.0f;
        int      current_skip_streak = 0;
        int      max_skip_streak     = 0;
    };

    GatingEngine(float confidence_threshold,
                 int   max_skip_frames,
                 float max_skip_time_ms,
                 float critical_crack_threshold = 0.6f);

    // -------------------------------------------------
    // Hot path — priority-ordered decision cascade
    //
    // Decision order is intentional and must not change:
    // 1. Forced inference
    // 2. Critical crack
    // 3. Max frame skip
    // 4. Max time skip
    // 5. Novel scene
    // 6. Low confidence
    // 7. High confidence skip
    //
    // Initial behavior:
    // If no inference has yet occurred (last_infer_time_ms_ == 0),
    // we simulate a time overflow so the first valid frame forces inference.
    // -------------------------------------------------
    GatingDecision decide(const SignatureMatch& sig_match,
                          int                   frame_id,
                          double                current_time_ms,
                          float                 crack_score,
                          bool                  force_infer);

    Stats get_stats() const;
    void  reset();

    void update_config(float confidence_threshold,
                       int   max_skip_frames,
                       float max_skip_time_ms,
                       float critical_crack_threshold);

private:

    GatingDecision make_decision(bool                  should_infer,
                                 float                 confidence,
                                 GatingReason          reason,
                                 const SignatureMatch& sig_match,
                                 double                time_since_last_ms,
                                 double                current_time_ms);

private:

    // ---- CONFIG (atomic) ----
    std::atomic<float> confidence_threshold_;
    std::atomic<int>   max_skip_frames_;
    std::atomic<float> max_skip_time_ms_;
    std::atomic<float> critical_crack_threshold_;

    // ---- Runtime state (single writer only) ----
    int      frames_since_last_infer_ = 0;
    double   last_infer_time_ms_      = 0.0;
    int      current_skip_streak_     = 0;
    int      max_skip_streak_         = 0;

    // ---- Atomic stats snapshot (safe external reads) ----
    std::atomic<uint64_t> total_decisions_{0};
    std::atomic<uint64_t> infer_count_{0};
    std::atomic<uint64_t> skip_count_{0};

    std::atomic<int>      current_skip_streak_snapshot_{0};
    std::atomic<int>      max_skip_streak_snapshot_{0};
};