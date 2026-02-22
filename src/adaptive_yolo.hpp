// adaptive_yolo.hpp
#pragma once

#include <atomic>
#include <cstdint>
#include <algorithm>

// Adaptive resolution choice for YOLO lane.
// This does NOT run YOLO; it just decides which resolution profile to use
// given a scalar "activity" (here: crack_score).
struct AdaptiveYoloConfig {
    int   base_width;      // e.g. 416
    int   base_height;     // e.g. 234
    int   low_width;       // e.g. 320
    int   low_height;      // e.g. 180
    int   high_width;      // e.g. 640
    int   high_height;     // e.g. 360
    bool  enable_low;      // allow dropping resolution
    bool  enable_high;     // allow bumping resolution
    float aggressiveness;  // how aggressively to change (0.5–2.0 typical)
};

// What we return to Lane 3 to configure YOLO input.
struct AdaptiveYoloChoice {
    int   width;           // chosen width
    int   height;          // chosen height
    float crack_score;     // crack_score used for this decision
    float level;           // 0.0 = low, 0.5 = base, 1.0 = high
};

class AdaptiveYoloManager {
public:
    explicit AdaptiveYoloManager(const AdaptiveYoloConfig& cfg)
        : cfg_(cfg)
        , last_level_(0.5f)  // start at base
    {}

    // Crack score in [0, +inf) – we clamp internally.
    // Returns resolution choice for current frame.
    AdaptiveYoloChoice adjust_resolution(float crack_score) {
        // Clamp crack_score into [0,1] band for decision logic.
        float s = std::max(0.0f, std::min(crack_score, 1.0f));

        // Map crack_score to a "target level" in [0,1]:
        //  - near 0 → prefer low resolution
        //  - around 0.3–0.7 → prefer base
        //  - near 1 → prefer high resolution
        // Aggressiveness scales how quickly we move toward extremes.
        float target_level = s * cfg_.aggressiveness;
        target_level = std::max(0.0f, std::min(target_level, 1.0f));

        // Simple low-pass filter so resolution doesn't thrash frame-to-frame.
        float prev = last_level_.load(std::memory_order_relaxed);
        float alpha = 0.2f;  // smoothing factor
        float level = alpha * target_level + (1.0f - alpha) * prev;
        level = std::max(0.0f, std::min(level, 1.0f));
        last_level_.store(level, std::memory_order_relaxed);

        // Thresholds for picking low/base/high bands.
        // You can tune these numbers later.
        const float LOW_UP   = 0.25f;
        const float HIGH_LOW = 0.75f;

        int w = cfg_.base_width;
        int h = cfg_.base_height;

        if (cfg_.enable_low && level < LOW_UP) {
            w = cfg_.low_width;
            h = cfg_.low_height;
        } else if (cfg_.enable_high && level > HIGH_LOW) {
            w = cfg_.high_width;
            h = cfg_.high_height;
        } else {
            w = cfg_.base_width;
            h = cfg_.base_height;
        }

        AdaptiveYoloChoice choice;
        choice.width       = w;
        choice.height      = h;
        choice.crack_score = crack_score;
        choice.level       = level;
        return choice;
    }

    float last_level() const {
        return last_level_.load(std::memory_order_relaxed);
    }

private:
    AdaptiveYoloConfig        cfg_;
    std::atomic<float>        last_level_;
};