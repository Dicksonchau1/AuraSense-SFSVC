// =============================================================================
// crack_statistics.cpp — Implementations for CrackStats and DetectionScheduler
// (declared in types.h)
// =============================================================================

#include "types.h"
#include <chrono>
#include <cmath>
#include <algorithm>

static double cs_now_ms() {
    return std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

// =============================================================================
// CrackStats
// =============================================================================

CrackStats::CrackStats(double window_ms)
    : window_ms_(window_ms)
{}

void CrackStats::add_sample(double ts_ms, float crack_score) {
    std::lock_guard<std::mutex> lock(m_);

    samples_.emplace_back(ts_ms, crack_score);
    total_frames_++;
    if (crack_score > 0.3f) crack_frames_++;

    // Prune old samples
    while (!samples_.empty() &&
           (ts_ms - samples_.front().first) > window_ms_) {
        samples_.pop_front();
    }
}

float CrackStats::window_crack_ratio() const {
    std::lock_guard<std::mutex> lock(m_);
    if (samples_.empty()) return 0.0f;

    int count = 0;
    for (const auto& s : samples_) {
        if (s.second > 0.3f) count++;
    }
    return static_cast<float>(count) /
           static_cast<float>(samples_.size());
}

float CrackStats::global_crack_ratio() const {
    std::lock_guard<std::mutex> lock(m_);
    if (total_frames_ == 0) return 0.0f;
    return static_cast<float>(crack_frames_) /
           static_cast<float>(total_frames_);
}

// =============================================================================
// DetectionScheduler
// =============================================================================

DetectionScheduler::DetectionScheduler()
    : base_interval_ms_(100.0)
    , last_detect_ms_(0.0)
{}

bool DetectionScheduler::should_detect(float crack_score) {
    double now = cs_now_ms();

    double interval = base_interval_ms_;

    // Shorten interval when crack score is high
    if (crack_score > 0.8f) {
        interval *= 0.25;
    } else if (crack_score > 0.5f) {
        interval *= 0.5;
    } else if (crack_score > 0.2f) {
        interval *= 0.75;
    }

    if (now - last_detect_ms_ >= interval) {
        last_detect_ms_ = now;
        return true;
    }
    return false;
}