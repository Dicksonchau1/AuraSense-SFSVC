#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

#include <opencv2/core.hpp>

#include "types.h"
#include "lockfree_queue.h"
#include "failsafe.hpp"
#include "adaptive_yolo.hpp"
#include "gating_engine.hpp"
#include "signature_bank.hpp"

extern "C" {
#include "rt_core.h"
}

static constexpr size_t CAMERA_QUEUE_CAP  = 64;
static constexpr size_t SIG_QUEUE_CAP     = 64;
static constexpr size_t YOLO_QUEUE_CAP    = 32;
static constexpr size_t CTRL_QUEUE_CAP    = 64;
static constexpr size_t UPLINK_QUEUE_CAP  = 128;
static constexpr size_t VIS_QUEUE_CAP     = 32;

class MultiRateEngine
{
public:
    using ControlCallback = std::function<void(const ControlDecision&)>;
    using UplinkCallback  = std::function<void(const UplinkPayload&)>;

    MultiRateEngine(ControlCallback ctrl_cb,
                    UplinkCallback  uplink_cb);

    MultiRateEngine(ControlCallback ctrl_cb,
                    UplinkCallback  uplink_cb,
                    int frame_height,
                    int frame_width);

    ~MultiRateEngine();

    void start();
    void stop();
    void push_frame(const uint8_t* bgr, int h, int w);

    void set_px_to_mm(float px_to_mm);

    Metrics              get_metrics() const;
    BenchmarkSuite       get_benchmark_suite() const;
    std::vector<uint8_t> get_spike_frame_jpeg() const;

    void print_stats() const;

private:
    void lane1_control();
    void lane2_signature();
    void lane3_yolo();
    void lane4_uplink();
    void lane5_visualize();

    double semantic_age_ms() const;

    ControlDecision make_decision(
        const ControlOutput& rt_out,
        float sig_conf,
        double sem_age_ms) const;

    CrackMetrics convert_crack_metrics(float crack_score) const;

    void add_detection_sample(double ts_ms,
                              float yolo_conf,
                              float crack_score,
                              float agreement);

    void record_benchmark_sample(const BenchmarkSample& sample);

    void _emergency_stop();

private:
    std::atomic<bool> running_{false};
    std::atomic<int>  frame_id_{0};

    int frame_height_{0};
    int frame_width_{0};

    double start_time_{0.0};

    ControlCallback ctrl_cb_;
    UplinkCallback  uplink_cb_;

    std::thread t1_, t2_, t3_, t4_, t5_;

    LockFreeQueue<Lane2Job,        CAMERA_QUEUE_CAP> camera_queue_;
    LockFreeQueue<Lane2Job,        SIG_QUEUE_CAP>    sig_queue_;
    LockFreeQueue<Lane3Job,        YOLO_QUEUE_CAP>   yolo_queue_;
    LockFreeQueue<ControlDecision, CTRL_QUEUE_CAP>   ctrl_queue_;
    LockFreeQueue<UplinkPayload,   UPLINK_QUEUE_CAP> uplink_queue_;
    LockFreeQueue<VisJob,          VIS_QUEUE_CAP>    vis_queue_;

    CrackStats          crack_stats_;
    DetectionController det_controller_;
    DetectionScheduler  detection_scheduler_;

    std::atomic<float> last_crack_score_{0.0f};
    std::atomic<float> latest_sig_conf_{0.0f};

    std::atomic<uint64_t> crack_frames_{0};
    std::atomic<uint64_t> yolo_count_{0};
    std::atomic<uint64_t> uplink_count_{0};
    std::atomic<uint64_t> latency_violations_{0};
    std::atomic<uint64_t> sig_updates_{0};

    std::atomic<float> avg_yolo_conf_{0.0f};
    std::atomic<float> avg_crack_score_{0.0f};
    std::atomic<float> avg_agreement_{0.0f};

    std::atomic<float> px_to_mm_{0.0f};
    std::atomic<float> vis_crack_width_mm_{0.0f};
    std::atomic<float> vis_crack_length_mm_{0.0f};

    // Semantic and signature state
    std::atomic<SemanticState*>  semantic_state_{nullptr};
    std::atomic<SignatureMatch*> last_sig_match_{nullptr};

    // YOLO / semantic metrics
    std::atomic<float> yolo_hz_{0.0f};
    double             last_yolo_stamp_{0.0};

    // Spike / visualization metrics
    std::atomic<float> spike_bitrate_mbps_{0.0f};
    double             last_vis_stamp_{0.0};

    std::array<double, 512> latency_ring_{};
    size_t latency_head_{0};
    size_t latency_count_{0};
    double max_control_latency_ms_{50.0};

    mutable std::mutex   vis_mutex_;
    std::vector<uint8_t> latest_spike_jpeg_;

    mutable std::mutex benchmark_mutex_;
    BenchmarkSuite      benchmark_;

    FailsafeMonitor     failsafe_;
    AdaptiveYoloManager yolo_manager_;
    SignatureBank*      signature_bank_;
    GatingEngine        gating_engine_;
};