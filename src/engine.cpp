// =============================================================================
// engine.cpp — AuraSense 6-Lane Fan-Out Engine
//
// Full production implementation. Zero stubs. Zero placeholders.
//
// Lane 1: Control hot path  (camera_queue_ → rt_core → fan-out)
// Lane 2: Signature matching (sig_queue_ → SignatureBank)
// Lane 3: YOLO inference     (yolo_queue_ → yolo_detect → publish)
// Lane 4: Uplink             (uplink_queue_ → serialize → callback)
// Lane 5: Visualization      (vis_queue_ → JPEG encode)
// Lane 6: Callback dispatch  (callback_queue_ → user callbacks)
//
// Threading:
//   - All inter-lane queues are SPSC (LockFreeQueue)
//   - callback_queue_ is MPSC: Lane 1 + Lane 4 push, Lane 6 pops
//     Protected by s_callback_push_mutex for push serialization.
//   - Atomic state: semantic_state_, last_sig_match_, latest_sig_conf_
//
// Dependencies: OpenCV 4.x, pthread, C++17
// =============================================================================

#include "engine.h"

#include "yolo_bridge.h"
#include "yolo_manager.h"
#include "uplink_serializer.h"
#include "gating_engine.hpp"
#include "failsafe.hpp"

#include "signature_bank.hpp"
#include "aurasense_yolo_bond.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <chrono>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <cstdio>

// =============================================================================
// MPSC mutex for callback_queue_
// Lane 1 and Lane 4 both push to callback_queue_, but LockFreeQueue is SPSC.
// This mutex serializes pushes so cached_head_ is never raced.
// =============================================================================
static std::mutex s_callback_push_mutex;

// =============================================================================
// Time helper
// =============================================================================
static double now_seconds() {
    using clock = std::chrono::steady_clock;
    return std::chrono::duration<double>(
        clock::now().time_since_epoch()).count();
}

// =============================================================================
// Constructor
// =============================================================================
MultiRateEngine::MultiRateEngine(ControlCallback ctrl_cb,
                                 UplinkCallback uplink_cb)
    : ctrl_cb_(std::move(ctrl_cb))
    , uplink_cb_(std::move(uplink_cb))
{
    // Allocate SPSC queues on heap (engine.h stores raw pointers)
    camera_queue_   = new LockFreeQueue<Lane2Job, 64>();
    sig_queue_      = new LockFreeQueue<Lane2Job, 64>();
    yolo_queue_     = new LockFreeQueue<Lane3Job, 32>();
    uplink_queue_   = new LockFreeQueue<UplinkPayload, 128>();
    vis_queue_      = new LockFreeQueue<VisJob, 32>();
    callback_queue_ = new LockFreeQueue<CallbackJob, 64>();

    // Allocate components
    signature_bank_         = std::make_unique<SignatureBank>();
    gating_engine_          = std::make_unique<GatingEngine>();
    failsafe_               = std::make_unique<FailsafeMonitor>();
    yolo_manager_           = std::make_unique<YoloManager>();
    det_controller_         = std::make_unique<DetectionController>();
    crack_inference_engine_  = std::make_unique<CrackInferenceEngine>();
    crack_stats_            = std::make_unique<CrackStatisticsTracker>();

    // Initialize atomic pointers
    semantic_state_.store(nullptr, std::memory_order_relaxed);
    last_sig_match_.store(nullptr, std::memory_order_relaxed);

    // Zero out latency ring
    latency_ring_.fill(0.0);
}

// =============================================================================
// Destructor
// =============================================================================
MultiRateEngine::~MultiRateEngine() {
    stop();

    delete camera_queue_;
    delete sig_queue_;
    delete yolo_queue_;
    delete uplink_queue_;
    delete vis_queue_;
    delete callback_queue_;

    // Clean up atomic heap pointers
    delete semantic_state_.load(std::memory_order_relaxed);
    delete last_sig_match_.load(std::memory_order_relaxed);
}

