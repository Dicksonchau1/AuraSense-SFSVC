// =============================================================================
// engine.cpp — AuraSense SFSVC MultiRateEngine (Production Grade)
// =============================================================================

#include "engine.h"
#include "rt_core.h"
#include "signature_bank.h"
#include "gating_engine.h"
#include "failsafe.h"
#include "detection_controller.h"
#include "crack_statistics.h"
#include "yolo_manager.h"
#include "yolo_bridge.h" // Real YOLO implementation
#include "types.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <vector>
#include <memory>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

// Use your real lock-free queue implementation
#include "lockfree_queue.h"

// =============================================================================
// Internal clock — nanosecond-resolution, monotonic
// =============================================================================
static inline double now_ms() noexcept
{
    using namespace std::chrono;
    return duration<double, std::milli>(
        steady_clock::now().time_since_epoch()).count();
}

// =============================================================================
// Failsafe configuration (index-based API)
// =============================================================================
static const FailsafeSignalConfig kFailsafeSignals[] = {
    { "lane1_latency", 200.0f, 500.0f,
      0.0f, 100.0f,
      0.0f, 200.0f,
      true },

    { "yolo_age", 300.0f, 1000.0f,
      0.0f, 500.0f,
      0.0f, 1500.0f,
      true },

    { "signature_age", 300.0f, 1000.0f,
      0.0f, 500.0f,
      0.0f, 1500.0f,
      true }
};

// =============================================================================
// Job Structures for Inter-Lane Communication
// =============================================================================
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

// =============================================================================
// Constructors / Destructor
// =============================================================================

