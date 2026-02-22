// yolo_manager.h
#pragma once

#include <opencv2/opencv.hpp>
#include "yolo_bridge.h" // <--- INCLUDE THIS TO GET THE YoloResult DEFINITION

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

    // This can now use YoloResult without redefining it
    YoloResult run(const cv::Mat& frame, double timestamp_ms) {
        YoloResult r;
        // Placeholder inference logic
        return r;
    }

private:
    AdaptiveYoloConfig config_;
};