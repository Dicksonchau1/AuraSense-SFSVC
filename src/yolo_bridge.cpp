// =============================================================================
// yolo_bridge.cpp — Full OpenCV DNN YOLOv8 ONNX Implementation
//
// Features:
//   - YOLOv8 ONNX inference via cv::dnn
//   - Letterbox preprocessing
//   - [1,84,N] output parsing (4 bbox + 80 COCO classes)
//   - NMS with IoU thresholding
//   - Spatial bucketing: front/left/right/crack risk zones
//   - Distance estimation from bbox size
//   - Priority class detection (person, vehicle)
//
// Dependencies: OpenCV 4.x (core, dnn, imgproc)
// =============================================================================

#include "yolo_bridge.h"

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>

#include <chrono>
#include <algorithm>
#include <cmath>
#include <mutex>
#include <cstdio>

namespace {

// --- Module state ---
cv::dnn::Net g_net;
bool         g_initialized = false;
int          g_input_size  = 640;
std::mutex   g_init_mutex;

static constexpr int NUM_CLASSES = 80;

// --- Priority classes (COCO): vehicles, persons, large obstacles ---
bool is_priority_class(int class_id) {
    // person=0, bicycle=1, car=2, motorcycle=3, bus=5, train=6, truck=7
    return class_id == 0 || class_id == 2 || class_id == 3 ||
           class_id == 5 || class_id == 6 || class_id == 7;
}

// --- Letterbox resize for YOLOv8 ---
cv::Mat letterbox(const cv::Mat& src, int target_size,
                  float& scale, int& pad_x, int& pad_y)
{
    int h = src.rows;
    int w = src.cols;

    scale = std::min(static_cast<float>(target_size) / h,
                     static_cast<float>(target_size) / w);

    int new_w = static_cast<int>(w * scale);
    int new_h = static_cast<int>(h * scale);

    pad_x = (target_size - new_w) / 2;
    pad_y = (target_size - new_h) / 2;

    cv::Mat resized;
    cv::resize(src, resized, cv::Size(new_w, new_h), 0, 0, cv::INTER_AREA);

    cv::Mat padded(target_size, target_size, CV_8UC3,
                   cv::Scalar(114, 114, 114));
    resized.copyTo(padded(cv::Rect(pad_x, pad_y, new_w, new_h)));

    return padded;
}

// --- Non-Maximum Suppression ---
void nms(std::vector<YoloDetection>& dets, float iou_threshold) {
    std::sort(dets.begin(), dets.end(),
              [](const YoloDetection& a, const YoloDetection& b) {
                  return a.confidence > b.confidence;
              });

    std::vector<bool> suppressed(dets.size(), false);

    for (size_t i = 0; i < dets.size(); i++) {
        if (suppressed[i]) continue;

        for (size_t j = i + 1; j < dets.size(); j++) {
            if (suppressed[j]) continue;

            float ix1 = std::max(dets[i].x1, dets[j].x1);
            float iy1 = std::max(dets[i].y1, dets[j].y1);
            float ix2 = std::min(dets[i].x2, dets[j].x2);
            float iy2 = std::min(dets[i].y2, dets[j].y2);

            float inter_w = std::max(0.0f, ix2 - ix1);
            float inter_h = std::max(0.0f, iy2 - iy1);
            float inter_area = inter_w * inter_h;

            float area_i = (dets[i].x2 - dets[i].x1) *
                           (dets[i].y2 - dets[i].y1);
            float area_j = (dets[j].x2 - dets[j].x1) *
                           (dets[j].y2 - dets[j].y1);

            float iou = inter_area /
                        (area_i + area_j - inter_area + 1e-6f);

            if (iou > iou_threshold) {
                suppressed[j] = true;
            }
        }
    }

    std::vector<YoloDetection> kept;
    kept.reserve(dets.size());
    for (size_t i = 0; i < dets.size(); i++) {
        if (!suppressed[i]) kept.push_back(dets[i]);
    }
    dets = std::move(kept);
}

// --- Spatial risk bucketing ---
void compute_spatial_risks(const std::vector<YoloDetection>& dets,
                           int frame_w, int frame_h,
                           float& front_risk, float& left_risk,
                           float& right_risk, float& crack_risk)
{
    front_risk = 0.0f;
    left_risk  = 0.0f;
    right_risk = 0.0f;
    crack_risk = 0.0f;

    float third_w = static_cast<float>(frame_w) / 3.0f;
    float bottom_third_y = static_cast<float>(frame_h) * 2.0f / 3.0f;

    for (const auto& d : dets) {
        float cx = (d.x1 + d.x2) * 0.5f;
        float cy = (d.y1 + d.y2) * 0.5f;
        float conf = d.confidence;

        // Front zone: center third, bottom half
        if (cx >= third_w && cx < 2.0f * third_w &&
            cy >= static_cast<float>(frame_h) * 0.5f) {
            front_risk = std::max(front_risk, conf);
        }

        // Left zone: left third
        if (cx < third_w) {
            left_risk = std::max(left_risk, conf);
        }

        // Right zone: right third
        if (cx >= 2.0f * third_w) {
            right_risk = std::max(right_risk, conf);
        }

        // Bottom third → ground-level crack risk
        if (cy >= bottom_third_y) {
            crack_risk = std::max(crack_risk, conf * 0.5f);
        }
    }
}

// --- Distance estimation from bbox height ---
float estimate_distance(const YoloDetection& det, int frame_h) {
    float bbox_h = det.y2 - det.y1;
    if (bbox_h < 1.0f) return 100.0f;

    float ratio = bbox_h / static_cast<float>(frame_h);
    // Empirical inverse mapping
    float distance = 2.0f / (ratio + 0.1f);
    return std::clamp(distance, 0.5f, 100.0f);
}

} // anonymous namespace

