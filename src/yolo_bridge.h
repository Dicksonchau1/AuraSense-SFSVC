+  1 #pragma once
+  2 
+  3 #include <opencv2/core.hpp>
+  4 #include <vector>
+  5 #include <string>
+  6 
+  7 // =============================================================================
+  8 // YoloDetection — single bounding box detection
+  9 // =============================================================================
+ 10 struct YoloDetection {
+ 11     float x1 = 0.0f;
+ 12     float y1 = 0.0f;
+ 13     float x2 = 0.0f;
+ 14     float y2 = 0.0f;
+ 15     float confidence = 0.0f;
+ 16     int   class_id   = -1;
+ 17 };
+ 18 
+ 19 // =============================================================================
+ 20 // YoloResult — aggregate result from one YOLO inference pass
+ 21 // =============================================================================
+ 22 struct YoloResult {
+ 23     float front_risk        = 0.0f;
+ 24     float left_risk         = 0.0f;
+ 25     float right_risk        = 0.0f;
+ 26     float crack_risk        = 0.0f;
+ 27     float min_distance_m    = 0.0f;
+ 28     float max_confidence    = 0.0f;
+ 29     int   num_detections    = 0;
+ 30     int   priority_detections = 0;
+ 31     int   num_filtered_out  = 0;
+ 32     double latency_ms       = 0.0;
+ 33     std::vector<YoloDetection> detections;
+ 34 };
+ 35 
+ 36 // =============================================================================
+ 37 // Public API
+ 38 // =============================================================================
+ 39 
+ 40 // Load YOLOv8 ONNX model. Returns true on success.
+ 41 bool yolo_init(const std::string& model_path, int input_size = 640);
+ 42 
+ 43 // Run inference on a BGR frame. Returns spatial risk assessment.
+ 44 YoloResult yolo_detect(const cv::Mat& frame,
+ 45                        float conf_threshold = 0.35f,
+ 46                        float nms_threshold  = 0.45f);
+ 47 
+ 48 // Release model resources.
+ 49 void yolo_shutdown();