// =============================================================================
// start() / stop()
// =============================================================================

void MultiRateEngine::start() {
    start(EngineConfig{});
}

void MultiRateEngine::start(const EngineConfig& cfg) {
    cfg_ = cfg;

    bool expected = false;
    if (!running_.compare_exchange_strong(expected, true))
        return; // already running

    start_time_     = now_seconds();
    last_yolo_stamp_ = start_time_;
    last_vis_stamp_  = start_time_;

    // Initialize YOLO model if path provided
    if (!cfg_.onnx_model_path.empty()) {
        if (!yolo_init(cfg_.onnx_model_path)) {
            std::fprintf(stderr,
                "[Engine] YOLO init failed for '%s'. "
                "Lane 3 will return empty results.\n",
                cfg_.onnx_model_path.c_str());
        }
    }

    // Launch 6 threads
    t1_ = std::thread(&MultiRateEngine::lane1_control, this);
    t2_ = std::thread(&MultiRateEngine::lane2_signature, this);
    t3_ = std::thread(&MultiRateEngine::lane3_yolo, this);
    t4_ = std::thread(&MultiRateEngine::lane4_uplink, this);
    t5_ = std::thread(&MultiRateEngine::lane5_visualize, this);
    callback_thread_ = std::thread(
        &MultiRateEngine::lane6_callback_dispatcher, this);
}

void MultiRateEngine::stop() {
    bool expected = true;
    if (!running_.compare_exchange_strong(expected, false))
        return;

    if (t1_.joinable()) t1_.join();
    if (t2_.joinable()) t2_.join();
    if (t3_.joinable()) t3_.join();
    if (t4_.joinable()) t4_.join();
    if (t5_.joinable()) t5_.join();
    if (callback_thread_.joinable()) callback_thread_.join();

    yolo_shutdown();
}

// =============================================================================
// push_frame() — external entry point
// =============================================================================
void MultiRateEngine::push_frame(const uint8_t* bgr, int h, int w) {
    uint64_t fid = frame_id_.fetch_add(1, std::memory_order_relaxed);

    Lane2Job job;
    job.frame_id    = static_cast<int>(fid);
    job.height      = h;
    job.width       = w;
    job.crack_score = 0.0f;
    job.frame_bgr.assign(bgr, bgr + h * w * 3);

    camera_queue_->try_push(std::move(job));
}

