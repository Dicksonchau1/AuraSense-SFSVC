#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <string>
#include <vector>
#include <shared_mutex>   // ✅ for reader/writer locking

#include "types.h"   // SignatureMatch

// =============================================================
// High-Resolution Time Helper
// =============================================================
inline double sb_now_seconds()
{
    using clock = std::chrono::steady_clock;
    return std::chrono::duration<double>(
        clock::now().time_since_epoch()).count();
}

// =============================================================
// L2 Distance (hot path — inline)
// =============================================================
inline float sb_l2_distance(const std::vector<float>& a,
                            const std::vector<float>& b)
{
    const size_t n = std::min(a.size(), b.size());
    float s = 0.0f;
    for (size_t i = 0; i < n; ++i) {
        float d = a[i] - b[i];
        s += d * d;
    }
    return std::sqrt(s);
}

// =============================================================
// Safe Unit Normalization
// =============================================================
inline void sb_safe_unit_norm(std::vector<float>& v,
                              float eps = 1e-8f)
{
    float n2 = 0.0f;
    for (float x : v) n2 += x * x;
    float n = std::sqrt(n2);
    if (n < eps)
        std::fill(v.begin(), v.end(), 0.0f);
    else {
        float inv = 1.0f / n;
        for (float& x : v) x *= inv;
    }
}

// =============================================================
// Match Result
// =============================================================
struct MatchResult
{
    bool  matched        = false;
    int   signature_id   = -1;
    float distance       = std::numeric_limits<float>::infinity();
    float confidence     = 0.0f;
    float match_time_ms  = 0.0f;
    float d_struct       = 0.0f;
    float avg_luminance  = 0.0f;
};

// =============================================================
// Signature Object
// =============================================================
struct Signature
{
    int signature_id = -1;

    std::vector<float> gabor_fingerprint;
    std::vector<float> semantic_profile;
    std::vector<float> context_vector;
    std::vector<float> motion_signature;

    double first_seen       = 0.0;
    double last_seen        = 0.0;
    int    occurrence_count = 1;

    float  persistence_trace = 0.0f;
    double last_match_time   = 0.0;

    float  historical_risk   = 0.5f;
    float  false_alarm_rate  = 0.0f;
    double refractory_until  = 0.0;

    float  avg_luminance     = 0.0f;
};

// =============================================================
// SignatureBank
// Vector storage for cache-friendly iteration on hot path.
// Threading model:
//   - Lane2 (writer): find_match_full(), register_signature(), feedback(), clear()
//   - Lane2 (writer, internal): prune()
//   - Lane3 + UI (readers): compute_confidence(), to_signature_match(), size()/stats
//   - Writer uses unique_lock, readers use shared_lock.
// =============================================================
class SignatureBank
{
public:

    SignatureBank(
        int    max_signatures        = 1000,
        float  match_threshold       = 0.30f,
        double forgetting_period     = 3600.0,
        float  trace_tau             = 4.0f,
        float  trace_increment       = 1.0f,
        float  trace_cap             = 10.0f,
        float  adapt_rate            = 0.05f,
        float  adapt_min_confidence  = 0.6f,
        float  w_gabor               = 0.5f,
        float  w_semantic            = 0.3f,
        float  w_context             = 0.1f,
        float  w_motion              = 0.1f,
        double refractory_sec        = 0.0
    )
        : max_signatures_(max_signatures)
        , match_threshold_(match_threshold)
        , forgetting_period_(forgetting_period)
        , trace_tau_(trace_tau > 1e-3f ? trace_tau : 1e-3f)
        , trace_increment_(trace_increment)
        , trace_cap_(trace_cap)
        , adapt_rate_(std::clamp(adapt_rate, 0.0f, 1.0f))
        , adapt_min_confidence_(adapt_min_confidence)
        , refractory_sec_(std::max(0.0, refractory_sec))
        , next_id_(0)
        , total_matches_(0)
        , total_registrations_(0)
        , total_prunes_(0)
    {
        float wsum   = w_gabor + w_semantic + w_context + w_motion;
        w_gabor_     = w_gabor    / wsum;
        w_semantic_  = w_semantic / wsum;
        w_context_   = w_context  / wsum;
        w_motion_    = w_motion   / wsum;

        signatures_.reserve(max_signatures_);
    }

