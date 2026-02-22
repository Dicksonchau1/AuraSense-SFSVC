// =============================================================================
// engine.cpp — AuraSense SFSVC MultiRateEngine
// Approach B: rt_core is the authoritative sensing core.
// Lane 1 owns the downscale + rt_core call. All other lanes are consumers.
// =============================================================================

#include "engine.h"
#include "rt_core.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <numeric>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

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
// Failsafe configuration (index-based API: 0=lane1,1=yolo_age,2=signature_age)
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
// Minimal YOLO result placeholder (real YOLO can fill this)
// =============================================================================
struct YoloResult {
    float front_risk          = 0.0f;
    float left_risk           = 0.0f;
    float right_risk          = 0.0f;
    float crack_risk          = 0.0f;
    float min_distance_m      = 0.0f;
    float max_confidence      = 0.0f;
    int   num_detections      = 0;
    int   priority_detections = 0;
    int   num_filtered_out    = 0;
};

// =============================================================================
// Constructors / Destructor
// =============================================================================

MultiRateEngine::MultiRateEngine(ControlCallback ctrl_cb,
                                 UplinkCallback  uplink_cb)
    : ctrl_cb_        (std::move(ctrl_cb))
    , uplink_cb_      (std::move(uplink_cb))
    , crack_stats_    (5000.0)
    , det_controller_ (5000.0)
    , failsafe_(kFailsafeSignals,
                sizeof(kFailsafeSignals) / sizeof(kFailsafeSignals[0]))
    , yolo_manager_   ({416, 234, 320, 180, 640, 360, true, true, 1.3f})
    , signature_bank_ (new SignatureBank(500, 0.30f, 3600.0))
    , gating_engine_  (0.75f, 50, 1000.0f)
{}

MultiRateEngine::MultiRateEngine(ControlCallback ctrl_cb,
                                 UplinkCallback  uplink_cb,
                                 int             frame_height,
                                 int             frame_width)
    : MultiRateEngine(std::move(ctrl_cb), std::move(uplink_cb))
{
    frame_height_ = frame_height;
    frame_width_  = frame_width;
}

MultiRateEngine::~MultiRateEngine()
{
    stop();
    delete semantic_state_.exchange(nullptr);
    delete last_sig_match_.exchange(nullptr);
    delete signature_bank_;
}

// =============================================================================
// Lifecycle
// =============================================================================

void MultiRateEngine::start()
{
    if (running_.exchange(true)) return;   // idempotent

    start_time_      = now_ms();
    last_yolo_stamp_ = start_time_;
    last_vis_stamp_  = start_time_;

    t1_ = std::thread(&MultiRateEngine::lane1_control,   this);
    t2_ = std::thread(&MultiRateEngine::lane2_signature, this);
    t3_ = std::thread(&MultiRateEngine::lane3_yolo,      this);
    t4_ = std::thread(&MultiRateEngine::lane4_uplink,    this);
    t5_ = std::thread(&MultiRateEngine::lane5_visualize, this);
}

void MultiRateEngine::stop()
{
    if (!running_.exchange(false)) return;   // idempotent

    if (t1_.joinable()) t1_.join();
    if (t2_.joinable()) t2_.join();
    if (t3_.joinable()) t3_.join();
    if (t4_.joinable()) t4_.join();
    if (t5_.joinable()) t5_.join();
}

// =============================================================================
// Frame Ingest — Camera → Lane 1
// =============================================================================

void MultiRateEngine::push_frame(const uint8_t* bgr, int h, int w)
{
    if (!running_.load(std::memory_order_relaxed)) return;
    if (!bgr || h <= 0 || w <= 0)           return;

    Lane2Job job;
    job.frame_id    = frame_id_.load(std::memory_order_acquire);
    job.height      = h;
    job.width       = w;
    job.crack_score = 0.0f;
    job.frame_bgr.assign(bgr, bgr + h * w * 3);

    camera_queue_.try_push(std::move(job));
}

// =============================================================================
// LANE 1 — RT Bridge (fan-out)
// =============================================================================