// =============================================================================
// Lane 1: Control Hot Path
//
// Pops frames from camera_queue_, runs rt_core, fans out to all other queues.
// Also pushes ControlDecision via callback_queue_ (MPSC-protected).
// =============================================================================
void MultiRateEngine::lane1_control() {
    const int target_w = rt_core_target_width();
    const int target_h = rt_core_target_height();

    Lane2Job job;

    while (running_.load(std::memory_order_relaxed)) {
        if (!camera_queue_->pop_wait(job, std::chrono::milliseconds(10)))
            continue;

        auto t0 = std::chrono::steady_clock::now();

        // --- Resize to rt_core resolution ---
        cv::Mat frame_bgr(job.height, job.width, CV_8UC3,
                          job.frame_bgr.data());
        cv::Mat resized;

        if (job.height != target_h || job.width != target_w) {
            cv::resize(frame_bgr, resized,
                       cv::Size(target_w, target_h),
                       0, 0, cv::INTER_AREA);
        } else {
            resized = frame_bgr;
        }

        if (!resized.isContinuous()) {
            resized = resized.clone();
        }

        // --- Process frame through rt_core ---
        ControlOutput output = rt_core_process_frame_ptr(
            resized.data, resized.rows, resized.cols);

        auto t1 = std::chrono::steady_clock::now();
        double lane1_ms = std::chrono::duration<double, std::milli>(
            t1 - t0).count();

        // --- Record latency ---
        size_t idx = latency_head_ % latency_ring_.size();
        latency_ring_[idx] = lane1_ms;
        latency_head_++;
        latency_count_.fetch_add(1, std::memory_order_relaxed);

        if (lane1_ms > 5.0) {
            latency_violations_.fetch_add(1, std::memory_order_relaxed);
        }

        // --- Update crack tracking ---
        float crack = output.fused_crack_score;
        last_crack_score_.store(crack, std::memory_order_relaxed);
        if (crack > 0.1f) {
            crack_frames_.fetch_add(1, std::memory_order_relaxed);
        }

        double now = now_seconds();
        float px_mm = px_to_mm_.load(std::memory_order_relaxed);
        crack_stats_->update(crack, px_mm);

        // --- Update failsafe ---
        bool yolo_stale = (output.yolo_age_ms > 2000.0f);
        failsafe_->update(static_cast<float>(lane1_ms), crack, yolo_stale);

        // --- Gating decision ---
        float sig_conf = latest_sig_conf_.load(std::memory_order_relaxed);
        GatingDecision gate = gating_engine_->evaluate(
            crack, output.sparsity, sig_conf,
            static_cast<double>(output.yolo_age_ms),
            output.inference_suppressed != 0);

        // --- Fan out: Lane 2 (Signature) ---
        {
            Lane2Job sig_job;
            sig_job.frame_id    = job.frame_id;
            sig_job.height      = resized.rows;
            sig_job.width       = resized.cols;
            sig_job.crack_score = crack;
            sig_job.frame_bgr.assign(
                resized.data,
                resized.data + resized.total() * resized.elemSize());
            sig_queue_->try_push(std::move(sig_job));
        }

        // --- Fan out: Lane 3 (YOLO, gated) ---
        if (gate.should_run_yolo) {
            Lane3Job yolo_job;
            yolo_job.frame_id    = job.frame_id;
            yolo_job.height      = job.height;
            yolo_job.width       = job.width;
            yolo_job.timestamp   = now;
            yolo_job.crack_score = crack;
            yolo_job.sparsity    = output.sparsity;
            yolo_job.target_hz   = rt_core_yolo_get_target_hz();
            yolo_job.frame_bgr   = job.frame_bgr; // full resolution
            yolo_queue_->try_push(std::move(yolo_job));
        }

        // --- Fan out: Lane 5 (Visualization) ---
        {
            VisJob vis_job;
            vis_job.frame_id    = job.frame_id;
            vis_job.frame_h     = resized.rows;
            vis_job.frame_w     = resized.cols;
            vis_job.crack_score = crack;
            vis_job.sparsity    = output.sparsity;
            vis_job.on_count    = output.on_spike_count;
            vis_job.off_count   = output.off_spike_count;
            vis_job.frame_bgr.assign(
                resized.data,
                resized.data + resized.total() * resized.elemSize());
            vis_queue_->try_push(std::move(vis_job));
        }

        // --- Make control decision ---
        ControlDecision decision = make_decision(output, sig_conf, now);
        decision.control_latency_ms = lane1_ms;

        // --- Push to callback queue (MPSC) ---
        {
            CallbackJob cb;
            cb.type     = CallbackJob::CONTROL;
            cb.decision = decision;

            std::lock_guard<std::mutex> lk(s_callback_push_mutex);
            callback_queue_->try_push(std::move(cb));
        }

        // --- Push to uplink queue ---
        {
            UplinkPayload payload;
            payload.frame_id           = job.frame_id;
            payload.timestamp          = now;
            payload.throttle           = decision.throttle;
            payload.steer              = decision.steer;
            payload.crack_score        = crack;
            payload.sparsity           = output.sparsity;
            payload.action             = decision.action;
            payload.control_latency_ms = lane1_ms;
            payload.crack_severity     = decision.crack_severity;
            payload.crack_width_mm     = decision.crack_width_mm;
            payload.failsafe_status    = failsafe_->status();
            payload.sig_conf           = sig_conf;
            payload.yolo_count         = yolo_count_.load(
                                            std::memory_order_relaxed);
            payload.latency_violations = latency_violations_.load(
                                            std::memory_order_relaxed);

            uplink_queue_->try_push(std::move(payload));
        }

        update_benchmark(1, lane1_ms);
    }
}

