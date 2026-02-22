#include "types.h"

#include <algorithm>
#include <chrono>
#include <cmath>

// =============================================================================
// CrackStats
// =============================================================================

CrackStats::CrackStats(double window_ms)
    : window_ms_(window_ms)
{}

void CrackStats::add_sample(double ts_ms, float crack_score)
{
    std::lock_guard<std::mutex> lk(m_);

    samples_.emplace_back(ts_ms, crack_score);
    ++total_frames_;
    if (crack_score > 0.11f)
        ++crack_frames_;

    // Remove samples outside time window
    while (!samples_.empty() &&
           (ts_ms - samples_.front().first) > window_ms_)
    {
        samples_.pop_front();
    }
}

float CrackStats::window_crack_ratio() const
{
    std::lock_guard<std::mutex> lk(m_);

    if (samples_.empty())
        return 0.0f;

    int count = 0;
    for (const auto& s : samples_) {
        if (s.second > 0.11f)
            ++count;
    }

    return static_cast<float>(count) /
           static_cast<float>(samples_.size());
}

float CrackStats::global_crack_ratio() const
{
    std::lock_guard<std::mutex> lk(m_);

    if (total_frames_ == 0)
        return 0.0f;

    return static_cast<float>(crack_frames_) /
           static_cast<float>(total_frames_);
}

// =============================================================================
// DetectionScheduler
// =============================================================================

DetectionScheduler::DetectionScheduler()
    : base_interval_ms_(200.0),
      last_detect_ms_(0.0)
{}

bool DetectionScheduler::should_detect(float crack_score)
{
    const double now =
        std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count();

    if (now - last_detect_ms_ < base_interval_ms_)
        return false;

    if (crack_score > 0.11f) {
        last_detect_ms_ = now;
        return true;
    }

    return false;
}

// =============================================================================
// DetectionController
// =============================================================================

DetectionController::DetectionController(double window_ms)
    : window_ms_(window_ms)
{}

void DetectionController::add_detection(double ts_ms,
                                        float yolo_conf,
                                        float crack_score)
{
    std::lock_guard<std::mutex> lk(m_);

    samples_.emplace_back(ts_ms, yolo_conf);

    constexpr float alpha = 0.05f;

    // EMA updates
    avg_yolo_conf_ =
        (1.0f - alpha) * avg_yolo_conf_ + alpha * yolo_conf;

    avg_crack_score_ =
        (1.0f - alpha) * avg_crack_score_ + alpha * crack_score;

    // Agreement proxy
    float diff = std::fabs(yolo_conf - crack_score);
    float agreement = std::max(0.0f, 1.0f - diff);
    avg_agreement_ =
        (1.0f - alpha) * avg_agreement_ + alpha * agreement;

    // Trim old samples
    while (!samples_.empty() &&
           (ts_ms - samples_.front().first) > window_ms_)
    {
        samples_.pop_front();
    }
}

void DetectionController::update_adaptive_thresholds()
{
    std::lock_guard<std::mutex> lk(m_);

    yolo_conf_thr_ =
        std::clamp(avg_yolo_conf_ * 0.8f, 0.2f, 0.9f);

    crack_thr_ =
        std::clamp(avg_crack_score_ * 1.2f, 0.1f, 0.8f);
}

float DetectionController::avg_yolo_conf() const
{
    return avg_yolo_conf_;
}

float DetectionController::avg_crack_score() const
{
    return avg_crack_score_;
}

float DetectionController::avg_agreement() const
{
    return avg_agreement_;
}

float DetectionController::yolo_conf_threshold() const
{
    return yolo_conf_thr_;
}

float DetectionController::crack_threshold() const
{
    return crack_thr_;
}