void MultiRateEngine::lane1_control()
{
    // Pin this thread to RT core (core 2) using rt_core API
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
        if (!camera_queue_.try_pop(job)) {
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

        frame_id_.store(out.frame_id,
                        std::memory_order_release);

        last_crack_score_.store(out.crack_score,
                                std::memory_order_relaxed);

        crack_stats_.add_sample(t_start, out.crack_score);
        ++crack_frames_;

        // Control decision
        const float  sig_conf = latest_sig_conf_.load(
                                    std::memory_order_relaxed);
        const double sem_age  = semantic_age_ms();
        ControlDecision decision =
            make_decision(out, sig_conf, sem_age);

        ctrl_queue_.try_push(decision);
        if (ctrl_cb_) ctrl_cb_(decision);

        // Fan-out to Lane 2 (signature)
        job.frame_id    = out.frame_id;
        job.crack_score = out.crack_score;
        sig_queue_.try_push(job); // copy; Lane 3 also needs data

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
            yolo_queue_.try_push(std::move(yj));
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
            vis_queue_.try_push(std::move(vj));
        }

        // Latency + failsafe (ignore warm-up frames for stats)
        const double latency = now_ms() - t_start;

        if (out.frame_id > 300) {
            latency_ring_[latency_head_] = latency;
            latency_head_ = (latency_head_ + 1) % latency_ring_.size();
            if (latency_count_ < latency_ring_.size())
                ++latency_count_;
            if (latency > max_control_latency_ms_)
                ++latency_violations_;
        }

        // Failsafe sees every cycle
        failsafe_.update(0, static_cast<float>(latency));

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
            uplink_queue_.try_push(std::move(up));
        }
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
        if (!sig_queue_.try_pop(job)) {
            std::this_thread::yield();
            continue;
        }
        if (job.frame_bgr.empty()) continue;

        MatchResult raw_match =
            signature_bank_->find_match_full(
                {}, {}, {}, {}, job.crack_score);

        SignatureMatch match =
            signature_bank_->to_signature_match(raw_match);

        latest_sig_conf_.store(match.confidence,
                               std::memory_order_relaxed);

        SignatureMatch* new_match =
            new SignatureMatch(match);
        SignatureMatch* old_match =
            last_sig_match_.exchange(new_match,
                                     std::memory_order_acq_rel);
        delete old_match;
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
        if (!yolo_queue_.try_pop(job)) {
            std::this_thread::yield();
            continue;
        }

        const double t0 = now_ms();

        SignatureMatch sig_match{};
        if (auto* sm = last_sig_match_.load(std::memory_order_acquire))
            sig_match = *sm;

        GatingDecision gd =
            gating_engine_.decide(
                sig_match,
                job.frame_id,
                t0,
                job.crack_score,
                false);

        if (!gd.should_infer)
            continue;

        // Placeholder YOLO result; replace with real inference when ready
        YoloResult yolo_result{};
        yolo_result.max_confidence = 0.0f;
        yolo_result.crack_risk     = job.crack_score;

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

        // YOLO metrics: count + Hz
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

        // Semantic state: last YOLO time and frame (for age)
        SemanticState* s = new SemanticState{};
        s->frame_id     = job.frame_id;
        s->timestamp_ms = t0;
        s->latency_ms   = 0.0;

        SemanticState* old =
            semantic_state_.exchange(s, std::memory_order_acq_rel);
        delete old;
    }
}

// =============================================================================
// Lane 4 — Uplink (real count + callback)
// =============================================================================

void MultiRateEngine::lane4_uplink()
{
    while (running_.load(std::memory_order_relaxed))
    {
        UplinkPayload up;
        if (!uplink_queue_.try_pop(up)) {
            std::this_thread::yield();
            continue;
        }

        ++uplink_count_;

        if (uplink_cb_) {
            uplink_cb_(up);
        }
    }
}

// =============================================================================
// Lane 5 — Visualization: side-by-side original | crack-tinted + bitrate
// =============================================================================