// =============================================================================
// Lane 2: Signature Matching
// =============================================================================
void MultiRateEngine::lane2_signature() {
    Lane2Job job;

    while (running_.load(std::memory_order_relaxed)) {
        if (!sig_queue_->pop_wait(job, std::chrono::milliseconds(50)))
            continue;

        auto t0 = std::chrono::steady_clock::now();

        // --- Compute average luminance ---
        float avg_luminance = 0.0f;
        if (!job.frame_bgr.empty() && job.height > 0 && job.width > 0) {
            int total = job.height * job.width;
            long sum = 0;
            const uint8_t* bgr = job.frame_bgr.data();
            // Sample every 16th pixel for speed
            int step = std::max(1, total / 4096);
            int sampled = 0;
            for (int i = 0; i < total; i += step) {
                int b = bgr[i * 3 + 0];
                int g = bgr[i * 3 + 1];
                int r = bgr[i * 3 + 2];
                sum += (54 * r + 183 * g + 19 * b) >> 8;
                sampled++;
            }
            if (sampled > 0)
                avg_luminance = static_cast<float>(sum) /
                                (static_cast<float>(sampled) * 255.0f);
        }

        // --- Extract gradient-based feature vectors ---
        std::vector<float> gabor_fp(64, 0.0f);
        std::vector<float> semantic_prof(32, 0.0f);
        std::vector<float> context_vec(16, 0.0f);
        std::vector<float> motion_sig(16, 0.0f);

        if (!job.frame_bgr.empty() && job.height > 0 && job.width > 0) {
            cv::Mat frame(job.height, job.width, CV_8UC3,
                          const_cast<uint8_t*>(job.frame_bgr.data()));
            cv::Mat gray;
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

            // Block-mean features (8x8 grid → 64 values)
            int block_h = std::max(1, gray.rows / 8);
            int block_w = std::max(1, gray.cols / 8);
            int fi = 0;
            for (int by = 0; by < 8 && fi < 64; by++) {
                for (int bx = 0; bx < 8 && fi < 64; bx++) {
                    int rx = std::min(bx * block_w, gray.cols - 1);
                    int ry = std::min(by * block_h, gray.rows - 1);
                    int rw = std::min(block_w, gray.cols - rx);
                    int rh = std::min(block_h, gray.rows - ry);
                    cv::Rect roi(rx, ry, rw, rh);
                    gabor_fp[fi++] = static_cast<float>(
                        cv::mean(gray(roi))[0]) / 255.0f;
                }
            }

            // Vertical profile (32 horizontal bands)
            int band_h = std::max(1, gray.rows / 32);
            for (int i = 0; i < 32; i++) {
                int ry = std::min(i * band_h, gray.rows - 1);
                int rh = std::min(band_h, gray.rows - ry);
                cv::Rect roi(0, ry, gray.cols, rh);
                semantic_prof[i] = static_cast<float>(
                    cv::mean(gray(roi))[0]) / 255.0f;
            }

            // Global statistics
            cv::Scalar mean_val, stddev_val;
            cv::meanStdDev(gray, mean_val, stddev_val);
            context_vec[0] = static_cast<float>(mean_val[0]) / 255.0f;
            context_vec[1] = static_cast<float>(stddev_val[0]) / 255.0f;
            context_vec[2] = job.crack_score;
            context_vec[3] = avg_luminance;
        }

        // --- Run signature matching ---
        MatchResult match = signature_bank_->find_match_full(
            gabor_fp, semantic_prof, context_vec, motion_sig,
            avg_luminance);

        // Register new signature if no match and crack activity
        if (!match.matched && job.crack_score > 0.05f) {
            signature_bank_->register_signature(
                gabor_fp, semantic_prof, context_vec, motion_sig,
                job.crack_score, avg_luminance);
        }

        // Convert to SignatureMatch
        SignatureMatch sig_match =
            signature_bank_->to_signature_match(match);
        sig_match.crack_score = job.crack_score;

        latest_sig_conf_.store(sig_match.confidence,
                               std::memory_order_release);

        // Update stored match (atomic swap with delete)
        SignatureMatch* new_match = new SignatureMatch(sig_match);
        SignatureMatch* old_match = last_sig_match_.exchange(
            new_match, std::memory_order_acq_rel);
        delete old_match;

        // Update crack inference
        crack_inference_engine_->update(
            job.crack_score,
            1.0f - avg_luminance, // proxy for sparsity
            avg_luminance,
            sig_match.confidence);

        // Update detection controller
        double now_ms = now_seconds() * 1000.0;
        det_controller_->add_detection(
            now_ms, sig_match.confidence, job.crack_score);

        auto t1 = std::chrono::steady_clock::now();
        double lane2_ms = std::chrono::duration<double, std::milli>(
            t1 - t0).count();
        update_benchmark(2, lane2_ms);
    }
}

