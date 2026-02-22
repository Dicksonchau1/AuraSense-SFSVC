#pragma once

#include <opencv2/opencv.hpp>

struct YoloResult {
    float front_risk       = 0.0f;
    float left_risk        = 0.0f;
    float right_risk       = 0.0f;
    float crack_risk       = 0.0f;
    float min_distance_m   = 0.0f;
    float max_confidence   = 0.0f;
    int   num_detections   = 0;
    int   priority_detections = 0;
    int   num_filtered_out = 0;
};

struct AdaptiveYoloConfig {
    int  base_w;
    int  base_h;
    int  min_w;
    int  min_h;
    int  max_w;
    int  max_h;
    bool enable_dynamic_scaling;
    bool enable_conf_thresholding;
    float scaling_factor;
};

class YoloManager {
public:
    explicit YoloManager(const AdaptiveYoloConfig& cfg)
        : config_(cfg) {}

    ~YoloManager() = default;

    YoloResult run(const cv::Mat& frame, double timestamp_ms) {
        YoloResult r;
        // Placeholder inference logic
        return r;
    }

private:
    AdaptiveYoloConfig config_;
};