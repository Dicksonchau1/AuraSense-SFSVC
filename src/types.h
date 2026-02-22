#pragma once

#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

// =============================================================================
// Failsafe Status (single authoritative definition)
// =============================================================================
enum class FailsafeStatus : int {
    OK       = 0,
    WARNING  = 1,
    CRITICAL = 2
};

// =============================================================================
// CrackMetrics
// =============================================================================
struct CrackMetrics {
    float score = 0.0f;
    float width_mm = 0.0f;
    float length_mm = 0.0f;
    std::string severity_label = "Unknown";
    float confidence_percent = 0.0f;
    bool is_critical = false;
    bool is_warning = false;
};

// =============================================================================
// ControlDecision
// =============================================================================
struct ControlDecision {
    int frame_id = 0;
    double timestamp = 0.0;
    float throttle = 1.0f;
    float steer = 0.0f;
    std::string action;

    float crack_score = 0.0f;
    float sparsity = 1.0f;
    float confidence = 0.0f;
    double semantic_age_ms = 99999.0;

    bool is_null_cycle = false;
    bool inference_suppressed = false;
    bool event_only_mode = false;

    bool yolo_active = false;
    float yolo_age_ms = 0.0f;

    int reference_frame_age = 0;
    double encode_time_ms = 0.0;
    double control_latency_ms = 0.0;

    float crack_width_mm = 0.0f;
    float crack_length_mm = 0.0f;
    std::string crack_severity;
    float crack_confidence_percent = 0.0f;
};

// =============================================================================
// Jobs
// =============================================================================
struct Lane2Job {
    int frame_id = 0;
    int height = 0;
    int width = 0;
    float crack_score = 0.0f;
    std::vector<uint8_t> frame_bgr;
};

struct Lane3Job {
    int frame_id = 0;
    int height = 0;
    int width = 0;
    double timestamp = 0.0;
    float crack_score = 0.0f;
    float sparsity = 1.0f;
    float target_hz = 5.0f;
    std::vector<uint8_t> frame_bgr;
};

struct VisJob {
    int frame_id = 0;
    int frame_h = 0;
    int frame_w = 0;
    float crack_score = 0.0f;
    float sparsity = 1.0f;
    int on_count = 0;
    int off_count = 0;
    std::vector<uint8_t> frame_bgr;
};

struct UplinkPayload {
    int frame_id = 0;
    double timestamp = 0.0;
    float throttle = 1.0f;
    float steer = 0.0f;
    float crack_score = 0.0f;
    float sparsity = 1.0f;
    std::string action;
    double control_latency_ms = 0.0;
    std::string crack_severity;
    float crack_width_mm = 0.0f;
    FailsafeStatus failsafe_status = FailsafeStatus::OK;
    float sig_conf = 0.0f;
    uint64_t yolo_count = 0;
    uint64_t latency_violations = 0;
    std::vector<uint8_t> packed_spikes;
};

struct SemanticState {
    int frame_id = 0;
    double timestamp_ms = 0.0;
    double latency_ms = 0.0;
    float front_risk = 0.0f;
    float left_risk = 0.0f;
    float right_risk = 0.0f;
    float crack_risk = 0.0f;
    int num_detections = 0;
    float max_confidence = 0.0f;
    float agreement = 0.0f;
};

struct SignatureMatch {
    bool matched = false;
    float confidence = 0.0f;
    int id = -1;
    float crack_score = 0.0f;
};

// =============================================================================
// Benchmarking
// =============================================================================
struct BenchmarkSample {
    int lane = 0;
    int frame_id = 0;
    float latency_ms = 0.0f;
    float yolo_latency_ms = 0.0f;
    bool yolo_ran = false;
};

class BenchmarkSuite {
public:
    void add(const BenchmarkSample& s) { samples_.push_back(s); }
    const std::vector<BenchmarkSample>& samples() const { return samples_; }
private:
    std::vector<BenchmarkSample> samples_;
};

// =============================================================================
// Metrics
// =============================================================================
struct Metrics {
    int frame_id = 0;
    float last_crack = 0.0f;
    float fused_crack = 0.0f;
    float sig_conf = 0.0f;
    uint64_t crack_frames = 0;
    uint64_t yolo_count = 0;
    uint64_t uplink_count = 0;
    uint64_t latency_violations = 0;
    float fps = 0.0f;
    float yolo_hz = 0.0f;
    float latency_p50_ms = 0.0f;
    float latency_p95_ms = 0.0f;
    float latency_p99_ms = 0.0f;
    float window_crack_ratio = 0.0f;
    float global_crack_ratio = 0.0f;
    float crack_alert_thr = 0.0f;
    float yolo_age_ms = 0.0f;
    float yolo_conf_thr = 0.0f;
    float spike_bitrate_mbps = 0.0f;
    float avg_yolo_conf = 0.0f;
    float avg_crack_score = 0.0f;
    float avg_agreement = 0.0f;
    float px_to_mm_scale = 0.0f;
    float vis_crack_width_mm = 0.0f;
    float vis_crack_length_mm = 0.0f;
};

// =============================================================================
// Detection / Stats Classes (RESTORED)
// =============================================================================
class CrackStats {
public:
    explicit CrackStats(double window_ms = 5000.0);
    void add_sample(double ts_ms, float crack_score);
    float window_crack_ratio() const;
    float global_crack_ratio() const;

private:
    double window_ms_;
    mutable std::mutex m_;
    std::deque<std::pair<double, float>> samples_;
    uint64_t total_frames_ = 0;
    uint64_t crack_frames_ = 0;
};

class DetectionController {
public:
    explicit DetectionController(double window_ms = 5000.0);
    void add_detection(double ts_ms, float yolo_conf, float crack_score);
    void update_adaptive_thresholds();
    float avg_yolo_conf() const;
    float avg_crack_score() const;
    float avg_agreement() const;
    float yolo_conf_threshold() const;
    float crack_threshold() const;

private:
    double window_ms_;
    mutable std::mutex m_;
    std::deque<std::pair<double, float>> samples_;
    float avg_yolo_conf_ = 0.0f;
    float avg_crack_score_ = 0.0f;
    float avg_agreement_ = 0.0f;
    float yolo_conf_thr_ = 0.40f;
    float crack_thr_ = 0.50f;
};

class DetectionScheduler {
public:
    DetectionScheduler();
    bool should_detect(float crack_score);
private:
    double base_interval_ms_;
    double last_detect_ms_;
};