void MultiRateEngine::lane5_visualize()
{
    cv::Mat vis, recon;

    while (running_.load(std::memory_order_relaxed))
    {
        VisJob job;
        if (!vis_queue_.try_pop(job)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }
        if (job.frame_bgr.empty() || job.frame_h <= 0 || job.frame_w <= 0)
            continue;

        // Wrap raw BGR buffer as Mat
        cv::Mat frame(job.frame_h, job.frame_w, CV_8UC3,
                      job.frame_bgr.data());

        // Reconstructed view as crack-tinted copy
        recon = frame.clone();
        const float cs = std::max(0.0f, std::min(1.0f, job.crack_score));
        if (cs > 0.0f) {
            cv::Mat tint(frame.size(), frame.type(), cv::Scalar(0, 0, 255)); // red tint
            cv::addWeighted(tint, cs, recon, 1.0 - cs, 0.0, recon);
        }

        // Side-by-side composite: [original | recon]
        vis = cv::Mat(job.frame_h,
                      job.frame_w * 2,
                      CV_8UC3);
        frame.copyTo(vis(cv::Rect(0, 0, job.frame_w, job.frame_h)));
        recon.copyTo(vis(cv::Rect(job.frame_w, 0, job.frame_w, job.frame_h)));

        // Overlay crack metrics
        const cv::Scalar box_color(0, 0, 0);
        const cv::Scalar text_color(200, 255, 200);
        const cv::Scalar warn_color(0, 255, 255);
        const cv::Scalar crit_color(0, 0, 255);

        const int box_w = 360;
        const int box_h = 80;
        cv::rectangle(vis,
                      cv::Rect(10, 10, box_w, box_h),
                      box_color,
                      cv::FILLED);

        char buf[160];
        std::snprintf(buf, sizeof(buf),
                      "Crack: %.3f  Spikes: %d / %d",
                      job.crack_score,
                      job.on_count,
                      job.off_count);

        cv::putText(vis,
                    buf,
                    cv::Point(20, 40),
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.6,
                    text_color,
                    1,
                    cv::LINE_AA);

        const char* sev_label = "OK";
        cv::Scalar sev_color  = text_color;
        if (cs > 0.33f) {
            sev_label = "CRITICAL";
            sev_color = crit_color;
        } else if (cs > 0.11f) {
            sev_label = "WARNING";
            sev_color = warn_color;
        }

        std::snprintf(buf, sizeof(buf),
                      "Severity: %s  Sparsity: %.3f",
                      sev_label,
                      job.sparsity);

        cv::putText(vis,
                    buf,
                    cv::Point(20, 65),
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.6,
                    sev_color,
                    1,
                    cv::LINE_AA);

        // Encode to JPEG and publish + compute spike bitrate
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
                spike_bitrate_mbps_.store(
                    static_cast<float>(mbps),
                    std::memory_order_relaxed);
            }
        }
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

// =============================================================================
// Decision logic
// =============================================================================

ControlDecision MultiRateEngine::make_decision(
    const ControlOutput& rt_out,
    float                sig_conf,
    double               sem_age_ms) const
{
    ControlDecision d{};

    d.frame_id         = rt_out.frame_id;
    d.timestamp        = now_ms();
    d.throttle         = rt_out.throttle;
    d.steer            = rt_out.steer;
    d.crack_score      = rt_out.fused_crack_score;
    d.sparsity         = rt_out.sparsity;
    d.confidence       = sig_conf;
    d.semantic_age_ms  = sem_age_ms;

    d.is_null_cycle        = rt_out.is_null_cycle;
    d.inference_suppressed = rt_out.inference_suppressed;
    d.event_only_mode      = rt_out.event_only_mode;

    d.reference_frame_age = rt_out.reference_frame_age;
    d.yolo_active         = rt_out.yolo_active;
    d.yolo_age_ms         = rt_out.yolo_age_ms;
    d.encode_time_ms      = rt_out.encode_time_ms;

    return d;
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

    m.window_crack_ratio = crack_stats_.window_crack_ratio();
    m.global_crack_ratio = crack_stats_.global_crack_ratio();

    m.yolo_hz            = yolo_hz_.load(std::memory_order_relaxed);
    m.yolo_age_ms        = semantic_age_ms();
    m.spike_bitrate_mbps = spike_bitrate_mbps_.load(std::memory_order_relaxed);

    m.crack_alert_thr    = 0.0f;
    m.yolo_conf_thr      = 0.0f;
    m.avg_yolo_conf      = 0.0f;
    m.avg_crack_score    = 0.0f;
    m.avg_agreement      = 0.0f;

    m.px_to_mm_scale     = px_to_mm_.load(std::memory_order_relaxed);
    m.vis_crack_width_mm = 0.0f;
    m.vis_crack_length_mm= 0.0f;

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
        "[Engine] frame=%d fps=%.1f crack=%.3f sig_conf=%.3f\n"
        "         latency P50=%.2fms P95=%.2fms P99=%.2fms\n"
        "         violations=%llu yolo_count=%llu uplink=%llu\n",
        m.frame_id, m.fps, m.last_crack, m.sig_conf,
        m.latency_p50_ms, m.latency_p95_ms, m.latency_p99_ms,
        (unsigned long long)m.latency_violations,
        (unsigned long long)m.yolo_count,
        (unsigned long long)m.uplink_count);
}

void MultiRateEngine::_emergency_stop()
{
    std::fprintf(stderr,
        "[Engine] EMERGENCY STOP triggered at frame %d\n",
        frame_id_.load());
    if (ctrl_cb_) {
        ControlDecision d{};
        d.throttle      = 0.0f;
        d.steer         = 0.0f;
        d.is_null_cycle = true;
        d.frame_id      = frame_id_.load();
        ctrl_cb_(d);
    }
    running_.store(false, std::memory_order_relaxed);
}
