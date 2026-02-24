+   1 // =============================================================================
+   2 // yolo_bridge.cpp — Full OpenCV DNN YOLOv8 ONNX Implementation
+   3 //
+   4 // Features:
+   5 //   - YOLOv8 ONNX inference via cv::dnn
+   6 //   - Letterbox preprocessing
+   7 //   - [1,84,N] output parsing (4 bbox + 80 COCO classes)
+   8 //   - NMS with IoU thresholding
+   9 //   - Spatial bucketing: front/left/right/crack risk zones
+  10 //   - Distance estimation from bbox size
+  11 //   - Priority class detection (person, vehicle)
+  12 //
+  13 // Dependencies: OpenCV 4.x (core, dnn, imgproc)
+  14 // =============================================================================
+  15 
+  16 #include "yolo_bridge.h"
+  17 
+  18 #include <opencv2/dnn.hpp>
+  19 #include <opencv2/imgproc.hpp>
+  20 
+  21 #include <chrono>
+  22 #include <algorithm>
+  23 #include <cmath>
+  24 #include <mutex>
+  25 #include <cstdio>
+  26 
+  27 namespace {
+  28 
+  29 // --- Module state ---
+  30 cv::dnn::Net g_net;
+  31 bool         g_initialized = false;
+  32 int          g_input_size  = 640;
+  33 std::mutex   g_init_mutex;
+  34 
+  35 static constexpr int NUM_CLASSES = 80;
+  36 
+  37 // --- Priority classes (COCO): vehicles, persons, large obstacles ---
+  38 bool is_priority_class(int class_id) {
+  39     // person=0, bicycle=1, car=2, motorcycle=3, bus=5, train=6, truck=7
+  40     return class_id == 0 || class_id == 2 || class_id == 3 ||
+  41            class_id == 5 || class_id == 6 || class_id == 7;
+  42 }
+  43 
+  44 // --- Letterbox resize for YOLOv8 ---
+  45 cv::Mat letterbox(const cv::Mat& src, int target_size,
+  46                   float& scale, int& pad_x, int& pad_y)
+  47 {
+  48     int h = src.rows;
+  49     int w = src.cols;
+  50 
+  51     scale = std::min(static_cast<float>(target_size) / h,
+  52                      static_cast<float>(target_size) / w);
+  53 
+  54     int new_w = static_cast<int>(w * scale);
+  55     int new_h = static_cast<int>(h * scale);
+  56 
+  57     pad_x = (target_size - new_w) / 2;
+  58     pad_y = (target_size - new_h) / 2;
+  59 
+  60     cv::Mat resized;
+  61     cv::resize(src, resized, cv::Size(new_w, new_h), 0, 0, cv::INTER_AREA);
+  62 
+  63     cv::Mat padded(target_size, target_size, CV_8UC3,
+  64                    cv::Scalar(114, 114, 114));
+  65     resized.copyTo(padded(cv::Rect(pad_x, pad_y, new_w, new_h)));
+  66 
+  67     return padded;
+  68 }
+  69 
+  70 // --- Non-Maximum Suppression ---
+  71 void nms(std::vector<YoloDetection>& dets, float iou_threshold) {
+  72     std::sort(dets.begin(), dets.end(),
+  73               [](const YoloDetection& a, const YoloDetection& b) {
+  74                   return a.confidence > b.confidence;
+  75               });
+  76 
+  77     std::vector<bool> suppressed(dets.size(), false);
+  78 
+  79     for (size_t i = 0; i < dets.size(); i++) {
+  80         if (suppressed[i]) continue;
+  81 
+  82         for (size_t j = i + 1; j < dets.size(); j++) {
+  83             if (suppressed[j]) continue;
+  84 
+  85             float ix1 = std::max(dets[i].x1, dets[j].x1);
+  86             float iy1 = std::max(dets[i].y1, dets[j].y1);
+  87             float ix2 = std::min(dets[i].x2, dets[j].x2);
+  88             float iy2 = std::min(dets[i].y2, dets[j].y2);
+  89 
+  90             float inter_w = std::max(0.0f, ix2 - ix1);
+  91             float inter_h = std::max(0.0f, iy2 - iy1);
+  92             float inter_area = inter_w * inter_h;
+  93 
+  94             float area_i = (dets[i].x2 - dets[i].x1) *
+  95                            (dets[i].y2 - dets[i].y1);
+  96             float area_j = (dets[j].x2 - dets[j].x1) *
+  97                            (dets[j].y2 - dets[j].y1);
+  98 
+  99             float iou = inter_area /
+ 100                         (area_i + area_j - inter_area + 1e-6f);
+ 101 
+ 102             if (iou > iou_threshold) {
+ 103                 suppressed[j] = true;
+ 104             }
+ 105         }
+ 106     }
+ 107 
+ 108     std::vector<YoloDetection> kept;
+ 109     kept.reserve(dets.size());
+ 110     for (size_t i = 0; i < dets.size(); i++) {
+ 111         if (!suppressed[i]) kept.push_back(dets[i]);
+ 112     }
+ 113     dets = std::move(kept);
+ 114 }
+ 115 
+ 116 // --- Spatial risk bucketing ---
+ 117 void compute_spatial_risks(const std::vector<YoloDetection>& dets,
+ 118                            int frame_w, int frame_h,
+ 119                            float& front_risk, float& left_risk,
+ 120                            float& right_risk, float& crack_risk)
+ 121 {
+ 122     front_risk = 0.0f;
+ 123     left_risk  = 0.0f;
+ 124     right_risk = 0.0f;
+ 125     crack_risk = 0.0f;
+ 126 
+ 127     float third_w = static_cast<float>(frame_w) / 3.0f;
+ 128     float bottom_third_y = static_cast<float>(frame_h) * 2.0f / 3.0f;
+ 129 
+ 130     for (const auto& d : dets) {
+ 131         float cx = (d.x1 + d.x2) * 0.5f;
+ 132         float cy = (d.y1 + d.y2) * 0.5f;
+ 133         float conf = d.confidence;
+ 134 
+ 135         // Front zone: center third, bottom half
+ 136         if (cx >= third_w && cx < 2.0f * third_w &&
+ 137             cy >= static_cast<float>(frame_h) * 0.5f) {
+ 138             front_risk = std::max(front_risk, conf);
+ 139         }
+ 140 
+ 141         // Left zone: left third
+ 142         if (cx < third_w) {
+ 143             left_risk = std::max(left_risk, conf);
+ 144         }
+ 145 
+ 146         // Right zone: right third
+ 147         if (cx >= 2.0f * third_w) {
+ 148             right_risk = std::max(right_risk, conf);
+ 149         }
+ 150 
+ 151         // Bottom third → ground-level crack risk
+ 152         if (cy >= bottom_third_y) {
+ 153             crack_risk = std::max(crack_risk, conf * 0.5f);
+ 154         }
+ 155     }
+ 156 }
+ 157 
+ 158 // --- Distance estimation from bbox height ---
+ 159 float estimate_distance(const YoloDetection& det, int frame_h) {
+ 160     float bbox_h = det.y2 - det.y1;
+ 161     if (bbox_h < 1.0f) return 100.0f;
+ 162 
+ 163     float ratio = bbox_h / static_cast<float>(frame_h);
+ 164     // Empirical inverse mapping
+ 165     float distance = 2.0f / (ratio + 0.1f);
+ 166     return std::clamp(distance, 0.5f, 100.0f);
+ 167 }
+ 168 
+ 169 } // anonymous namespace
+ 170 
+ 171 // =============================================================================
+ 172 // Public API
+ 173 // =============================================================================
+ 174 
+ 175 bool yolo_init(const std::string& model_path, int input_size) {
+ 176     std::lock_guard<std::mutex> lock(g_init_mutex);
+ 177 
+ 178     if (g_initialized) return true;
+ 179 
+ 180     try {
+ 181         g_net = cv::dnn::readNetFromONNX(model_path);
+ 182 
+ 183         // Use OpenCV backend + CPU target (portable)
+ 184         g_net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
+ 185         g_net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
+ 186 
+ 187         g_input_size  = input_size;
+ 188         g_initialized = true;
+ 189         return true;
+ 190     } catch (const cv::Exception& e) {
+ 191         std::fprintf(stderr, "[YOLO] Failed to load model '%s': %s\n",
+ 192                      model_path.c_str(), e.what());
+ 193         g_initialized = false;
+ 194         return false;
+ 195     }
+ 196 }
+ 197 
+ 198 YoloResult yolo_detect(const cv::Mat& frame,
+ 199                        float conf_threshold,
+ 200                        float nms_threshold)
+ 201 {
+ 202     YoloResult result;
+ 203 
+ 204     auto t0 = std::chrono::steady_clock::now();
+ 205 
+ 206     if (!g_initialized || frame.empty()) {
+ 207         auto t1 = std::chrono::steady_clock::now();
+ 208         result.latency_ms = std::chrono::duration<double, std::milli>(
+ 209             t1 - t0).count();
+ 210         return result;
+ 211     }
+ 212 
+ 213     int frame_w = frame.cols;
+ 214     int frame_h = frame.rows;
+ 215 
+ 216     // Letterbox resize
+ 217     float scale;
+ 218     int pad_x, pad_y;
+ 219     cv::Mat letterboxed = letterbox(frame, g_input_size, scale, pad_x, pad_y);
+ 220 
+ 221     // Create blob: [1, 3, H, W], normalized to [0,1]
+ 222     cv::Mat blob = cv::dnn::blobFromImage(
+ 223         letterboxed, 1.0 / 255.0,
+ 224         cv::Size(g_input_size, g_input_size),
+ 225         cv::Scalar(), true, false);
+ 226 
+ 227     g_net.setInput(blob);
+ 228 
+ 229     // Forward pass
+ 230     std::vector<cv::Mat> outputs;
+ 231     g_net.forward(outputs, g_net.getUnconnectedOutLayersNames());
+ 232 
+ 233     if (outputs.empty()) {
+ 234         auto t1 = std::chrono::steady_clock::now();
+ 235         result.latency_ms = std::chrono::duration<double, std::milli>(
+ 236             t1 - t0).count();
+ 237         return result;
+ 238     }
+ 239 
+ 240     // Parse YOLOv8 output: shape [1, 84, N]
+ 241     // Layout in memory: feature f for detection d → data[f * N + d]
+ 242     cv::Mat output = outputs[0];
+ 243 
+ 244     if (output.dims < 2) {
+ 245         auto t1 = std::chrono::steady_clock::now();
+ 246         result.latency_ms = std::chrono::duration<double, std::milli>(
+ 247             t1 - t0).count();
+ 248         return result;
+ 249     }
+ 250 
+ 251     int num_features   = output.size[1];  // 84 = 4 bbox + 80 classes
+ 252     int num_candidates = (output.dims == 3) ? output.size[2] : output.size[1];
+ 253 
+ 254     if (output.dims == 2) {
+ 255         // [N, 84] format — some ONNX exports use this
+ 256         num_candidates = output.size[0];
+ 257         num_features   = output.size[1];
+ 258     }
+ 259 
+ 260     float* raw = reinterpret_cast<float*>(output.data);
+ 261 
+ 262     std::vector<YoloDetection> detections;
+ 263     int filtered_count = 0;
+ 264 
+ 265     bool transposed = (output.dims == 3); // [1, 84, N] needs column-major access
+ 266 
+ 267     for (int i = 0; i < num_candidates; i++) {
+ 268         float cx, cy, bw, bh;
+ 269         float max_score = 0.0f;
+ 270         int best_class = -1;
+ 271 
+ 272         if (transposed) {
+ 273             // [1, 84, N]: feature f, detection i → raw[f * N + i]
+ 274             int N = num_candidates;
+ 275             cx = raw[0 * N + i];
+ 276             cy = raw[1 * N + i];
+ 277             bw = raw[2 * N + i];
+ 278             bh = raw[3 * N + i];
+ 279 
+ 280             for (int c = 0; c < NUM_CLASSES && (4 + c) < num_features; c++) {
+ 281                 float score = raw[(4 + c) * N + i];
+ 282                 if (score > max_score) {
+ 283                     max_score = score;
+ 284                     best_class = c;
+ 285                 }
+ 286             }
+ 287         } else {
+ 288             // [N, 84]: detection i, feature f → raw[i * 84 + f]
+ 289             float* row = raw + i * num_features;
+ 290             cx = row[0];
+ 291             cy = row[1];
+ 292             bw = row[2];
+ 293             bh = row[3];
+ 294 
+ 295             for (int c = 0; c < NUM_CLASSES && (4 + c) < num_features; c++) {
+ 296                 float score = row[4 + c];
+ 297                 if (score > max_score) {
+ 298                     max_score = score;
+ 299                     best_class = c;
+ 300                 }
+ 301             }
+ 302         }
+ 303 
+ 304         if (max_score < conf_threshold) {
+ 305             filtered_count++;
+ 306             continue;
+ 307         }
+ 308 
+ 309         // Convert from letterbox coords to original frame coords
+ 310         float x1 = (cx - bw * 0.5f - pad_x) / scale;
+ 311         float y1 = (cy - bh * 0.5f - pad_y) / scale;
+ 312         float x2 = (cx + bw * 0.5f - pad_x) / scale;
+ 313         float y2 = (cy + bh * 0.5f - pad_y) / scale;
+ 314 
+ 315         // Clamp to frame bounds
+ 316         x1 = std::clamp(x1, 0.0f, static_cast<float>(frame_w));
+ 317         y1 = std::clamp(y1, 0.0f, static_cast<float>(frame_h));
+ 318         x2 = std::clamp(x2, 0.0f, static_cast<float>(frame_w));
+ 319         y2 = std::clamp(y2, 0.0f, static_cast<float>(frame_h));
+ 320 
+ 321         // Validate bbox area
+ 322         float area_ratio = ((x2 - x1) * (y2 - y1)) /
+ 323                            (static_cast<float>(frame_w) *
+ 324                             static_cast<float>(frame_h));
+ 325         if (area_ratio < 0.001f || area_ratio > 0.80f) {
+ 326             filtered_count++;
+ 327             continue;
+ 328         }
+ 329 
+ 330         YoloDetection det;
+ 331         det.x1 = x1;
+ 332         det.y1 = y1;
+ 333         det.x2 = x2;
+ 334         det.y2 = y2;
+ 335         det.confidence = max_score;
+ 336         det.class_id   = best_class;
+ 337 
+ 338         detections.push_back(det);
+ 339     }
+ 340 
+ 341     result.num_filtered_out = filtered_count;
+ 342 
+ 343     // Apply NMS
+ 344     nms(detections, nms_threshold);
+ 345 
+ 346     result.detections     = detections;
+ 347     result.num_detections = static_cast<int>(detections.size());
+ 348 
+ 349     // Spatial risk assessment
+ 350     compute_spatial_risks(detections, frame_w, frame_h,
+ 351                           result.front_risk, result.left_risk,
+ 352                           result.right_risk, result.crack_risk);
+ 353 
+ 354     // Max confidence, priority detections, min distance
+ 355     result.min_distance_m = 100.0f;
+ 356     for (const auto& d : detections) {
+ 357         result.max_confidence = std::max(result.max_confidence, d.confidence);
+ 358 
+ 359         if (is_priority_class(d.class_id)) {
+ 360             result.priority_detections++;
+ 361         }
+ 362 
+ 363         float dist = estimate_distance(d, frame_h);
+ 364         result.min_distance_m = std::min(result.min_distance_m, dist);
+ 365     }
+ 366 
+ 367     if (detections.empty()) {
+ 368         result.min_distance_m = 0.0f;
+ 369     }
+ 370 
+ 371     auto t1 = std::chrono::steady_clock::now();
+ 372     result.latency_ms = std::chrono::duration<double, std::milli>(
+ 373         t1 - t0).count();
+ 374 
+ 375     return result;
+ 376 }
+ 377 
+ 378 void yolo_shutdown() {
+ 379     std::lock_guard<std::mutex> lock(g_init_mutex);
+ 380     if (g_initialized) {
+ 381         g_net = cv::dnn::Net();
+ 382         g_initialized = false;
+ 383     }
+ 384 }