// =============================================================================
// engine.cpp — AuraSense SFSVC MultiRateEngine (Production Grade)
// =============================================================================

#include "engine.h"
#include "rt_core.h"
#include "signature_bank.hpp"
#include "gating_engine.hpp"
#include "failsafe.hpp"
#include "yolo_manager.h"
#include "yolo_bridge.h"
#include "types.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <memory>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

// =============================================================================
// Internal clock
// =============================================================================
static inline double now_ms() noexcept
{
    using namespace std::chrono;
    return duration<double, std::milli>(
        steady_clock::now().time_since_epoch()).count();
}

// =============================================================================
// Failsafe configuration
// =============================================================================
static const FailsafeSignalConfig kFailsafeSignals[] = {
    { "lane1_latency", 200.0f, 500.0f, 0.0f, 100.0f, 0.0f, 200.0f, true },
    { "yolo_age",      300.0f,1000.0f, 0.0f, 500.0f, 0.0f,1500.0f, true },
    { "signature_age", 300.0f,1000.0f, 0.0f, 500.0f, 0.0f,1500.0f, true }
};

// =============================================================================
// Internal Job Structs
// =============================================================================
struct Lane2Job {
    uint64_t frame_id = 0;
    int height = 0;
    int width  = 0;
    float crack_score = 0.0f;
    std::vector<uint8_t> frame_bgr;
};

struct Lane3Job {
    uint64_t frame_id = 0;
    int height = 0;
    int width  = 0;
    double timestamp = 0.0;
    float crack_score = 0.0f;
    float sparsity = 0.0f;
    float target_hz = 0.0f;
    std::vector<uint8_t> frame_bgr;
};

struct VisJob {
    uint64_t frame_id = 0;
    int frame_h = 0;
    int frame_w = 0;
    float crack_score = 0.0f;
    float sparsity = 0.0f;
    int on_count = 0;
    int off_count = 0;
    std::vector<uint8_t> frame_bgr;
};

// =============================================================================
// Constructor / Destructor
// =============================================================================

MultiRateEngine::MultiRateEngine(ControlCallback ctrl_cb,
                                 UplinkCallback  uplink_cb)
    : ctrl_cb_(std::move(ctrl_cb))
    , uplink_cb_(std::move(uplink_cb))
{
    camera_queue_   = new LockFreeQueue<Lane2Job, 64>();
    sig_queue_      = new LockFreeQueue<Lane2Job, 64>();
    yolo_queue_     = new LockFreeQueue<Lane3Job, 32>();
    uplink_queue_   = new LockFreeQueue<UplinkPayload, 128>();
    vis_queue_      = new LockFreeQueue<VisJob, 32>();
    callback_queue_ = new LockFreeQueue<CallbackJob, 64>();

    crack_stats_ = std::make_unique<CrackStatisticsTracker>(
        5000.0, 0.1f, 3.0f, 1.0f, 0.3f);

    det_controller_ = std::make_unique<DetectionController>(5000.0);

    failsafe_ = std::make_unique<FailsafeMonitor>(
        kFailsafeSignals,
        sizeof(kFailsafeSignals) / sizeof(kFailsafeSignals[0]));

    yolo_manager_ = std::make_unique<YoloManager>(
        AdaptiveYoloConfig{416,234,320,180,640,360,true,true,1.3f});

    signature_bank_ = std::make_unique<SignatureBank>(500,0.30f,3600.0);
    gating_engine_  = std::make_unique<GatingEngine>(0.75f,50,1000.0f);
}

MultiRateEngine::~MultiRateEngine()
{
    stop();

    delete semantic_state_.exchange(nullptr);
    delete last_sig_match_.exchange(nullptr);

    delete camera_queue_;
    delete sig_queue_;
    delete yolo_queue_;
    delete uplink_queue_;
    delete vis_queue_;
    delete callback_queue_;
}

// =============================================================================
// Lifecycle
// =============================================================================

void MultiRateEngine::start()
{
    EngineConfig cfg;
    start(cfg);
}

void MultiRateEngine::start(const EngineConfig& cfg)
{
    if (running_.exchange(true)) return;

    cfg_ = cfg;
    px_to_mm_.store(cfg.px_to_mm_scale);

    start_time_ = now_ms();
    last_yolo_stamp_ = start_time_;
    last_vis_stamp_  = start_time_;

    if (cfg.enable_lane1)
        t1_ = std::thread(&MultiRateEngine::lane1_control,this);
    if (cfg.enable_lane2)
        t2_ = std::thread(&MultiRateEngine::lane2_signature,this);
    if (cfg.enable_lane3)
        t3_ = std::thread(&MultiRateEngine::lane3_yolo,this);
    if (cfg.enable_lane4)
        t4_ = std::thread(&MultiRateEngine::lane4_uplink,this);
    if (cfg.enable_lane5)
        t5_ = std::thread(&MultiRateEngine::lane5_visualize,this);

    if (ctrl_cb_ || uplink_cb_)
        callback_thread_ = std::thread(&MultiRateEngine::lane6_callback_dispatcher,this);
}

void MultiRateEngine::stop()
{
    if (!running_.exchange(false)) return;

    if (t1_.joinable()) t1_.join();
    if (t2_.joinable()) t2_.join();
    if (t3_.joinable()) t3_.join();
    if (t4_.joinable()) t4_.join();
    if (t5_.joinable()) t5_.join();
    if (callback_thread_.joinable()) callback_thread_.join();
}

// =============================================================================
// Frame Ingest
// =============================================================================

