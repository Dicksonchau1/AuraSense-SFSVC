#pragma once

#include <opencv2/core.hpp>
#include <vector>
#include <string>

// =============================================================================
// YoloDetection — single bounding box detection
// =============================================================================
struct YoloDetection {
    float x1 = 0.0f;
    float y1 = 0.0f;
    float x2 = 0.0f;
    float y2 = 0.0f;
    float confidence = 0.0f;
    int   class_id   = -1;
};

// =============================================================================
// YoloResult — aggregate result from one YOLO inference pass
// =============================================================================
struct YoloResult {
    float front_risk        = 0.0f;
    float left_risk         = 0.0f;
    float right_risk        = 0.0f;
    float crack_risk        = 0.0f;
    float min_distance_m    = 0.0f;
    float max_confidence    = 0.0f;
    int   num_detections    = 0;
    int   priority_detections = 0;
    int   num_filtered_out  = 0;
    double latency_ms       = 0.0;
    std::vector<YoloDetection> detections;
};

// =============================================================================
// Public API
// =============================================================================

// Load YOLOv8 ONNX model. Returns true on success.
bool yolo_init(const std::string& model_path, int input_size = 640);

// Run inference on a BGR frame. Returns spatial risk assessment.
YoloResult yolo_detect(const cv::Mat& frame,
                       float conf_threshold = 0.35f,
                       float nms_threshold  = 0.45f);

// Release model resources.
void yolo_shutdown();