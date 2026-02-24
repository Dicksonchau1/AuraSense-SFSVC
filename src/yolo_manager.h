#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <atomic>

// =============================================================================
// YoloManager — adaptive YOLO scheduling and rate control
// Controls when YOLO should run based on vehicle speed, scene activity,
// and recent detection history.
// =============================================================================
class YoloManager {
public:
    YoloManager()
        : target_hz_(10.0f)
        , min_hz_(5.0f)
        , max_hz_(20.0f)
        , last_run_time_(0.0)
        , consecutive_empty_(0)
        , consecutive_detect_(0)
    {}

    // Update target Hz based on vehicle speed (m/s)
    void update_speed(float speed_mps) {
        float hz;
        if (speed_mps > 1.0f) {
            hz = max_hz_;
        } else if (speed_mps > 0.5f) {
            hz = min_hz_ + (speed_mps - 0.5f) * 2.0f * (max_hz_ - min_hz_);
        } else {
            hz = min_hz_ + speed_mps * 2.0f * (max_hz_ - min_hz_) * 0.5f;
        }
        target_hz_ = std::clamp(hz, min_hz_, max_hz_);
    }

    // Check if enough time has elapsed to run YOLO again
    bool should_run(double now_s) const {
        if (last_run_time_ <= 0.0) return true;
        double period_s = 1.0 / static_cast<double>(target_hz_);
        return (now_s - last_run_time_) >= period_s;
    }

    // Record that YOLO ran at this timestamp
    void record_run(double now_s, int num_detections) {
        last_run_time_ = now_s;

        if (num_detections == 0) {
            consecutive_empty_++;
            consecutive_detect_ = 0;
            // Back off rate when scene is empty
            if (consecutive_empty_ > 10) {
                target_hz_ = std::max(min_hz_, target_hz_ * 0.95f);
            }
        } else {
            consecutive_detect_++;
            consecutive_empty_ = 0;
            // Increase rate when detections are frequent
            if (consecutive_detect_ > 3) {
                target_hz_ = std::min(max_hz_, target_hz_ * 1.05f);
            }
        }
    }

    float target_hz() const { return target_hz_; }
    float min_hz()    const { return min_hz_; }
    float max_hz()    const { return max_hz_; }

    void set_range(float min_hz, float max_hz) {
        min_hz_ = min_hz;
        max_hz_ = max_hz;
        target_hz_ = std::clamp(target_hz_, min_hz_, max_hz_);
    }

private:
    float  target_hz_;
    float  min_hz_;
    float  max_hz_;
    double last_run_time_;
    int    consecutive_empty_;
    int    consecutive_detect_;
};