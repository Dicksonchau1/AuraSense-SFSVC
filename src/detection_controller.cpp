// detection_controller.cpp
#include "detection_controller.h"
#include <algorithm>
#include <cmath>

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

    // Recompute averages (using last values for simplicity)
    if (!samples_.empty()) {
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