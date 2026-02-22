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
#include "lockfree_queue.h"

// THIS IS THE KEY: Include the single source of truth for all types
#include "types.h"

// Forward declarations from other modules (classes, not structs)
class SignatureBank;
class GatingEngine;
class FailsafeMonitor;
class YoloManager;
class DetectionController;

// =============================================================================
// MultiRateEngine Class
// =============================================================================
class MultiRateEngine {
public:
    // ✅ ONLY this constructor exists. No default constructor.
    MultiRateEngine(ControlCallback ctrl_cb, UplinkCallback uplink_cb);
    ~MultiRateEngine();

    void start();
    void start(const EngineConfig&);
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
    std::thread t1_, t2_, t3_, t4_, t5_, callback_thread_;
    std::atomic<bool> running_{false};

    // --- Queues ---
    LockFreeQueue<Lane2Job, 64>*      camera_queue_   = nullptr;
    LockFreeQueue<Lane2Job, 64>*      sig_queue_      = nullptr;
    LockFreeQueue<Lane3Job, 32>*      yolo_queue_     = nullptr;
    LockFreeQueue<UplinkPayload,128>* uplink_queue_   = nullptr;
    LockFreeQueue<VisJob, 32>*        vis_queue_      = nullptr;
    LockFreeQueue<CallbackJob, 64>*   callback_queue_ = nullptr;

    // --- Core state ---
    std::atomic<uint64_t> frame_id_{0};
    std::atomic<float>    last_crack_score_{0.0f};
    std::atomic<float>    latest_sig_conf_{0.0f};
    std::atomic<float>    yolo_hz_{0.0f};
    std::atomic<float>    spike_bitrate_mbps_{0.0f};
    std::atomic<float>    px_to_mm_{0.1f};

    // --- Core Components ---
    std::unique_ptr<SignatureBank>     signature_bank_;
    std::unique_ptr<GatingEngine>      gating_engine_;
    std::unique_ptr<FailsafeMonitor>   failsafe_;
    std::unique_ptr<YoloManager>       yolo_manager_;
    std::unique_ptr<DetectionController> det_controller_;
    std::unique_ptr<CrackInferenceEngine> crack_inference_engine_;
    std::unique_ptr<CrackStatisticsTracker> crack_stats_;

    // --- Atomic semantic state ---
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

    std::array<double,1024> latency_ring_{};
    size_t latency_head_ = 0;
    std::atomic<size_t> latency_count_{0};

    // --- Visualization ---
    mutable std::mutex vis_mutex_;
    std::vector<uint8_t> latest_spike_jpeg_;

    // --- Benchmark ---
    mutable std::mutex benchmark_mutex_;
    BenchmarkSuite benchmark_{};

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
    ControlDecision make_decision(const ControlOutput&, float, double) const;
    double semantic_age_ms() const;
    void   update_benchmark(int lane_id, double duration_ms);
};