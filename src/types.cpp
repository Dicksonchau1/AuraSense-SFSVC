#include "types.h"
#include <algorithm>
#include <cmath>
#include <chrono>

// =============================================================================
// CrackStats Implementation
// =============================================================================

CrackStats::CrackStats(double window_ms)
    : window_ms_(window_ms)
    , total_frames_(0)
    , crack_frames_(0)
{
}

void CrackStats::add_sample(double ts_ms, float crack_score) {
    std::lock_guard<std::mutex> lock(m_);

    total_frames_++;

    if (crack_score > 0.05f) {
        crack_frames_++;
    }

    // Add sample to window
    samples_.emplace_back(ts_ms, crack_score);

    // Prune old samples outside window
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
        if (s.second > 0.05f) count++;
    }

    return static_cast<float>(count) / static_cast<float>(samples_.size());
}

float CrackStats::global_crack_ratio() const {
    std::lock_guard<std::mutex> lock(m_);

    if (total_frames_ == 0) return 0.0f;

    return static_cast<float>(crack_frames_) /
           static_cast<float>(total_frames_);
}

// =============================================================================
// DetectionController Implementation
// =============================================================================

DetectionController::DetectionController(double window_ms)
    : window_ms_(window_ms)
    , avg_yolo_conf_(0.0f)
    , avg_crack_score_(0.0f)
    , avg_agreement_(0.0f)
    , yolo_conf_thr_(0.40f)
    , crack_thr_(0.50f)
{
}

void DetectionController::add_detection(
    double ts_ms,
    float yolo_conf,
    float crack_score
) {
    std::lock_guard<std::mutex> lock(m_);

    // Add to window (store combined metric)
    float combined = (yolo_conf + crack_score) * 0.5f;
    samples_.emplace_back(ts_ms, combined);

    // Prune old samples
    while (!samples_.empty() &&
           (ts_ms - samples_.front().first) > window_ms_) {
        samples_.pop_front();
    }

    // Recompute averages
    if (!samples_.empty()) {
        float sum = 0.0f;
        for (const auto& s : samples_) {
            sum += s.second;
        }
        float avg = sum / static_cast<float>(samples_.size());

        avg_yolo_conf_ = yolo_conf;  // Last value
        avg_crack_score_ = crack_score;  // Last value

        // Agreement: how close are yolo and crack scores?
        avg_agreement_ = 1.0f - std::abs(yolo_conf - crack_score);
    }
}

void DetectionController::update_adaptive_thresholds() {
    std::lock_guard<std::mutex> lock(m_);

    // Adaptive thresholding based on recent statistics
    // If average is high, we can be more selective
    // If average is low, we should be more sensitive

    if (avg_crack_score_ > 0.6f) {
        // High crack activity: increase threshold to reduce false positives
        crack_thr_ = std::min(0.70f, crack_thr_ + 0.02f);
    } else if (avg_crack_score_ < 0.3f) {
        // Low crack activity: decrease threshold to increase sensitivity
        crack_thr_ = std::max(0.30f, crack_thr_ - 0.02f);
    }

    if (avg_yolo_conf_ > 0.6f) {
        yolo_conf_thr_ = std::min(0.60f, yolo_conf_thr_ + 0.02f);
    } else if (avg_yolo_conf_ < 0.3f) {
        yolo_conf_thr_ = std::max(0.25f, yolo_conf_thr_ - 0.02f);
    }
}

float DetectionController::avg_yolo_conf() const {
    std::lock_guard<std::mutex> lock(m_);
    return avg_yolo_conf_;
}

float DetectionController::avg_crack_score() const {
    std::lock_guard<std::mutex> lock(m_);
    return avg_crack_score_;
}

float DetectionController::avg_agreement() const {
    std::lock_guard<std::mutex> lock(m_);
    return avg_agreement_;
}

float DetectionController::yolo_conf_threshold() const {
    std::lock_guard<std::mutex> lock(m_);
    return yolo_conf_thr_;
}

float DetectionController::crack_threshold() const {
    std::lock_guard<std::mutex> lock(m_);
    return crack_thr_;
}

// =============================================================================
// DetectionScheduler Implementation
// =============================================================================

DetectionScheduler::DetectionScheduler()
    : base_interval_ms_(200.0)  // 5 Hz default
    , last_detect_ms_(0.0)
{
}

bool DetectionScheduler::should_detect(float crack_score) {
    // Get current time in milliseconds
    auto now = std::chrono::steady_clock::now();
    double now_ms = std::chrono::duration<double, std::milli>(
        now.time_since_epoch()).count();

    // First detection always runs
    if (last_detect_ms_ == 0.0) {
        last_detect_ms_ = now_ms;
        return true;
    }

    double elapsed = now_ms - last_detect_ms_;

    // Adaptive interval based on crack score
    // Higher crack score → detect more frequently
    double interval = base_interval_ms_;

    if (crack_score > 0.6f) {
        interval = 100.0;  // 10 Hz for critical cracks
    } else if (crack_score > 0.3f) {
        interval = 150.0;  // ~6.7 Hz for moderate cracks
    } else {
        interval = 200.0;  // 5 Hz for low/no cracks
    }

    if (elapsed >= interval) {
        last_detect_ms_ = now_ms;
        return true;
    }

    return false;
}