void MultiRateEngine::push_frame(const uint8_t* bgr, int h, int w)
{
    if (!running_ || !bgr || h<=0 || w<=0) return;

    Lane2Job job;
    job.frame_id = frame_id_.load();
    job.height = h;
    job.width  = w;
    job.frame_bgr.assign(bgr,bgr+h*w*3);

    camera_queue_->try_push(std::move(job));
}

// =============================================================================
// LANE 1
// =============================================================================

void MultiRateEngine::lane1_control()
{
    const int target_w = rt_core_target_width();
    const int target_h = rt_core_target_height();

    cv::Mat downscaled(target_h,target_w,CV_8UC3);

    while (running_)
    {
        Lane2Job job;
        if (!camera_queue_->try_pop(job)) {
            std::this_thread::yield();
            continue;
        }

        const double t0 = now_ms();

        cv::Mat input(job.height,job.width,CV_8UC3,job.frame_bgr.data());
        cv::resize(input,downscaled,cv::Size(target_w,target_h));

        ControlOutput out =
            rt_core_process_frame_ptr(downscaled.data,target_h,target_w);

        frame_id_.store(out.frame_id);
        last_crack_score_.store(out.crack_score);
        crack_frames_++;

        crack_stats_->update(t0,out.crack_score,out.crack_score,0.0f,out.frame_id);

        ControlDecision decision =
            make_decision(out,latest_sig_conf_.load(),semantic_age_ms());

        if (ctrl_cb_) {
            CallbackJob j;
            j.is_control=true;
            j.ctrl_dec=decision;
            callback_queue_->try_push(std::move(j));
        }

        update_benchmark(1,now_ms()-t0);
    }
}

// =============================================================================
// LANE 2
// =============================================================================

void MultiRateEngine::lane2_signature()
{
    while (running_)
    {
        Lane2Job job;
        if (!sig_queue_->try_pop(job)) {
            std::this_thread::yield();
            continue;
        }

        latest_sig_conf_.store(0.5f); // placeholder
    }
}

// =============================================================================
// LANE 3
// =============================================================================

void MultiRateEngine::lane3_yolo()
{
    while (running_)
    {
        Lane3Job job;
        if (!yolo_queue_->try_pop(job)) {
            std::this_thread::yield();
            continue;
        }

        ++yolo_count_;
    }
}

// =============================================================================
// LANE 4
// =============================================================================

void MultiRateEngine::lane4_uplink()
{
    while (running_)
    {
        UplinkPayload up;
        if (!uplink_queue_->try_pop(up)) {
            std::this_thread::yield();
            continue;
        }

        if (uplink_cb_) {
            CallbackJob j;
            j.is_control=false;
            j.uplink_payload=up;
            callback_queue_->try_push(std::move(j));
        }
    }
}

// =============================================================================
// LANE 5
// =============================================================================

void MultiRateEngine::lane5_visualize()
{
    while (running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

// =============================================================================
// LANE 6
// =============================================================================

void MultiRateEngine::lane6_callback_dispatcher()
{
    while (running_)
    {
        CallbackJob job;
        if (!callback_queue_->try_pop(job)) {
            std::this_thread::yield();
            continue;
        }

        if (job.is_control) {
            if (ctrl_cb_) ctrl_cb_(job.ctrl_dec);
        } else {
            if (uplink_cb_) uplink_cb_(job.uplink_payload);
            ++uplink_count_;   // ✅ only increment here
        }
    }
}

// =============================================================================
// Helpers
// =============================================================================

double MultiRateEngine::semantic_age_ms() const
{
    const SemanticState* s =
        semantic_state_.load();
    if (!s) return 99999.0;
    return now_ms() - s->timestamp_ms;
}

ControlDecision MultiRateEngine::make_decision(
    const ControlOutput& rt_out,
    float sig_conf,
    double sem_age_ms) const
{
    ControlDecision d{};
    d.frame_id = rt_out.frame_id;
    d.timestamp = now_ms();
    d.throttle = rt_out.throttle;
    d.steer = rt_out.steer;
    d.crack_score = rt_out.crack_score;
    d.confidence = sig_conf;
    d.semantic_age_ms = sem_age_ms;
    return d;
}

void MultiRateEngine::update_benchmark(int lane_id,double duration_ms)
{
    std::lock_guard<std::mutex> lk(benchmark_mutex_);
    const double alpha=0.05;
    if (lane_id==1)
        benchmark_.lane1_avg_ms =
            (1-alpha)*benchmark_.lane1_avg_ms + alpha*duration_ms;
}

// =============================================================================
// Metrics
// =============================================================================

Metrics MultiRateEngine::get_metrics() const
{
    Metrics m{};
    m.frame_id = frame_id_.load();
    m.last_crack = last_crack_score_.load();
    m.crack_frames = crack_frames_.load();
    m.yolo_count = yolo_count_.load();
    m.uplink_count = uplink_count_.load();

    m.window_crack_ratio = crack_stats_->get_window_crack_ratio();
    m.global_crack_ratio = crack_stats_->get_global_crack_ratio();

    return m;
}

BenchmarkSuite MultiRateEngine::get_benchmark_suite() const
{
    std::lock_guard<std::mutex> lk(benchmark_mutex_);
    return benchmark_;
}

std::vector<uint8_t> MultiRateEngine::get_spike_frame_jpeg() const
{
    std::lock_guard<std::mutex> lk(vis_mutex_);
    return latest_spike_jpeg_;
}

void MultiRateEngine::print_stats() const
{
    Metrics m = get_metrics();
    std::printf("[Engine] frame=%llu crack=%.3f\n",
        (unsigned long long)m.frame_id,
        m.last_crack);
}

void MultiRateEngine::_emergency_stop()
{
    running_=false;
}