// engine.h
#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>
#include <array>
#include <memory>

// Core C API
#include "rt_core.h"

// Advanced inference & statistics
#include "crack_inference.hpp"
#include "crack_statistics.hpp"

// Forward declarations from other modules
struct SignatureMatch;
struct SemanticState;
class SignatureBank;
class GatingEngine;
class FailsafeMonitor;
class YoloManager;
class CrackStatistics;
class DetectionController;

// =============================================================================
// Public Data Structures
// =============================================================================

struct ControlDecision {
    uint64_t frame_id        = 0;
    double   timestamp       = 0.0;
    float    throttle        = 0.0f;
    float    steer           = 0.0f;
    float    crack_score     = 0.0f;
    float    sparsity        = 0.0f;
    float    confidence      = 0.0f;  // Signature confidence
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

struct BenchmarkSuite {
    double lane1_avg_ms = 0.0;
    double lane2_avg_ms = 0.0;
    double lane3_avg_ms = 0.0;
    double lane4_avg_ms = 0.0;
    double lane5_avg_ms = 0.0;
};

// Non-blocking callback dispatch jobs
struct CallbackJob {
    ControlDecision ctrl_dec;
    UplinkPayload   uplink_payload;
    bool            is_control = true;
};

struct EngineConfig {
    bool  enable_lane1           = true;
    bool  enable_lane2           = true;
    bool  enable_lane3           = true;
    bool  enable_lane4           = true;
    bool  enable_lane5           = true;
    float max_control_latency_ms = 50.0f; // Configurable latency threshold
    float px_to_mm_scale         = 0.1f;
};

using ControlCallback = std::function<void(const ControlDecision&)>;
using UplinkCallback  = std::function<void(const UplinkPayload&)>;

// Forward declare internal job structs
struct Lane2Job;
struct Lane3Job;
struct VisJob;

// Simple lock-free queue template (provided elsewhere)
template<typename T> class LockFreeQueue;

class MultiRateEngine {
public:
    MultiRateEngine(ControlCallback ctrl_cb, UplinkCallback uplink_cb);
    ~MultiRateEngine();

    void start();                     // default: all lanes
    void start(const EngineConfig&);  // config-driven start
    void stop();

    void push_frame(const uint8_t* bgr, int h, int w);

    Metrics              get_metrics() const;
    BenchmarkSuite       get_benchmark_suite() const;
    std::vector<uint8_t> get_spike_frame_jpeg() const;

    void print_stats() const;
    void _emergency_stop();

    void set_px_to_mm(float px_to_mm);

private:
    // --- Threads ---
    std::thread       t1_, t2_, t3_, t4_, t5_, callback_thread_;
    std::atomic<bool> running_{false};

    // --- Queues ---
    LockFreeQueue<Lane2Job>*      camera_queue_   = nullptr;
    LockFreeQueue<Lane2Job>*      sig_queue_      = nullptr;
    LockFreeQueue<Lane3Job>*      yolo_queue_     = nullptr;
    LockFreeQueue<UplinkPayload>* uplink_queue_   = nullptr;
    LockFreeQueue<VisJob>*        vis_queue_      = nullptr;
    LockFreeQueue<CallbackJob>*   callback_queue_ = nullptr;

    // --- Core State ---
    std::atomic<uint64_t> frame_id_{0};
    std::atomic<float>    last_crack_score_{0.0f};
    std::atomic<float>    latest_sig_conf_{0.0f};
    std::atomic<float>    yolo_hz_{0.0f};
    std::atomic<float>    spike_bitrate_mbps_{0.0f};
    std::atomic<float>    px_to_mm_{0.1f};

    std::unique_ptr<SignatureBank>     signature_bank_;
    std::unique_ptr<GatingEngine>      gating_engine_;
    std::unique_ptr<FailsafeMonitor>   failsafe_;
    std::unique_ptr<CrackStatistics>   crack_stats_;
    std::unique_ptr<YoloManager>       yolo_manager_;
    std::unique_ptr<DetectionController> det_controller_;

    // --- NEW: Advanced Inference & Statistics ---
    CrackInferenceEngine   crack_inference_engine_;
    CrackStatisticsTracker crack_statistics_tracker_;

    // --- Atomic pointers for latest data ---
    std::atomic<SemanticState*>  semantic_state_{nullptr};
    std::atomic<SignatureMatch*> last_sig_match_{nullptr};

    // --- Metrics & Timing ---
    double              start_time_      = 0.0;
    double              last_yolo_stamp_ = 0.0;
    double              last_vis_stamp_  = 0.0;
    std::atomic<uint64_t> crack_frames_{0};
    std::atomic<uint64_t> yolo_count_{0};
    std::atomic<uint64_t> uplink_count_{0};
    std::atomic<uint64_t> latency_violations_{0};

    std::array<double, 1024> latency_ring_{};
    size_t                   latency_head_  = 0;
    std::atomic<size_t>      latency_count_{0};

    // --- Visualization ---
    mutable std::mutex   vis_mutex_;
    std::vector<uint8_t> latest_spike_jpeg_;

    // --- Benchmarking ---
    mutable std::mutex benchmark_mutex_;
    BenchmarkSuite      benchmark_{};

    // --- Configuration ---
    EngineConfig cfg_{};

    // --- Callbacks ---
    ControlCallback ctrl_cb_;
    UplinkCallback  uplink_cb_;

    // --- Lane Functions ---
    void lane1_control();
    void lane2_signature();
    void lane3_yolo();
    void lane4_uplink();
    void lane5_visualize();
    void lane6_callback_dispatcher();

    // --- Helpers ---
    ControlDecision make_decision(const ControlOutput& rt_out,
                                  float sig_conf,
                                  double sem_age_ms) const;
    double semantic_age_ms() const;
    void   update_benchmark(int lane_id, double duration_ms);
};