    // =========================================================
    // Full Multi-Vector Matching (Lane 2 WRITER)
    // =========================================================
    MatchResult find_match_full(
        const std::vector<float>& gabor_fp,
        const std::vector<float>& semantic_prof,
        const std::vector<float>& context_vec,
        const std::vector<float>& motion_sig,
        float current_luminance = 0.0f)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);

        MatchResult res;
        res.avg_luminance = current_luminance;

        if (signatures_.empty())
            return res;

        double t0  = sb_now_seconds();
        double now = t0;

        int   best_index   = -1;
        float best_dist    = std::numeric_limits<float>::infinity();
        float best_dstruct = 0.0f;

        for (size_t i = 0; i < signatures_.size(); ++i)
        {
            Signature& sig = signatures_[i];
            if (now < sig.refractory_until) continue;

            float d_struct =
                w_gabor_    * sb_l2_distance(gabor_fp,      sig.gabor_fingerprint) +
                w_semantic_ * sb_l2_distance(semantic_prof, sig.semantic_profile);

            float d_context =
                w_context_  * sb_l2_distance(context_vec,   sig.context_vector) +
                w_motion_   * sb_l2_distance(motion_sig,    sig.motion_signature);

            float d = d_struct + d_context;

            if (d < best_dist) {
                best_dist    = d;
                best_dstruct = d_struct;
                best_index   = static_cast<int>(i);
            }
        }

        if (best_index >= 0 && best_dist <= match_threshold_)
        {
            Signature& sig = signatures_[best_index];

            sig.last_seen = now;
            sig.occurrence_count++;
            update_trace(sig, now);

            float raw_conf = std::exp(-best_dist / match_threshold_);

            if (raw_conf >= adapt_min_confidence_ && adapt_rate_ > 0.0f) {
                float lr = adapt_rate_ * raw_conf;
                ema_update(sig.gabor_fingerprint, gabor_fp,      lr);
                ema_update(sig.semantic_profile,  semantic_prof, lr);
                ema_update(sig.context_vector,    context_vec,   lr);
                ema_update(sig.motion_signature,  motion_sig,    lr);
            }

            if (refractory_sec_ > 0.0)
                sig.refractory_until = now + refractory_sec_;

            total_matches_++;

            res.matched      = true;
            res.signature_id = sig.signature_id;
            res.distance     = best_dist;
            res.d_struct     = best_dstruct;
        }

        res.match_time_ms =
            static_cast<float>((sb_now_seconds() - t0) * 1000.0);

        return res;
    }

    // =========================================================
    // Confidence — READER (Lane 3 / UI)
    // =========================================================
    float compute_confidence(const MatchResult& match,
                             float growth_risk  = 0.0f,
                             float growth_accel = 0.0f) const
    {
        if (!match.matched) return 0.0f;

        std::shared_lock<std::shared_mutex> lock(mutex_);

        const Signature* sig = find_by_id(match.signature_id);
        if (!sig) return 0.0f;

        float f_struct = 1.0f, f_sim = 1.0f;
        if (match_threshold_ > 1e-6f) {
            f_struct = std::exp(-2.0f * match.d_struct / match_threshold_);
            f_sim    = std::exp(-1.0f * match.distance / match_threshold_);
        }

        float f = 0.7f * f_struct + 0.3f * f_sim;
        float F = std::min(sig->persistence_trace / trace_cap_, 1.0f);
        float Q = 1.0f - sig->false_alarm_rate;
        float R = sig->historical_risk;

        float growth_factor =
            1.0f + 0.5f * growth_risk +
            0.3f * std::max(0.0f, growth_accel);

        return std::clamp(f * F * Q * (1.0f - R) * growth_factor, 0.0f, 1.0f);
    }

    // =========================================================
    // Register Signature (WRITER)
    // =========================================================
    int register_signature(
        std::vector<float> gabor_fp,
        std::vector<float> semantic_prof,
        std::vector<float> context_vec,
        std::vector<float> motion_sig,
        float initial_risk   = 0.1f,
        float avg_luminance  = 0.0f)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);

        if ((int)signatures_.size() >= max_signatures_)
            prune();    // assumes lock is held

        const int sid = next_id_++;
        double now    = sb_now_seconds();

        sb_safe_unit_norm(gabor_fp);
        sb_safe_unit_norm(semantic_prof);
        sb_safe_unit_norm(context_vec);
        sb_safe_unit_norm(motion_sig);

        Signature sig;
        sig.signature_id      = sid;
        sig.gabor_fingerprint = std::move(gabor_fp);
        sig.semantic_profile  = std::move(semantic_prof);
        sig.context_vector    = std::move(context_vec);
        sig.motion_signature  = std::move(motion_sig);
        sig.first_seen        = now;
        sig.last_seen         = now;
        sig.occurrence_count  = 1;
        sig.persistence_trace = 0.0f;
        sig.last_match_time   = 0.0;
        sig.historical_risk   = std::clamp(initial_risk, 0.0f, 1.0f);
        sig.false_alarm_rate  = 0.0f;
        sig.refractory_until  = 0.0;
        sig.avg_luminance     = avg_luminance;

        signatures_.push_back(std::move(sig));
        total_registrations_++;

        return sid;
    }

    // =========================================================
    // Sufficiency Gate (Lane 2 / Reader semantics)
    // =========================================================
    bool is_sufficient(
        const MatchResult& match,
        float confidence_threshold   = 0.88f,
        float familiarity_bonus      = 0.08f,
        bool  require_stable         = true,
        float stable_trace_threshold = 3.0f,
        float max_risk_to_ignore     = 0.8f,
        float max_luminance_jump     = 0.15f,
        float lum_conf_boost         = 0.12f) const
    {
        if (!match.matched) return false;

        std::shared_lock<std::shared_mutex> lock(mutex_);

        const Signature* sig = find_by_id(match.signature_id);
        if (!sig) return false;

        float base_conf     = compute_confidence_locked(*sig, match);
        float familiarity   = std::min(1.0f, sig->occurrence_count / 20.0f);
        float adjusted_conf = base_conf + familiarity * familiarity_bonus;

        float lum_jump          = std::fabs(match.avg_luminance - sig->avg_luminance);
        float effective_thresh  = confidence_threshold;
        if (lum_jump > max_luminance_jump)
            effective_thresh = std::min(1.0f, confidence_threshold + lum_conf_boost);

        if (sig->historical_risk > max_risk_to_ignore) return false;
        if (require_stable && sig->persistence_trace < stable_trace_threshold) return false;

        return adjusted_conf >= effective_thresh;
    }

    // =========================================================
    // Feedback — online risk / false alarm learning (WRITER)
    // =========================================================
    void feedback(int signature_id, const std::string& outcome)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);

        Signature* sig = find_by_id(signature_id);
        if (!sig) return;

        if (outcome == "confirmed") {
            sig->historical_risk =
                std::min(1.0f, sig->historical_risk + 0.1f);
        } else if (outcome == "false_alarm") {
            sig->false_alarm_rate =
                std::min(1.0f, sig->false_alarm_rate + 0.1f);
            sig->historical_risk =
                std::max(0.0f, sig->historical_risk - 0.05f);
        } else if (outcome == "alerted") {
            sig->historical_risk =
                std::min(1.0f, sig->historical_risk + 0.02f);
        }
    }

    // =========================================================
    // Lane 3 Adapter (READER)
    // =========================================================
    SignatureMatch to_signature_match(const MatchResult& m,
                                      float growth_risk  = 0.0f,
                                      float growth_accel = 0.0f) const
    {
        SignatureMatch out;
        out.matched = m.matched;
        out.id      = m.signature_id;

        // This calls compute_confidence(), which takes a shared_lock.
        out.confidence = compute_confidence(m, growth_risk, growth_accel);
        return out;
    }

    // =========================================================
    // Utilities
    // =========================================================
    void clear()
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);

        signatures_.clear();
        next_id_             = 0;
        total_matches_       = 0;
        total_registrations_ = 0;
        total_prunes_        = 0;
    }

    int size() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return (int)signatures_.size();
    }
    int total_matches() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return (int)total_matches_;
    }
    int total_registrations() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return (int)total_registrations_;
    }
    int total_prunes() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return (int)total_prunes_;
    }