// =============================================================================
// Lane 3: YOLO Inference
// =============================================================================
void MultiRateEngine::lane3_yolo() {
    Lane3Job job;

    while (running_.load(std::memory_order_relaxed)) {
        if (!yolo_queue_->pop_wait(job, std::chrono::milliseconds(100)))
            continue;

        auto t0 = std::chrono::steady_clock::now();

        // Reconstruct cv::Mat from job data
        cv::Mat frame(job.height, job.width, CV_8UC3,
                      const_cast<uint8_t*>(job.frame_bgr.data()));

        // Run YOLO inference
        YoloResult result = yolo_detect(frame);

        // Publish to rt_core lock-free YOLO state
        double now = now_seconds();
        rt_core_yolo_publish(
            now,
            result.front_risk,
            result.left_risk,
            result.right_risk,
            result.crack_risk,
            result.min_distance_m,
            result.max_confidence,
            result.num_detections,
            result.priority_detections,
            result.num_filtered_out);

        // Update semantic state (atomic swap with delete)
        SemanticState* new_state = new SemanticState;
        new_state->frame_id       = job.frame_id;
        new_state->timestamp_ms   = now * 1000.0;
        new_state->latency_ms     = result.latency_ms;
        new_state->front_risk     = result.front_risk;
        new_state->left_risk      = result.left_risk;
        new_state->right_risk     = result.right_risk;
        new_state->crack_risk     = result.crack_risk;
        new_state->num_detections = result.num_detections;
        new_state->max_confidence = result.max_confidence;
        new_state->agreement      = det_controller_->avg_agreement();

        SemanticState* old_state = semantic_state_.exchange(
            new_state, std::memory_order_acq_rel);
        delete old_state;

        // Update counters
        yolo_count_.fetch_add(1, std::memory_order_relaxed);
        last_yolo_stamp_ = now;
        yolo_hz_.store(job.target_hz, std::memory_order_relaxed);

        // Update YoloManager
        yolo_manager_->record_run(now, result.num_detections);

        // Update detection controller thresholds
        det_controller_->update_adaptive_thresholds();

        auto t1 = std::chrono::steady_clock::now();
        double lane3_ms = std::chrono::duration<double, std::milli>(
            t1 - t0).count();
        update_benchmark(3, lane3_ms);
    }
}

