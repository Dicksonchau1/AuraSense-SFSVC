+  1 #pragma once
+  2 
+  3 #include <algorithm>
+  4 #include <chrono>
+  5 #include <cmath>
+  6 #include <atomic>
+  7 
+  8 // =============================================================================
+  9 // YoloManager — adaptive YOLO scheduling and rate control
+ 10 // Controls when YOLO should run based on vehicle speed, scene activity,
+ 11 // and recent detection history.
+ 12 // =============================================================================
+ 13 class YoloManager {
+ 14 public:
+ 15     YoloManager()
+ 16         : target_hz_(10.0f)
+ 17         , min_hz_(5.0f)
+ 18         , max_hz_(20.0f)
+ 19         , last_run_time_(0.0)
+ 20         , consecutive_empty_(0)
+ 21         , consecutive_detect_(0)
+ 22     {}
+ 23 
+ 24     // Update target Hz based on vehicle speed (m/s)
+ 25     void update_speed(float speed_mps) {
+ 26         float hz;
+ 27         if (speed_mps > 1.0f) {
+ 28             hz = max_hz_;
+ 29         } else if (speed_mps > 0.5f) {
+ 30             hz = min_hz_ + (speed_mps - 0.5f) * 2.0f * (max_hz_ - min_hz_);
+ 31         } else {
+ 32             hz = min_hz_ + speed_mps * 2.0f * (max_hz_ - min_hz_) * 0.5f;
+ 33         }
+ 34         target_hz_ = std::clamp(hz, min_hz_, max_hz_);
+ 35     }
+ 36 
+ 37     // Check if enough time has elapsed to run YOLO again
+ 38     bool should_run(double now_s) const {
+ 39         if (last_run_time_ <= 0.0) return true;
+ 40         double period_s = 1.0 / static_cast<double>(target_hz_);
+ 41         return (now_s - last_run_time_) >= period_s;
+ 42     }
+ 43 
+ 44     // Record that YOLO ran at this timestamp
+ 45     void record_run(double now_s, int num_detections) {
+ 46         last_run_time_ = now_s;
+ 47 
+ 48         if (num_detections == 0) {
+ 49             consecutive_empty_++;
+ 50             consecutive_detect_ = 0;
+ 51             // Back off rate when scene is empty
+ 52             if (consecutive_empty_ > 10) {
+ 53                 target_hz_ = std::max(min_hz_, target_hz_ * 0.95f);
+ 54             }
+ 55         } else {
+ 56             consecutive_detect_++;
+ 57             consecutive_empty_ = 0;
+ 58             // Increase rate when detections are frequent
+ 59             if (consecutive_detect_ > 3) {
+ 60                 target_hz_ = std::min(max_hz_, target_hz_ * 1.05f);
+ 61             }
+ 62         }
+ 63     }
+ 64 
+ 65     float target_hz() const { return target_hz_; }
+ 66     float min_hz()    const { return min_hz_; }
+ 67     float max_hz()    const { return max_hz_; }
+ 68 
+ 69     void set_range(float min_hz, float max_hz) {
+ 70         min_hz_ = min_hz;
+ 71         max_hz_ = max_hz;
+ 72         target_hz_ = std::clamp(target_hz_, min_hz_, max_hz_);
+ 73     }
+ 74 
+ 75 private:
+ 76     float  target_hz_;
+ 77     float  min_hz_;
+ 78     float  max_hz_;
+ 79     double last_run_time_;
+ 80     int    consecutive_empty_;
+ 81     int    consecutive_detect_;
+ 82 };