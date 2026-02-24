+  1 // =============================================================================
+  2 // crack_statistics.cpp — Implementations for CrackStats and DetectionScheduler
+  3 // (declared in types.h)
+  4 // =============================================================================
+  5 
+  6 #include "types.h"
+  7 #include <chrono>
+  8 #include <cmath>
+  9 #include <algorithm>
+ 10 
+ 11 static double cs_now_ms() {
+ 12     return std::chrono::duration<double, std::milli>(
+ 13         std::chrono::steady_clock::now().time_since_epoch()).count();
+ 14 }
+ 15 
+ 16 // =============================================================================
+ 17 // CrackStats
+ 18 // =============================================================================
+ 19 
+ 20 CrackStats::CrackStats(double window_ms)
+ 21     : window_ms_(window_ms)
+ 22 {}
+ 23 
+ 24 void CrackStats::add_sample(double ts_ms, float crack_score) {
+ 25     std::lock_guard<std::mutex> lock(m_);
+ 26 
+ 27     samples_.emplace_back(ts_ms, crack_score);
+ 28     total_frames_++;
+ 29     if (crack_score > 0.3f) crack_frames_++;
+ 30 
+ 31     // Prune old samples
+ 32     while (!samples_.empty() &&
+ 33            (ts_ms - samples_.front().first) > window_ms_) {
+ 34         samples_.pop_front();
+ 35     }
+ 36 }
+ 37 
+ 38 float CrackStats::window_crack_ratio() const {
+ 39     std::lock_guard<std::mutex> lock(m_);
+ 40     if (samples_.empty()) return 0.0f;
+ 41 
+ 42     int count = 0;
+ 43     for (const auto& s : samples_) {
+ 44         if (s.second > 0.3f) count++;
+ 45     }
+ 46     return static_cast<float>(count) /
+ 47            static_cast<float>(samples_.size());
+ 48 }
+ 49 
+ 50 float CrackStats::global_crack_ratio() const {
+ 51     std::lock_guard<std::mutex> lock(m_);
+ 52     if (total_frames_ == 0) return 0.0f;
+ 53     return static_cast<float>(crack_frames_) /
+ 54            static_cast<float>(total_frames_);
+ 55 }
+ 56 
+ 57 // =============================================================================
+ 58 // DetectionScheduler
+ 59 // =============================================================================
+ 60 
+ 61 DetectionScheduler::DetectionScheduler()
+ 62     : base_interval_ms_(100.0)
+ 63     , last_detect_ms_(0.0)
+ 64 {}
+ 65 
+ 66 bool DetectionScheduler::should_detect(float crack_score) {
+ 67     double now = cs_now_ms();
+ 68 
+ 69     double interval = base_interval_ms_;
+ 70 
+ 71     // Shorten interval when crack score is high
+ 72     if (crack_score > 0.8f) {
+ 73         interval *= 0.25;
+ 74     } else if (crack_score > 0.5f) {
+ 75         interval *= 0.5;
+ 76     } else if (crack_score > 0.2f) {
+ 77         interval *= 0.75;
+ 78     }
+ 79 
+ 80     if (now - last_detect_ms_ >= interval) {
+ 81         last_detect_ms_ = now;
+ 82         return true;
+ 83     }
+ 84     return false;
+ 85 }