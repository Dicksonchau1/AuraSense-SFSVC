#pragma once
#include <opencv2/core.hpp>

struct YoloResult {
    float front_risk = 0.0f;
    float left_risk = 0.0f;
    float right_risk = 0.0f;
    float crack_risk = 0.0f;
    float min_distance_m = 0.0f;
    float max_confidence = 0.0f;
    int num_detections = 0;
    int priority_detections = 0;
    int num_filtered_out = 0;
};

inline YoloResult run_yolo(const cv::Mat&, double)
{
    return {}; // Stub for now
}