MultiRateEngine::MultiRateEngine(ControlCallback ctrl_cb,
                                 UplinkCallback  uplink_cb)
    : ctrl_cb_(std::move(ctrl_cb))
    , uplink_cb_(std::move(uplink_cb))
    // NEW: initialize advanced statistics tracker with sane defaults
    , crack_statistics_tracker_(
        5000.0,   // window_ms
        0.1f,     // px_to_mm_scale (will be updated from config)
        3.0f,     // critical_width_mm
        1.0f,     // warning_width_mm
        0.3f      // hairline_width_mm
      )
{
    // Allocate queues using your LockFreeQueue implementation
    camera_queue_    = new LockFreeQueue<Lane2Job, 64>();
    sig_queue_       = new LockFreeQueue<Lane2Job, 64>();
    yolo_queue_      = new LockFreeQueue<Lane3Job, 32>();
    uplink_queue_    = new LockFreeQueue<UplinkPayload, 128>();
    vis_queue_       = new LockFreeQueue<VisJob, 32>();
    callback_queue_  = new LockFreeQueue<CallbackJob, 64>();

    // Allocate core components using unique_ptr for safety
    crack_stats_     = std::make_unique<CrackStatistics>(5000.0);
    det_controller_  = std::make_unique<DetectionController>(5000.0);
    failsafe_        = std::make_unique<FailsafeMonitor>(kFailsafeSignals,
                        sizeof(kFailsafeSignals) / sizeof(kFailsafeSignals[0]));
    yolo_manager_    = std::make_unique<YoloManager>(
                        AdaptiveYoloConfig{416, 234, 320, 180, 640, 360, true, true, 1.3f});
    signature_bank_  = std::make_unique<SignatureBank>(500, 0.30f, 3600.0);
    gating_engine_   = std::make_unique<GatingEngine>(0.75f, 50, 1000.0f);
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
    px_to_mm_.store(cfg.px_to_mm_scale, std::memory_order_relaxed);

    start_time_      = now_ms();
    last_yolo_stamp_ = start_time_;
    last_vis_stamp_  = start_time_;

    if (cfg.enable_lane1)
        t1_ = std::thread(&MultiRateEngine::lane1_control,   this);
    if (cfg.enable_lane2)
        t2_ = std::thread(&MultiRateEngine::lane2_signature, this);
    if (cfg.enable_lane3)
        t3_ = std::thread(&MultiRateEngine::lane3_yolo,      this);
    if (cfg.enable_lane4)
        t4_ = std::thread(&MultiRateEngine::lane4_uplink,    this);
    if (cfg.enable_lane5)
        t5_ = std::thread(&MultiRateEngine::lane5_visualize, this);

    if (ctrl_cb_ || uplink_cb_)
        callback_thread_ = std::thread(&MultiRateEngine::lane6_callback_dispatcher, this);
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
// Frame Ingest — Camera → Lane 1
// =============================================================================

void MultiRateEngine::push_frame(const uint8_t* bgr, int h, int w)
{
    if (!running_.load(std::memory_order_relaxed)) return;
    if (!bgr || h <= 0 || w <= 0)         return;

    Lane2Job job;
    job.frame_id    = frame_id_.load(std::memory_order_acquire);
    job.height      = h;
    job.width       = w;
    job.crack_score = 0.0f;
    job.frame_bgr.assign(bgr, bgr + h * w * 3);

    camera_queue_->try_push(std::move(job));
}

// =============================================================================
// LANE 1 — RT Bridge (fan-out)
// =============================================================================

void MultiRateEngine::lane1_control()
{
    const bool pinned = rt_core_pin_thread(2) != 0;
    if (!pinned) {
        std::fprintf(stderr,
            "[Lane1] WARNING: rt_core_pin_thread(2) failed "
            "(no RT priority on this platform)\n");
    }

    const int target_w = rt_core_target_width();
    const int target_h = rt_core_target_height();

    cv::Mat downscaled(target_h, target_w, CV_8UC3);

    while (running_.load(std::memory_order_relaxed))
    {
        Lane2Job job;
        if (!camera_queue_->try_pop(job)) {
            std::this_thread::yield();
            continue;
        }

        const double t_start = now_ms();

        // Downscale once per frame
        cv::Mat input(job.height, job.width,
                      CV_8UC3, job.frame_bgr.data());
        cv::resize(input, downscaled,
                   cv::Size(target_w, target_h),
                   0.0, 0.0, cv::INTER_AREA);

        // RT core — single hot call
        ControlOutput out =
            rt_core_process_frame_ptr(
                downscaled.data, target_h, target_w);

        // --- NEW: Advanced Crack Fusion ---
        const float sig_conf_lane1 = latest_sig_conf_.load(std::memory_order_relaxed);

        CrackInferenceOutput inference_out = crack_inference_engine_.update(
            out.crack_score,   // raw_crack from rt_core
            out.sparsity,      // sparsity
            0.5f,              // luminance (placeholder, real value later)
            sig_conf_lane1     // signature confidence
        );

        out.fused_crack_score = inference_out.fused_probability;

        CrackStatistics current_stats = crack_statistics_tracker_.update(
            t_start,
            out.crack_score,
            out.fused_crack_score,
            0.0f,              // yolo_crack_confidence (lane3 will refine)
            out.frame_id
        );
        // --- End of NEW Section ---

        frame_id_.store(out.frame_id,
                        std::memory_order_release);

        last_crack_score_.store(out.fused_crack_score,
                                std::memory_order_relaxed);

        crack_stats_->add_sample(t_start, out.crack_score);
        ++crack_frames_;

        // Control decision
        const float  sig_conf = latest_sig_conf_.load(
                                    std::memory_order_relaxed);
        const double sem_age  = semantic_age_ms();
        ControlDecision decision =
            make_decision(out, sig_conf, sem_age);

        if (ctrl_cb_) {
            CallbackJob cb_job;
            cb_job.is_control = true;
            cb_job.ctrl_dec   = decision;
            callback_queue_->try_push(std::move(cb_job));
        }

        // Fan-out to Lane 2 (signature)
        Lane2Job sig_job = job;
        sig_job.frame_id    = out.frame_id;
        sig_job.crack_score = out.crack_score;
        sig_queue_->try_push(std::move(sig_job));

        // Fan-out to Lane 3 (YOLO)
        {
            Lane3Job yj{};
            yj.frame_id    = out.frame_id;
            yj.height      = target_h;
            yj.width       = target_w;
            yj.timestamp   = t_start;
            yj.crack_score = out.crack_score;
            yj.sparsity    = out.sparsity;
            yj.target_hz   = out.yolo_target_hz;
            yj.frame_bgr.assign(downscaled.data,
                                downscaled.data +
                                    target_w * target_h * 3);
            yolo_queue_->try_push(std::move(yj));
        }

        // Fan-out to Lane 5 (vis)
        {
            VisJob vj{};
            vj.frame_id    = out.frame_id;
            vj.frame_h     = job.height;
            vj.frame_w     = job.width;
            vj.crack_score = out.crack_score;
            vj.sparsity    = out.sparsity;
            vj.on_count    = static_cast<int>(out.on_spike_count);
            vj.off_count   = static_cast<int>(out.off_spike_count);
            vj.frame_bgr   = std::move(job.frame_bgr);
            vis_queue_->try_push(std::move(vj));
        }

        // Latency + failsafe
        const double latency = now_ms() - t_start;

        if (out.frame_id > 300) {
            latency_ring_[latency_head_] = latency;
            latency_head_ = (latency_head_ + 1) % latency_ring_.size();
            if (latency_count_ < latency_ring_.size())
                ++latency_count_;
            if (latency > cfg_.max_control_latency_ms)
                ++latency_violations_;
        }

        failsafe_->update(0, static_cast<float>(latency));

        // Fan-out to Lane 4 (uplink)
        {
            UplinkPayload up{};
            up.frame_id           = out.frame_id;
            up.timestamp          = t_start / 1000.0;
            up.throttle           = out.throttle;
            up.steer              = out.steer;
            up.crack_score        = out.crack_score;
            up.sparsity           = out.sparsity;
            up.control_latency_ms = latency;
            uplink_queue_->try_push(std::move(up));
        }

        update_benchmark(1, latency);
    }
}

// =============================================================================
// Lane 2 — Signature (real confidence from SignatureBank)
// =============================================================================

void MultiRateEngine::lane2_signature()
{
    while (running_.load(std::memory_order_relaxed))
    {
        Lane2Job job;
        if (!sig_queue_->try_pop(job)) {
            std::this_thread::yield();
            continue;
        }
        if (job.frame_bgr.empty()) continue;

        const double t_start = now_ms();

        cv::Mat frame_gray, hist;
        cv::cvtColor(cv::Mat(job.height, job.width, CV_8UC3, job.frame_bgr.data()),
                     frame_gray, cv::COLOR_BGR2GRAY);
        int histSize = 256;
        float range[] = { 0, 256 };
        const float* histRange = { range };
        cv::calcHist(&frame_gray, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);
        cv::normalize(hist, hist, 0, 1, cv::NORM_L1);
        std::vector<float> descriptor;
        hist.copyTo(descriptor);

        MatchResult raw_match =
            signature_bank_->find_match_full(
                descriptor, {}, {}, {}, job.crack_score);

        SignatureMatch match =
            signature_bank_->to_signature_match(raw_match);

        latest_sig_conf_.store(match.confidence,
                               std::memory_order_relaxed);

        auto new_match = std::make_unique<SignatureMatch>(match);
        SignatureMatch* old_match =
            last_sig_match_.exchange(new_match.release(), std::memory_order_acq_rel);
        delete old_match;

        update_benchmark(2, now_ms() - t_start);
    }
}

// =============================================================================
// Lane 3 — YOLO + gating (real Hz + age metrics)
// =============================================================================

void MultiRateEngine::lane3_yolo()
{
    while (running_.load(std::memory_order_relaxed))
    {
        Lane3Job job;
        if (!yolo_queue_->try_pop(job)) {
            std::this_thread::yield();
            continue;
        }

        const double t0 = now_ms();

        SignatureMatch sig_match{};
        if (auto* sm = last_sig_match_.load(std::memory_order_acquire))
            sig_match = *sm;

        GatingDecision gd =
            gating_engine_->decide(
                sig_match,
                job.frame_id,
                t0,
                job.crack_score,
                false);

        if (!gd.should_infer)
            continue;

        cv::Mat yolo_input(job.height, job.width, CV_8UC3, job.frame_bgr.data());
        YoloResult yolo_result = run_yolo(yolo_input, t0);

        rt_core_yolo_publish(
            t0 / 1000.0,
            yolo_result.front_risk,
            yolo_result.left_risk,
            yolo_result.right_risk,
            yolo_result.crack_risk,
            yolo_result.min_distance_m,
            yolo_result.max_confidence,
            yolo_result.num_detections,
            yolo_result.priority_detections,
            yolo_result.num_filtered_out);

        det_controller_->add_detection(t0, yolo_result.max_confidence, job.crack_score);
        det_controller_->update_adaptive_thresholds();

        ++yolo_count_;
        const double prev = last_yolo_stamp_;
        last_yolo_stamp_  = t0;
        if (prev > 0.0) {
            const double dt_s = (t0 - prev) / 1000.0;
            if (dt_s > 0.0) {
                const float hz = static_cast<float>(1.0 / dt_s);
                yolo_hz_.store(hz, std::memory_order_relaxed);
            }
        }

        auto s = std::make_unique<SemanticState>();
        s->frame_id       = job.frame_id;
        s->timestamp_ms   = t0;
        s->latency_ms     = 0.0;
        s->front_risk     = yolo_result.front_risk;
        s->left_risk      = yolo_result.left_risk;
        s->right_risk     = yolo_result.right_risk;
        s->crack_risk     = yolo_result.crack_risk;
        s->num_detections = yolo_result.num_detections;
        s->max_confidence = yolo_result.max_confidence;
        s->agreement      = (yolo_result.crack_risk > 0.2f && job.crack_score > 0.2f) ? 1.0f : 0.0f;

        SemanticState* old =
            semantic_state_.exchange(s.release(), std::memory_order_acq_rel);
        delete old;

        update_benchmark(3, now_ms() - t0);
    }
}

// =============================================================================
// Lane 4 — Uplink
// =============================================================================

void MultiRateEngine::lane4_uplink()
{
    while (running_.load(std::memory_order_relaxed))
    {
        UplinkPayload up;
        if (!uplink_queue_->try_pop(up)) {
            std::this_thread::yield();
            continue;
        }

        ++uplink_count_;

        if (uplink_cb_) {
            CallbackJob cb_job;
            cb_job.is_control     = false;
            cb_job.uplink_payload = up;
            callback_queue_->try_push(std::move(cb_job));
        }
        update_benchmark(4, 0.0);
    }
}

// =============================================================================
// Lane 5 — Visualization
// =============================================================================

void MultiRateEngine::lane5_visualize()
{
    cv::Mat vis;

    while (running_.load(std::memory_order_relaxed))
    {
        VisJob job;
        if (!vis_queue_->try_pop(job)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }
        if (job.frame_bgr.empty() || job.frame_h <= 0 || job.frame_w <= 0)
            continue;

        const double t_start = now_ms();

        cv::Mat frame(job.frame_h, job.frame_w, CV_8UC3, job.frame_bgr.data());

        vis = cv::Mat(job.frame_h, job.frame_w * 2, CV_8UC3);
        frame.copyTo(vis(cv::Rect(0, 0, job.frame_w, job.frame_h)));

        cv::Mat right_half = vis(cv::Rect(job.frame_w, 0, job.frame_w, job.frame_h));
        const float cs = std::max(0.0f, std::min(1.0f, job.crack_score));
        if (cs > 0.0f) {
            cv::Mat tint(right_half.size(), right_half.type(), cv::Scalar(0, 0, 255));
            cv::addWeighted(tint, cs, right_half, 1.0 - cs, 0.0, right_half);
        }

        const cv::Scalar box_color(0, 0, 0);
        const cv::Scalar text_color(200, 255, 200);
        const cv::Scalar warn_color(0, 255, 255);
        const cv::Scalar crit_color(0, 0, 255);

        const int box_w = 360;
        const int box_h = 80;
        cv::rectangle(vis, cv::Rect(10, 10, box_w, box_h), box_color, cv::FILLED);

        char buf[160];
        std::snprintf(buf, sizeof(buf), "Crack: %.3f  Spikes: %d / %d",
                      job.crack_score, job.on_count, job.off_count);
        cv::putText(vis, buf, cv::Point(20, 40), cv::FONT_HERSHEY_SIMPLEX,
                    0.6, text_color, 1, cv::LINE_AA);

        const char* sev_label = "OK";
        cv::Scalar sev_color  = text_color;
        if (cs > 0.33f) { sev_label = "CRITICAL"; sev_color = crit_color; }
        else if (cs > 0.11f) { sev_label = "WARNING"; sev_color = warn_color; }

        std::snprintf(buf, sizeof(buf), "Severity: %s  Sparsity: %.3f",
                      sev_label, job.sparsity);
        cv::putText(vis, buf, cv::Point(20, 65), cv::FONT_HERSHEY_SIMPLEX,
                    0.6, sev_color, 1, cv::LINE_AA);

        std::vector<uint8_t> jpeg;
        if (cv::imencode(".jpg", vis, jpeg)) {
            {
                std::lock_guard<std::mutex> lk(vis_mutex_);
                latest_spike_jpeg_ = jpeg;
            }

            const double bits = static_cast<double>(jpeg.size()) * 8.0;
            const double now  = now_ms();
            const double dt_s = (now - last_vis_stamp_) / 1000.0;
            last_vis_stamp_   = now;
            if (dt_s > 0.0) {
                const double mbps = (bits / 1e6) / dt_s;
                spike_bitrate_mbps_.store(static_cast<float>(mbps),
                                          std::memory_order_relaxed);
            }
        }
        update_benchmark(5, now_ms() - t_start);
    }
}

// =============================================================================
// Lane 6 — Callback Dispatcher
// =============================================================================

void MultiRateEngine::lane6_callback_dispatcher()
{
    while (running_.load(std::memory_order_relaxed)) {
        CallbackJob job;
        if (!callback_queue_->try_pop(job)) {
            std::this_thread::yield();
            continue;
        }

        if (job.is_control) {
            if (ctrl_cb_) ctrl_cb_(job.ctrl_dec);
        } else {
            if (uplink_cb_) uplink_cb_(job.uplink_payload);
        }
        ++uplink_count_;
    }
}

// =============================================================================
// Helpers
// =============================================================================

double MultiRateEngine::semantic_age_ms() const
{
    const SemanticState* s =
        semantic_state_.load(std::memory_order_acquire);
    if (!s) return 99999.0;
    return now_ms() - s->timestamp_ms;
}

ControlDecision MultiRateEngine::make_decision(
    const ControlOutput& rt_out,
    float                sig_conf,
    double               sem_age_ms) const
{
    ControlDecision d{};

    d.frame_id        = rt_out.frame_id;
    d.timestamp       = now_ms();
    d.throttle        = rt_out.throttle;
    d.steer           = rt_out.steer;
    d.crack_score     = rt_out.fused_crack_score;
    d.sparsity        = rt_out.sparsity;
    d.confidence      = sig_conf;
    d.semantic_age_ms = sem_age_ms;

    d.is_null_cycle        = rt_out.is_null_cycle;
    d.inference_suppressed = rt_out.inference_suppressed;
    d.event_only_mode      = rt_out.event_only_mode;

    d.reference_frame_age = rt_out.reference_frame_age;
    d.yolo_active         = rt_out.yolo_active;
    d.yolo_age_ms         = rt_out.yolo_age_ms;
    d.encode_time_ms      = rt_out.encode_time_ms;

    return d;
}

void MultiRateEngine::update_benchmark(int lane_id, double duration_ms)
{
    std::lock_guard<std::mutex> lk(benchmark_mutex_);
    const double alpha = 0.05;
    switch(lane_id) {
        case 1: benchmark_.lane1_avg_ms = (1.0 - alpha) * benchmark_.lane1_avg_ms + alpha * duration_ms; break;
        case 2: benchmark_.lane2_avg_ms = (1.0 - alpha) * benchmark_.lane2_avg_ms + alpha * duration_ms; break;
        case 3: benchmark_.lane3_avg_ms = (1.0 - alpha) * benchmark_.lane3_avg_ms + alpha * duration_ms; break;
        case 4: benchmark_.lane4_avg_ms = (1.0 - alpha) * benchmark_.lane4_avg_ms + alpha * duration_ms; break;
        case 5: benchmark_.lane5_avg_ms = (1.0 - alpha) * benchmark_.lane5_avg_ms + alpha * duration_ms; break;
    }
}

// =============================================================================
// Public interface / Metrics
// =============================================================================

void MultiRateEngine::set_px_to_mm(float px_to_mm)
{
    px_to_mm_.store(px_to_mm, std::memory_order_relaxed);
}

Metrics MultiRateEngine::get_metrics() const
{
    Metrics m{};

    m.frame_id           = frame_id_.load(std::memory_order_acquire);
    m.last_crack         = last_crack_score_.load(std::memory_order_relaxed);
    m.fused_crack        = m.last_crack;
    m.sig_conf           = latest_sig_conf_.load(std::memory_order_relaxed);
    m.crack_frames       = crack_frames_.load(std::memory_order_relaxed);
    m.yolo_count         = yolo_count_.load(std::memory_order_relaxed);
    m.uplink_count       = uplink_count_.load(std::memory_order_relaxed);
    m.latency_violations =
        latency_violations_.load(std::memory_order_relaxed);

    const double elapsed_s = (now_ms() - start_time_) / 1000.0;
    m.fps = (elapsed_s > 0.0)
        ? static_cast<float>(
              static_cast<double>(m.crack_frames) / elapsed_s)
        : 0.0f;

    if (latency_count_ > 0) {
        const size_t n = latency_count_;
        std::vector<double> tmp(latency_ring_.begin(),
                                latency_ring_.begin() + n);
        std::sort(tmp.begin(), tmp.end());
        m.latency_p50_ms = tmp[static_cast<size_t>(n * 0.50)];
        m.latency_p95_ms = tmp[static_cast<size_t>(n * 0.95)];
        m.latency_p99_ms = tmp[static_cast<size_t>(n * 0.99)];
    }

    m.window_crack_ratio = crack_stats_->window_crack_ratio();
    m.global_crack_ratio = crack_stats_->global_crack_ratio();

    m.yolo_hz            = yolo_hz_.load(std::memory_order_relaxed);
    m.yolo_age_ms        = static_cast<float>(semantic_age_ms());
    m.spike_bitrate_mbps = spike_bitrate_mbps_.load(std::memory_order_relaxed);

    m.crack_alert_thr    = 0.2f;
    m.yolo_conf_thr      = det_controller_->yolo_conf_threshold();
    m.avg_yolo_conf      = det_controller_->avg_yolo_conf();
    m.avg_crack_score    = det_controller_->avg_crack_score();
    m.avg_agreement      = det_controller_->avg_agreement();

    m.px_to_mm_scale     = px_to_mm_.load(std::memory_order_relaxed);

    // --- NEW: advanced statistics snapshot ---
    CrackStatistics stats = crack_statistics_tracker_.compute_statistics(
        now_ms(),
        m.last_crack,
        m.frame_id
    );
    m.vis_crack_width_mm  = stats.width_mm;
    m.vis_crack_length_mm = stats.length_mm;
    m.avg_crack_score     = stats.avg_crack_score;
    // --- End of NEW Section ---

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
    std::printf(
        "[Engine] frame=%llu fps=%.1f crack=%.3f sig_conf=%.3f\n"
        "         latency P50=%.2fms P95=%.2fms P99=%.2fms (thr=%.1fms)\n"
        "         violations=%llu yolo_count=%llu uplink=%llu\n"
        "         Benchmarks: L1=%.2fms L2=%.2fms L3=%.2fms L5=%.2fms\n"
        "         Averages: YoloConf=%.3f Crack=%.3f Agreement=%.3f\n",
        (unsigned long long)m.frame_id, m.fps, m.last_crack, m.sig_conf,
        m.latency_p50_ms, m.latency_p95_ms, m.latency_p99_ms, cfg_.max_control_latency_ms,
        (unsigned long long)m.latency_violations,
        (unsigned long long)m.yolo_count,
        (unsigned long long)m.uplink_count,
        benchmark_.lane1_avg_ms, benchmark_.lane2_avg_ms, benchmark_.lane3_avg_ms, benchmark_.lane5_avg_ms,
        m.avg_yolo_conf, m.avg_crack_score, m.avg_agreement
    );
}

void MultiRateEngine::_emergency_stop()
{
    std::fprintf(stderr,
        "[Engine] EMERGENCY STOP triggered at frame %llu\n",
        (unsigned long long)frame_id_.load());
    if (ctrl_cb_) {
        ControlDecision d{};
        d.throttle      = 0.0f;
        d.steer         = 0.0f;
        d.is_null_cycle = true;
        d.frame_id      = frame_id_.load();
        CallbackJob job;
        job.is_control = true;
        job.ctrl_dec   = d;
        callback_queue_->try_push(std::move(job));
    }
    running_.store(false, std::memory_order_relaxed);
}