private:

    // Helper: same formula as compute_confidence(), but assumes lock is held
    float compute_confidence_locked(const Signature& sig,
                                    const MatchResult& match,
                                    float growth_risk  = 0.0f,
                                    float growth_accel = 0.0f) const
    {
        float f_struct = 1.0f, f_sim = 1.0f;
        if (match_threshold_ > 1e-6f) {
            f_struct = std::exp(-2.0f * match.d_struct / match_threshold_);
            f_sim    = std::exp(-1.0f * match.distance / match_threshold_);
        }

        float f = 0.7f * f_struct + 0.3f * f_sim;
        float F = std::min(sig.persistence_trace / trace_cap_, 1.0f);
        float Q = 1.0f - sig.false_alarm_rate;
        float R = sig.historical_risk;

        float growth_factor =
            1.0f + 0.5f * growth_risk +
            0.3f * std::max(0.0f, growth_accel);

        return std::clamp(f * F * Q * (1.0f - R) * growth_factor, 0.0f, 1.0f);
    }

    // Linear search by ID — callers must hold appropriate lock
    Signature* find_by_id(int id)
    {
        for (auto& s : signatures_)
            if (s.signature_id == id) return &s;
        return nullptr;
    }

    const Signature* find_by_id(int id) const
    {
        for (const auto& s : signatures_)
            if (s.signature_id == id) return &s;
        return nullptr;
    }

    void decay_trace(Signature& sig, double now) const
    {
        if (sig.last_match_time <= 0.0) return;
        double dt = std::max(0.0, now - sig.last_match_time);
        sig.persistence_trace *=
            std::exp(-static_cast<float>(dt) / trace_tau_);
    }

    void update_trace(Signature& sig, double now) const
    {
        decay_trace(sig, now);
        sig.persistence_trace =
            std::min(trace_cap_, sig.persistence_trace + trace_increment_);
        sig.last_match_time = now;
    }

    void ema_update(std::vector<float>&       old_v,
                    const std::vector<float>& new_v,
                    float                     lr) const
    {
        const size_t n = std::min(old_v.size(), new_v.size());
        for (size_t i = 0; i < n; ++i)
            old_v[i] = (1.0f - lr) * old_v[i] + lr * new_v[i];
        sb_safe_unit_norm(old_v);
    }

    // prune() assumes caller holds unique_lock on mutex_
    void prune()
    {
        if (signatures_.empty()) return;

        struct Scored { float value; size_t index; };
        std::vector<Scored> scored;
        scored.reserve(signatures_.size());

        double now = sb_now_seconds();

        for (size_t i = 0; i < signatures_.size(); ++i)
        {
            Signature& sig = signatures_[i];
            float recency    = std::exp(-(float)(now - sig.last_seen) /
                                        (float)forgetting_period_);
            float familiarity = std::min(1.0f, sig.occurrence_count / 10.0f);
            float value       = recency * familiarity * (sig.historical_risk + 0.1f);
            scored.push_back({value, i});
        }

        std::sort(scored.begin(), scored.end(),
                  [](const Scored& a, const Scored& b)
                  { return a.value < b.value; });

        int remove_n = std::max(1, (int)scored.size() / 10);

        std::vector<size_t> to_remove;
        to_remove.reserve(remove_n);
        for (int i = 0; i < remove_n; ++i)
            to_remove.push_back(scored[i].index);

        std::sort(to_remove.begin(), to_remove.end(),
                  [](size_t a, size_t b) { return a > b; });

        for (size_t idx : to_remove) {
            signatures_.erase(signatures_.begin() + idx);
            total_prunes_++;
        }
    }

    // ---- Config ----
    int    max_signatures_;
    float  match_threshold_;
    double forgetting_period_;
    float  trace_tau_;
    float  trace_increment_;
    float  trace_cap_;
    float  adapt_rate_;
    float  adapt_min_confidence_;
    double refractory_sec_;
    float  w_gabor_;
    float  w_semantic_;
    float  w_context_;
    float  w_motion_;

    // ---- Storage ----
    std::vector<Signature> signatures_;   // vector: cache-friendly iteration

    // ---- Counters ----
    int      next_id_;
    uint64_t total_matches_;
    uint64_t total_registrations_;
    uint64_t total_prunes_;

    // ---- Concurrency ----
    mutable std::shared_mutex mutex_;
};