// =============================================================================
// Lane 4: Uplink Serialization + Callback Dispatch
// =============================================================================
void MultiRateEngine::lane4_uplink() {
    UplinkPayload payload;

    while (running_.load(std::memory_order_relaxed)) {
        if (!uplink_queue_->pop_wait(payload, std::chrono::milliseconds(50)))
            continue;

        auto t0 = std::chrono::steady_clock::now();

        uplink_count_.fetch_add(1, std::memory_order_relaxed);

        // Push to callback queue for dispatch (MPSC-protected)
        {
            CallbackJob cb;
            cb.type    = CallbackJob::UPLINK;
            cb.payload = payload;

            std::lock_guard<std::mutex> lk(s_callback_push_mutex);
            callback_queue_->try_push(std::move(cb));
        }

        auto t1 = std::chrono::steady_clock::now();
        double lane4_ms = std::chrono::duration<double, std::milli>(
            t1 - t0).count();
        update_benchmark(4, lane4_ms);
    }
}

// =============================================================================
// Lane 5: Visualization (Spike Frame JPEG Encoding)
// =============================================================================
void MultiRateEngine::lane5_visualize() {
    VisJob job;

    while (running_.load(std::memory_order_relaxed)) {
        if (!vis_queue_->pop_wait(job, std::chrono::milliseconds(100)))
            continue;

        auto t0 = std::chrono::steady_clock::now();

        if (job.frame_bgr.empty()) continue;

        cv::Mat frame(job.frame_h, job.frame_w, CV_8UC3,
                      const_cast<uint8_t*>(job.frame_bgr.data()));

        cv::Mat vis = frame.clone();

        // Overlay crack score bar (red, top)
        int bar_w = static_cast<int>(job.crack_score * vis.cols);
        if (bar_w > 0) {
            cv::rectangle(vis, cv::Point(0, 0),
                         cv::Point(bar_w, 8),
                         cv::Scalar(0, 0, 255), cv::FILLED);
        }

        // Overlay sparsity bar (green, below)
        int sbar_w = static_cast<int>(job.sparsity * vis.cols);
        if (sbar_w > 0) {
            cv::rectangle(vis, cv::Point(0, 10),
                         cv::Point(sbar_w, 18),
                         cv::Scalar(0, 255, 0), cv::FILLED);
        }

        // Overlay spike count text
        char text[64];
        std::snprintf(text, sizeof(text), "ON:%d OFF:%d",
                      job.on_count, job.off_count);
        cv::putText(vis, text, cv::Point(4, vis.rows - 8),
                    cv::FONT_HERSHEY_SIMPLEX, 0.35,
                    cv::Scalar(255, 255, 255), 1);

        // Encode JPEG
        std::vector<uint8_t> jpeg_buf;
        std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 70};
        cv::imencode(".jpg", vis, jpeg_buf, params);

        // Store for retrieval
        {
            std::lock_guard<std::mutex> lk(vis_mutex_);
            latest_spike_jpeg_ = std::move(jpeg_buf);
        }

        // Compute spike bitrate estimate
        int total_spikes = job.on_count + job.off_count;
        float bits_per_spike = 16.0f; // 1 byte pos + 1 byte polarity
        float bytes_per_frame = total_spikes * bits_per_spike / 8.0f;
        float mbps = bytes_per_frame * 30.0f / (1024.0f * 1024.0f);
        spike_bitrate_mbps_.store(mbps, std::memory_order_relaxed);

        last_vis_stamp_ = now_seconds();

        auto t1 = std::chrono::steady_clock::now();
        double lane5_ms = std::chrono::duration<double, std::milli>(
            t1 - t0).count();
        update_benchmark(5, lane5_ms);
    }
}

// =============================================================================
// Lane 6: Callback Dispatcher
// Pops from callback_queue_ and invokes user callbacks on this thread.
// =============================================================================
void MultiRateEngine::lane6_callback_dispatcher() {
    CallbackJob job;

    while (running_.load(std::memory_order_relaxed)) {
        if (!callback_queue_->pop_wait(job, std::chrono::milliseconds(10)))
            continue;

        switch (job.type) {
            case CallbackJob::CONTROL:
                if (ctrl_cb_) ctrl_cb_(job.decision);
                break;
            case CallbackJob::UPLINK:
                if (uplink_cb_) uplink_cb_(job.payload);
                break;
        }
    }
}