// =============================================================================
// Public API
// =============================================================================

bool yolo_init(const std::string& model_path, int input_size) {
    std::lock_guard<std::mutex> lock(g_init_mutex);

    if (g_initialized) return true;

    try {
        g_net = cv::dnn::readNetFromONNX(model_path);

        // Use OpenCV backend + CPU target (portable)
        g_net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        g_net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

        g_input_size  = input_size;
        g_initialized = true;
        return true;
    } catch (const cv::Exception& e) {
        std::fprintf(stderr, "[YOLO] Failed to load model '%s': %s\n",
                     model_path.c_str(), e.what());
        g_initialized = false;
        return false;
    }
}

YoloResult yolo_detect(const cv::Mat& frame,
                       float conf_threshold,
                       float nms_threshold)
{
    YoloResult result;

    auto t0 = std::chrono::steady_clock::now();

    if (!g_initialized || frame.empty()) {
        auto t1 = std::chrono::steady_clock::now();
        result.latency_ms = std::chrono::duration<double, std::milli>(
            t1 - t0).count();
        return result;
    }

    int frame_w = frame.cols;
    int frame_h = frame.rows;

    // Letterbox resize
    float scale;
    int pad_x, pad_y;
    cv::Mat letterboxed = letterbox(frame, g_input_size, scale, pad_x, pad_y);

    // Create blob: [1, 3, H, W], normalized to [0,1]
    cv::Mat blob = cv::dnn::blobFromImage(
        letterboxed, 1.0 / 255.0,
        cv::Size(g_input_size, g_input_size),
        cv::Scalar(), true, false);

    g_net.setInput(blob);

    // Forward pass
    std::vector<cv::Mat> outputs;
    g_net.forward(outputs, g_net.getUnconnectedOutLayersNames());

    if (outputs.empty()) {
        auto t1 = std::chrono::steady_clock::now();
        result.latency_ms = std::chrono::duration<double, std::milli>(
            t1 - t0).count();
        return result;
    }

    // Parse YOLOv8 output: shape [1, 84, N]
    // Layout in memory: feature f for detection d → data[f * N + d]
    cv::Mat output = outputs[0];

    if (output.dims < 2) {
        auto t1 = std::chrono::steady_clock::now();
        result.latency_ms = std::chrono::duration<double, std::milli>(
            t1 - t0).count();
        return result;
    }

    int num_features   = output.size[1];  // 84 = 4 bbox + 80 classes
    int num_candidates = (output.dims == 3) ? output.size[2] : output.size[1];

    if (output.dims == 2) {
        // [N, 84] format — some ONNX exports use this
        num_candidates = output.size[0];
        num_features   = output.size[1];
    }

    float* raw = reinterpret_cast<float*>(output.data);

    std::vector<YoloDetection> detections;
    int filtered_count = 0;

    bool transposed = (output.dims == 3); // [1, 84, N] needs column-major access

    for (int i = 0; i < num_candidates; i++) {
        float cx, cy, bw, bh;
        float max_score = 0.0f;
        int best_class = -1;

        if (transposed) {
            // [1, 84, N]: feature f, detection i → raw[f * N + i]
            int N = num_candidates;
            cx = raw[0 * N + i];
            cy = raw[1 * N + i];
            bw = raw[2 * N + i];
            bh = raw[3 * N + i];

            for (int c = 0; c < NUM_CLASSES && (4 + c) < num_features; c++) {
                float score = raw[(4 + c) * N + i];
                if (score > max_score) {
                    max_score = score;
                    best_class = c;
                }
            }
        } else {
            // [N, 84]: detection i, feature f → raw[i * 84 + f]
            float* row = raw + i * num_features;
            cx = row[0];
            cy = row[1];
            bw = row[2];
            bh = row[3];

            for (int c = 0; c < NUM_CLASSES && (4 + c) < num_features; c++) {
                float score = row[4 + c];
                if (score > max_score) {
                    max_score = score;
                    best_class = c;
                }
            }
        }

        if (max_score < conf_threshold) {
            filtered_count++;
            continue;
        }

        // Convert from letterbox coords to original frame coords
        float x1 = (cx - bw * 0.5f - pad_x) / scale;
        float y1 = (cy - bh * 0.5f - pad_y) / scale;
        float x2 = (cx + bw * 0.5f - pad_x) / scale;
        float y2 = (cy + bh * 0.5f - pad_y) / scale;

        // Clamp to frame bounds
        x1 = std::clamp(x1, 0.0f, static_cast<float>(frame_w));
        y1 = std::clamp(y1, 0.0f, static_cast<float>(frame_h));
        x2 = std::clamp(x2, 0.0f, static_cast<float>(frame_w));
        y2 = std::clamp(y2, 0.0f, static_cast<float>(frame_h));

        // Validate bbox area
        float area_ratio = ((x2 - x1) * (y2 - y1)) /
                           (static_cast<float>(frame_w) *
                            static_cast<float>(frame_h));
        if (area_ratio < 0.001f || area_ratio > 0.80f) {
            filtered_count++;
            continue;
        }

        YoloDetection det;
        det.x1 = x1;
        det.y1 = y1;
        det.x2 = x2;
        det.y2 = y2;
        det.confidence = max_score;
        det.class_id   = best_class;

        detections.push_back(det);
    }

    result.num_filtered_out = filtered_count;

    // Apply NMS
    nms(detections, nms_threshold);

    result.detections     = detections;
    result.num_detections = static_cast<int>(detections.size());

    // Spatial risk assessment
    compute_spatial_risks(detections, frame_w, frame_h,
                          result.front_risk, result.left_risk,
                          result.right_risk, result.crack_risk);

    // Max confidence, priority detections, min distance
    result.min_distance_m = 100.0f;
    for (const auto& d : detections) {
        result.max_confidence = std::max(result.max_confidence, d.confidence);

        if (is_priority_class(d.class_id)) {
            result.priority_detections++;
        }

        float dist = estimate_distance(d, frame_h);
        result.min_distance_m = std::min(result.min_distance_m, dist);
    }

    if (detections.empty()) {
        result.min_distance_m = 0.0f;
    }

    auto t1 = std::chrono::steady_clock::now();
    result.latency_ms = std::chrono::duration<double, std::milli>(
        t1 - t0).count();

    return result;
}

void yolo_shutdown() {
    std::lock_guard<std::mutex> lock(g_init_mutex);
    if (g_initialized) {
        g_net = cv::dnn::Net();
        g_initialized = false;
    }
}