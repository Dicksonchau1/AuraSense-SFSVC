#pragma once

#include <algorithm>
#include <cmath>
#include <deque>
#include <mutex>
#include <vector>
#include <string>  // ADD THIS


// =============================================================================
// CrackStatistics - Real-Time Crack Analysis with Statistical Tracking
// =============================================================================

struct CrackStatistics {
    // Current frame
    float crack_score = 0.0f;
    float fused_crack_score = 0.0f;
    float width_mm = 0.0f;
    float length_mm = 0.0f;
    float severity_index = 0.0f;  // 0.0 = none, 1.0 = critical

    // Classification
    bool is_critical = false;
    bool is_warning = false;
    bool is_hairline = false;
    std::string severity_label = "None";

    // Confidence metrics
    float detection_confidence = 0.0f;
    float measurement_confidence = 0.0f;
    float temporal_confidence = 0.0f;

    // Window statistics (last N frames)
    float avg_crack_score = 0.0f;
    float max_crack_score = 0.0f;
    float crack_variance = 0.0f;
    float crack_trend = 0.0f;  // -1.0 = decreasing, +1.0 = increasing

    // Growth analysis
    float growth_rate_mm_per_sec = 0.0f;
    float growth_acceleration = 0.0f;
    bool is_growing = false;

    // Spatial features
    float vertical_extent_mm = 0.0f;
    float horizontal_extent_mm = 0.0f;
    float aspect_ratio = 0.0f;  // width / length
    int num_branches = 0;

    // Temporal tracking
    int frames_since_first_detection = 0;
    int consecutive_detection_frames = 0;
    double first_detection_timestamp_ms = 0.0;
    double last_detection_timestamp_ms = 0.0;

    // Alert status
    bool should_alert = false;
    bool alert_acknowledged = false;
    int alert_priority = 0;  // 0 = none, 1 = low, 2 = medium, 3 = high, 4 = critical
};

// =============================================================================
// CrackStatisticsTracker - Maintains running statistics over time window
// =============================================================================

class CrackStatisticsTracker {
public:
    explicit CrackStatisticsTracker(
        double window_ms = 5000.0,
        float px_to_mm_scale = 1.0f,
        float critical_width_mm = 3.0f,
        float warning_width_mm = 1.0f,
        float hairline_width_mm = 0.3f
    )
        : window_ms_(window_ms)
        , px_to_mm_scale_(px_to_mm_scale)
        , critical_width_mm_(critical_width_mm)
        , warning_width_mm_(warning_width_mm)
        , hairline_width_mm_(hairline_width_mm)
        , total_frames_(0)
        , crack_detected_frames_(0)
        , first_detection_frame_(-1)
    {}

    // =========================================================================
    // Update with new crack measurement
    // =========================================================================
    CrackStatistics update(
        double timestamp_ms,
        float crack_score,
        float fused_crack_score,
        float yolo_crack_confidence = 0.0f,
        int frame_id = 0
    ) {
        std::lock_guard<std::mutex> lock(mutex_);

        total_frames_++;

        // Add to history
        CrackSample sample;
        sample.timestamp_ms = timestamp_ms;
        sample.crack_score = crack_score;
        sample.fused_crack_score = fused_crack_score;
        sample.yolo_confidence = yolo_crack_confidence;
        sample.frame_id = frame_id;

        history_.push_back(sample);

        // Prune old samples outside window
        while (!history_.empty() &&
               (timestamp_ms - history_.front().timestamp_ms) > window_ms_) {
            history_.pop_front();
        }

        // Track detection state
        if (fused_crack_score > 0.05f) {
            crack_detected_frames_++;

            if (first_detection_frame_ < 0) {
                first_detection_frame_ = frame_id;
                first_detection_timestamp_ms_ = timestamp_ms;
            }

            consecutive_detections_++;
            last_detection_timestamp_ms_ = timestamp_ms;
        } else {
            consecutive_detections_ = 0;
        }

        // Compute statistics
        return compute_statistics(timestamp_ms, fused_crack_score, frame_id);
    }

    // =========================================================================
    // Get window statistics
    // =========================================================================
    float get_window_crack_ratio() const {
        std::lock_guard<std::mutex> lock(mutex_);

        if (history_.empty()) return 0.0f;

        int count = 0;
        for (const auto& s : history_) {
            if (s.fused_crack_score > 0.05f) count++;
        }

        return static_cast<float>(count) / static_cast<float>(history_.size());
    }

