// detection_controller.h
#pragma once

#include <deque>
#include <mutex>

class DetectionController {
public:
    explicit DetectionController(double window_ms = 5000.0);
    void add_detection(double ts_ms, float yolo_conf, float crack_score);
    void update_adaptive_thresholds();
    float avg_yolo_conf() const;
    float avg_crack_score() const;
    float avg_agreement() const;
    float yolo_conf_threshold() const;
    float crack_threshold() const;

private:
    double window_ms_;
    mutable std::mutex m_;
    std::deque<std::pair<double, float>> samples_;
    float avg_yolo_conf_ = 0.0f;
    float avg_crack_score_ = 0.0f;
    float avg_agreement_ = 0.0f;
    float yolo_conf_thr_ = 0.40f;
    float crack_thr_ = 0.50f;
};