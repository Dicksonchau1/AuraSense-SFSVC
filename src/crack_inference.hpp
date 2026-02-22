#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

struct CrackInferenceOutput {
    float fused_probability;     // final intelligent crack probability
    float stability;             // temporal stability metric
    float noise_index;           // noise estimation
    bool  should_schedule_yolo;  // scheduling hint
};

class CrackInferenceEngine {
public:
    CrackInferenceEngine()
        : ema_(0.0f),
          variance_(0.0f),
          persistence_(0.0f),
          prev_sparsity_(0.0f),
          scene_lum_ema_(0.0f),
          initialized_(false)
    {}

    CrackInferenceOutput update(float raw_crack,
                                float sparsity,
                                float luminance,
                                float signature_conf)
    {
        CrackInferenceOutput out{};

        if (!initialized_) {
            ema_ = raw_crack;
            scene_lum_ema_ = luminance;
            initialized_ = true;
        }

        // -----------------------------
        // 1. Temporal smoothing
        // -----------------------------
        constexpr float alpha = 0.15f;
        ema_ = alpha * raw_crack + (1.0f - alpha) * ema_;

        variance_ = 0.1f * std::fabs(raw_crack - ema_) +
                    0.9f * variance_;

        float stable_crack = std::max(0.0f,
                                      ema_ - 1.5f * variance_);

        // -----------------------------
        // 2. Noise estimation
        // -----------------------------
        float noise_index =
            sparsity / (std::max(0.01f, luminance));

        float noise_suppression =
            std::clamp(1.0f - noise_index, 0.2f, 1.0f);

        // -----------------------------
        // 3. Motion coherence
        // -----------------------------
        float coherence =
            1.0f - std::fabs(sparsity - prev_sparsity_);

        coherence = std::clamp(coherence, 0.0f, 1.0f);
        prev_sparsity_ = sparsity;

        // -----------------------------
        // 4. Persistence model
        // -----------------------------
        if (stable_crack > 0.05f)
            persistence_ = std::min(1.0f, persistence_ + 0.1f);
        else
            persistence_ *= 0.95f;

        // -----------------------------
        // 5. Scene stability
        // -----------------------------
        scene_lum_ema_ =
            0.05f * luminance +
            0.95f * scene_lum_ema_;

        float scene_change =
            std::fabs(luminance - scene_lum_ema_);

        float scene_stability =
            std::clamp(1.0f - scene_change * 2.0f,
                       0.0f, 1.0f);

        // -----------------------------
        // 6. Bayesian-like fusion
        // -----------------------------
        float fused =
            0.40f * stable_crack +
            0.20f * coherence +
            0.20f * persistence_ +
            0.20f * signature_conf;

        fused *= noise_suppression;
        fused *= scene_stability;

        fused = std::clamp(fused, 0.0f, 1.0f);

        out.fused_probability = fused;
        out.stability         = persistence_;
        out.noise_index       = noise_index;

        // YOLO scheduling heuristic
        out.should_schedule_yolo =
            (fused > 0.20f && persistence_ > 0.25f);

        return out;
    }

private:
    float ema_;
    float variance_;
    float persistence_;
    float prev_sparsity_;
    float scene_lum_ema_;
    bool  initialized_;
};