    float get_global_crack_ratio() const {
        std::lock_guard<std::mutex> lock(mutex_);

        if (total_frames_ == 0) return 0.0f;
        return static_cast<float>(crack_detected_frames_) /
               static_cast<float>(total_frames_);
    }

    // =========================================================================
    // Configuration
    // =========================================================================
    void set_px_to_mm_scale(float scale) {
        std::lock_guard<std::mutex> lock(mutex_);
        px_to_mm_scale_ = scale;
    }

    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);

        history_.clear();
        total_frames_ = 0;
        crack_detected_frames_ = 0;
        consecutive_detections_ = 0;
        first_detection_frame_ = -1;
        first_detection_timestamp_ms_ = 0.0;
        last_detection_timestamp_ms_ = 0.0;
    }

private:
    struct CrackSample {
        double timestamp_ms = 0.0;
        float crack_score = 0.0f;
        float fused_crack_score = 0.0f;
        float yolo_confidence = 0.0f;
        int frame_id = 0;
    };

    // =========================================================================
    // Compute full statistics (assumes lock is held)
    // =========================================================================
    CrackStatistics compute_statistics(
        double current_time_ms,
        float current_fused_score,
        int current_frame_id
    ) {
        CrackStatistics stats;

        stats.crack_score = history_.empty() ? 0.0f : history_.back().crack_score;
        stats.fused_crack_score = current_fused_score;

        // Compute width in mm (deterministic linear mapping)
        const float WIDTH_SCALE_MM = 3.0f;
        stats.width_mm = current_fused_score * WIDTH_SCALE_MM;

        // Estimate length (proportional to width with aspect ratio ~3:1)
        stats.length_mm = stats.width_mm * 3.0f;
        stats.vertical_extent_mm = stats.length_mm;
        stats.horizontal_extent_mm = stats.width_mm;
        stats.aspect_ratio = stats.width_mm / std::max(0.001f, stats.length_mm);

        // Severity classification (deterministic thresholds)
        if (stats.width_mm >= critical_width_mm_) {
            stats.severity_label = "Critical";
            stats.severity_index = 1.0f;
            stats.is_critical = true;
            stats.alert_priority = 4;
        } else if (stats.width_mm >= warning_width_mm_) {
            stats.severity_label = "Warning";
            stats.severity_index = 0.6f;
            stats.is_warning = true;
            stats.alert_priority = 3;
        } else if (stats.width_mm >= hairline_width_mm_) {
            stats.severity_label = "Hairline";
            stats.severity_index = 0.3f;
            stats.is_hairline = true;
            stats.alert_priority = 1;
        } else {
            stats.severity_label = "None";
            stats.severity_index = 0.0f;
            stats.alert_priority = 0;
        }

        // Detection confidence (based on score magnitude and consistency)
        stats.detection_confidence = std::min(1.0f, current_fused_score * 2.5f);

        // Measurement confidence (based on temporal stability)
        if (history_.size() >= 3) {
            float var = compute_variance();
            stats.measurement_confidence = std::exp(-var * 10.0f);
        } else {
            stats.measurement_confidence = 0.5f;  // Low confidence with few samples
        }

        // Temporal confidence (based on consecutive detections)
        if (consecutive_detections_ > 0) {
            stats.temporal_confidence = std::min(1.0f,
                static_cast<float>(consecutive_detections_) / 10.0f);
        } else {
            stats.temporal_confidence = 0.0f;
        }

        // Window statistics
        if (!history_.empty()) {
            float sum = 0.0f;
            float max_val = -1.0f;

            for (const auto& s : history_) {
                sum += s.fused_crack_score;
                max_val = std::max(max_val, s.fused_crack_score);
            }

            stats.avg_crack_score = sum / static_cast<float>(history_.size());
            stats.max_crack_score = max_val;
            stats.crack_variance = compute_variance();
        }

        // Trend analysis (linear regression over window)
        stats.crack_trend = compute_trend();

        // Growth analysis
        if (history_.size() >= 5) {
            float dt_sec = (current_time_ms - history_.front().timestamp_ms) / 1000.0f;
            if (dt_sec > 0.1f) {
                float width_start = history_.front().fused_crack_score * WIDTH_SCALE_MM;
                float width_end = current_fused_score * WIDTH_SCALE_MM;

                stats.growth_rate_mm_per_sec = (width_end - width_start) / dt_sec;
                stats.is_growing = (stats.growth_rate_mm_per_sec > 0.01f);

                // Compute acceleration if we have enough history
                if (history_.size() >= 10) {
                    size_t mid = history_.size() / 2;
                    double t_mid = history_[mid].timestamp_ms;
                    float w_mid = history_[mid].fused_crack_score * WIDTH_SCALE_MM;

                    float dt1 = (t_mid - history_.front().timestamp_ms) / 1000.0f;
                    float dt2 = (current_time_ms - t_mid) / 1000.0f;

                    float rate1 = (w_mid - width_start) / std::max(0.01f, dt1);
                    float rate2 = (width_end - w_mid) / std::max(0.01f, dt2);

                    stats.growth_acceleration = (rate2 - rate1) /
                        std::max(0.01f, (dt1 + dt2) / 2.0f);
                }
            }
        }

        // Spatial features (estimate branches from score variance)
        if (stats.crack_variance > 0.02f) {
            stats.num_branches = static_cast<int>(stats.crack_variance * 20.0f);
        } else {
            stats.num_branches = 0;
        }

        // Temporal tracking
        if (first_detection_frame_ >= 0) {
            stats.frames_since_first_detection = current_frame_id - first_detection_frame_;
            stats.first_detection_timestamp_ms = first_detection_timestamp_ms_;
        }
        stats.consecutive_detection_frames = consecutive_detections_;
        stats.last_detection_timestamp_ms = last_detection_timestamp_ms_;

        // Alert logic (deterministic)
        stats.should_alert = false;

        // Critical alert: width >= 3.0mm OR rapid growth
        if (stats.is_critical || stats.growth_rate_mm_per_sec > 0.5f) {
            stats.should_alert = true;
            stats.alert_priority = 4;
        }
        // Warning alert: width >= 1.0mm AND consecutive detections > 5
        else if (stats.is_warning && consecutive_detections_ > 5) {
            stats.should_alert = true;
            stats.alert_priority = 3;
        }
        // Hairline alert: growing AND detected for > 20 frames
        else if (stats.is_hairline && stats.is_growing &&
                 stats.frames_since_first_detection > 20) {
            stats.should_alert = true;
            stats.alert_priority = 2;
        }

        return stats;
    }

    // =========================================================================
    // Helper: Compute variance of crack scores in window
    // =========================================================================
    float compute_variance() const {
        if (history_.size() < 2) return 0.0f;

        float sum = 0.0f;
        for (const auto& s : history_) {
            sum += s.fused_crack_score;
        }
        float mean = sum / static_cast<float>(history_.size());

        float var_sum = 0.0f;
        for (const auto& s : history_) {
            float diff = s.fused_crack_score - mean;
            var_sum += diff * diff;
        }

        return var_sum / static_cast<float>(history_.size());
    }

    // =========================================================================
    // Helper: Compute trend via linear regression
    // Returns value in [-1.0, 1.0]: -1 = strong decrease, +1 = strong increase
    // =========================================================================
    float compute_trend() const {
        if (history_.size() < 3) return 0.0f;

        // Linear regression: y = a + b*x
        // We want slope 'b' normalized

        float sum_x = 0.0f;
        float sum_y = 0.0f;
        float sum_xy = 0.0f;
        float sum_x2 = 0.0f;
        float n = static_cast<float>(history_.size());

        for (size_t i = 0; i < history_.size(); ++i) {
            float x = static_cast<float>(i);
            float y = history_[i].fused_crack_score;

            sum_x += x;
            sum_y += y;
            sum_xy += x * y;
            sum_x2 += x * x;
        }

        float denom = n * sum_x2 - sum_x * sum_x;
        if (std::abs(denom) < 1e-6f) return 0.0f;

        float slope = (n * sum_xy - sum_x * sum_y) / denom;

        // Normalize slope to [-1, 1]
        // slope is in units of crack_score per sample
        // Typical range: -0.1 to +0.1 per sample
        return std::clamp(slope * 10.0f, -1.0f, 1.0f);
    }

    // Configuration
    double window_ms_;
    float px_to_mm_scale_;
    float critical_width_mm_;
    float warning_width_mm_;
    float hairline_width_mm_;

    // Runtime state
    std::deque<CrackSample> history_;
    uint64_t total_frames_;
    uint64_t crack_detected_frames_;
    int consecutive_detections_ = 0;
    int first_detection_frame_;
    double first_detection_timestamp_ms_;
    double last_detection_timestamp_ms_;

    // Thread safety
    mutable std::mutex mutex_;
};
