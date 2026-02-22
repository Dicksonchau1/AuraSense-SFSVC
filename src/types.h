// types.h
#pragma once

#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <utility>
#include <vector>
#include <functional> // Required for std::function

// =============================================================================
// Failsafe Status
// =============================================================================
enum class FailsafeStatus : int {
    OK       = 0,
    WARNING  = 1,
    CRITICAL = 2
};

// =============================================================================
// Core Job & Data Structures
// =============================================================================

struct ControlDecision {
    uint64_t frame_id        = 0;
    double   timestamp       = 0.0;
    float    throttle        = 0.0f;
    float    steer           = 0.0f;
    float    crack_score     = 0.0f;
    float    sparsity        = 0.0f;
    float    confidence      = 0.0f;
    double   semantic_age_ms = 0.0;
    bool     is_null_cycle        = true;
    bool     inference_suppressed = false;
    bool     event_only_mode      = false;
    double   reference_frame_age  = 0.0;
    bool     yolo_active          = false;
    double   yolo_age_ms          = 0.0;
    double   encode_time_ms       = 0.0;
};

struct UplinkPayload {
    uint64_t frame_id           = 0;
    double   timestamp          = 0.0;
    float    throttle           = 0.0f;
    float    steer              = 0.0f;
    float    crack_score        = 0.0f;
    float    sparsity           = 0.0f;
    double   control_latency_ms = 0.0;
};

struct Lane2Job {
    uint64_t frame_id    = 0;
    int      height      = 0;
    int      width       = 0;
    float    crack_score = 0.0f;
    std::vector<uint8_t> frame_bgr;
};

struct Lane3Job {
    uint64_t frame_id    = 0;
    int      height      = 0;
    int      width       = 0;
    double   timestamp   = 0.0;
    float    crack_score = 0.0f;
    float    sparsity    = 0.0f;
    float    target_hz   = 0.0f;
    std::vector<uint8_t> frame_bgr;
};

struct VisJob {
    uint64_t frame_id    = 0;
    int      frame_h     = 0;
    int      frame_w     = 0;
    float    crack_score = 0.0f;
    float    sparsity    = 0.0f;
    int      on_count    = 0;
    int      off_count   = 0;
    std::vector<uint8_t> frame_bgr;
};

struct SemanticState {
    uint64_t frame_id     = 0;
    double   timestamp_ms = 0.0;
    double   latency_ms   = 0.0;
    float    front_risk   = 0.0f;
    float    left_risk    = 0.0f;
    float    right_risk   = 0.0f;
    float    crack_risk   = 0.0f;
    int      num_detections = 0;
    float    max_confidence = 0.0f;
    float    agreement    = 0.0f;
};

struct SignatureMatch {
    bool matched = false;
    float confidence = 0.0f;
    int id = -1;
    float crack_score = 0.0f;
};

// =============================================================================
// Public Configuration & Callback Types
// =============================================================================

struct EngineConfig {
    bool  enable_lane1           = true;
    bool  enable_lane2           = true;
    bool  enable_lane3           = true;
    bool  enable_lane4           = true;
    bool  enable_lane5           = true;
    float max_control_latency_ms = 50.0f;
    float px_to_mm_scale         = 0.1f;
};

struct CallbackJob {
    ControlDecision ctrl_dec;
    UplinkPayload   uplink_payload;
    bool            is_control = true;
};

using ControlCallback = std::function<void(const ControlDecision&)>;
using UplinkCallback  = std::function<void(const UplinkPayload&)>;

// =============================================================================
// Metrics
// =============================================================================
struct Metrics {
    uint64_t frame_id           = 0;
    float    last_crack         = 0.0f;
    float    fused_crack        = 0.0f;
    float    sig_conf           = 0.0f;
    uint64_t crack_frames       = 0;
    uint64_t yolo_count         = 0;
    uint64_t uplink_count       = 0;
    uint64_t latency_violations = 0;
    float    fps                = 0.0f;
    double   latency_p50_ms     = 0.0;
    double   latency_p95_ms     = 0.0;
    double   latency_p99_ms     = 0.0;
    float    window_crack_ratio = 0.0f;
    float    global_crack_ratio = 0.0f;
    float    yolo_hz            = 0.0f;
    float    yolo_age_ms        = 0.0f;
    float    spike_bitrate_mbps = 0.0f;
    float    crack_alert_thr    = 0.0f;
    float    yolo_conf_thr      = 0.0f;
    float    avg_yolo_conf      = 0.0f;
    float    avg_crack_score    = 0.0f;
    float    avg_agreement      = 0.0f;
    float    px_to_mm_scale     = 0.0f;
    float    vis_crack_width_mm = 0.0f;
    float    vis_crack_length_mm= 0.0f;
};

// =============================================================================
// Benchmarking
// =============================================================================
struct BenchmarkSuite {
    double lane1_avg_ms = 0.0;
    double lane2_avg_ms = 0.0;
    double lane3_avg_ms = 0.0;
    double lane4_avg_ms = 0.0;
    double lane5_avg_ms = 0.0;
};