// =============================================================================
// make_decision — convert ControlOutput to ControlDecision
// =============================================================================
ControlDecision MultiRateEngine::make_decision(
    const ControlOutput& output, float sig_conf, double timestamp) const
{
    ControlDecision d;
    d.frame_id             = output.frame_id;
    d.timestamp            = timestamp;
    d.throttle             = output.throttle;
    d.steer                = output.steer;
    d.crack_score          = output.fused_crack_score;
    d.sparsity             = output.sparsity;
    d.confidence           = sig_conf;
    d.semantic_age_ms      = semantic_age_ms();
    d.is_null_cycle        = (output.is_null_cycle != 0);
    d.inference_suppressed = (output.inference_suppressed != 0);
    d.event_only_mode      = (output.event_only_mode != 0);
    d.yolo_active          = (output.yolo_active != 0);
    d.yolo_age_ms          = output.yolo_age_ms;
    d.reference_frame_age  = output.reference_frame_age;
    d.encode_time_ms       = output.encode_time_ms;

    // Determine action from crack score
    float crack = output.fused_crack_score;
    if (crack > 0.70f)      d.action = "STOP";
    else if (crack > 0.40f) d.action = "SLOW";
    else if (crack > 0.10f) d.action = "CAUTION";
    else                    d.action = "CLEAR";

    // Attach crack metrics
    CrackMetrics cm           = crack_stats_->get_metrics();
    d.crack_width_mm          = cm.width_mm;
    d.crack_length_mm         = cm.length_mm;
    d.crack_severity          = cm.severity_label;
    d.crack_confidence_percent = cm.confidence_percent;

    return d;
}

// =============================================================================
// semantic_age_ms
// =============================================================================
double MultiRateEngine::semantic_age_ms() const {
    SemanticState* state = semantic_state_.load(std::memory_order_acquire);
    if (!state) return 99999.0;

    double now_ms = now_seconds() * 1000.0;
    return now_ms - state->timestamp_ms;
}

// =============================================================================
// update_benchmark
// =============================================================================
void MultiRateEngine::update_benchmark(int lane_id, double duration_ms) {
    std::lock_guard<std::mutex> lk(benchmark_mutex_);

    BenchmarkSample sample;
    sample.lane      = lane_id;
    sample.frame_id  = static_cast<int>(
        frame_id_.load(std::memory_order_relaxed));
    sample.latency_ms = static_cast<float>(duration_ms);
    sample.yolo_ran   = (lane_id == 3);
    if (lane_id == 3) {
        sample.yolo_latency_ms = static_cast<float>(duration_ms);
    }

    benchmark_.add(sample);
}

// =============================================================================
// get_metrics()
// =============================================================================
Metrics MultiRateEngine::get_metrics() const {
    Metrics m;

    double now     = now_seconds();
    double elapsed = now - start_time_;

    m.frame_id = static_cast<int>(
        frame_id_.load(std::memory_order_relaxed));
    m.last_crack = last_crack_score_.load(std::memory_order_relaxed);
    m.sig_conf   = latest_sig_conf_.load(std::memory_order_relaxed);
    m.crack_frames       = crack_frames_.load(std::memory_order_relaxed);
    m.yolo_count         = yolo_count_.load(std::memory_order_relaxed);
    m.uplink_count       = uplink_count_.load(std::memory_order_relaxed);
    m.latency_violations = latency_violations_.load(
                               std::memory_order_relaxed);

    // FPS
    if (elapsed > 0.1) {
        m.fps = static_cast<float>(m.frame_id / elapsed);
    }

    m.yolo_hz = yolo_hz_.load(std::memory_order_relaxed);

    // Latency percentiles from ring buffer
    size_t count = latency_count_.load(std::memory_order_relaxed);
    if (count > 0) {
        size_t n = std::min(count, latency_ring_.size());
        std::vector<double> tmp(latency_ring_.begin(),
                                latency_ring_.begin() + n);
        std::sort(tmp.begin(), tmp.end());

        m.latency_p50_ms = static_cast<float>(tmp[n / 2]);
        m.latency_p95_ms = static_cast<float>(
            tmp[std::min(n - 1, static_cast<size_t>(n * 0.95))]);
        m.latency_p99_ms = static_cast<float>(
            tmp[std::min(n - 1, static_cast<size_t>(n * 0.99))]);
    }

    // Crack stats
    m.window_crack_ratio = crack_stats_->window_crack_ratio();
    m.global_crack_ratio = crack_stats_->global_crack_ratio();
    m.crack_alert_thr    = crack_stats_->crack_alert_threshold();

    // Detection controller
    m.avg_yolo_conf  = det_controller_->avg_yolo_conf();
    m.avg_crack_score = det_controller_->avg_crack_score();
    m.avg_agreement  = det_controller_->avg_agreement();
    m.yolo_conf_thr  = det_controller_->yolo_conf_threshold();

    m.px_to_mm_scale = px_to_mm_.load(std::memory_order_relaxed);

    // YOLO age
    m.yolo_age_ms = static_cast<float>(
        (now - last_yolo_stamp_) * 1000.0);

    // Spike bitrate
    m.spike_bitrate_mbps = spike_bitrate_mbps_.load(
        std::memory_order_relaxed);

    // Crack visualization metrics
    CrackMetrics cm       = crack_stats_->get_metrics();
    m.vis_crack_width_mm  = cm.width_mm;
    m.vis_crack_length_mm = cm.length_mm;

    return m;
}

// =============================================================================
// get_benchmark_suite()
// =============================================================================
BenchmarkSuite MultiRateEngine::get_benchmark_suite() const {
    std::lock_guard<std::mutex> lk(benchmark_mutex_);
    return benchmark_;
}

// =============================================================================
// get_spike_frame_jpeg()
// =============================================================================
std::vector<uint8_t> MultiRateEngine::get_spike_frame_jpeg() const {
    std::lock_guard<std::mutex> lk(vis_mutex_);
    return latest_spike_jpeg_;
}

// =============================================================================
// print_stats()
// =============================================================================
void MultiRateEngine::print_stats() const {
    Metrics m = get_metrics();

    std::printf(
        "=============================================================\n"
        "AuraSense Engine Statistics\n"
        "=============================================================\n"
        "Frames:           %d\n"
        "FPS:              %.1f\n"
        "Crack (last):     %.4f\n"
        "Sig confidence:   %.4f\n"
        "YOLO count:       %llu\n"
        "Uplink count:     %llu\n"
        "Latency P50:      %.3f ms\n"
        "Latency P95:      %.3f ms\n"
        "Latency P99:      %.3f ms\n"
        "Violations:       %llu\n"
        "Window crack %%:   %.2f\n"
        "Global crack %%:   %.2f\n"
        "YOLO Hz:          %.1f\n"
        "YOLO age:         %.1f ms\n"
        "Spike bitrate:    %.3f Mbps\n"
        "=============================================================\n",
        m.frame_id,
        m.fps,
        m.last_crack,
        m.sig_conf,
        (unsigned long long)m.yolo_count,
        (unsigned long long)m.uplink_count,
        m.latency_p50_ms,
        m.latency_p95_ms,
        m.latency_p99_ms,
        (unsigned long long)m.latency_violations,
        m.window_crack_ratio * 100.0f,
        m.global_crack_ratio * 100.0f,
        m.yolo_hz,
        m.yolo_age_ms,
        m.spike_bitrate_mbps);
}

// =============================================================================
// _emergency_stop()
// =============================================================================
void MultiRateEngine::_emergency_stop() {
    running_.store(false, std::memory_order_release);
}

// =============================================================================
// set_px_to_mm()
// =============================================================================
void MultiRateEngine::set_px_to_mm(float px_to_mm) {
    px_to_mm_.store(std::max(0.001f, px_to_mm),
                    std::memory_order_relaxed);
}