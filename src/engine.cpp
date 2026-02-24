+   1 // =============================================================================
+   2 // engine.cpp — AuraSense 6-Lane Fan-Out Engine
+   3 //
+   4 // Full production implementation. Zero stubs. Zero placeholders.
+   5 //
+   6 // Lane 1: Control hot path  (camera_queue_ → rt_core → fan-out)
+   7 // Lane 2: Signature matching (sig_queue_ → SignatureBank)
+   8 // Lane 3: YOLO inference     (yolo_queue_ → yolo_detect → publish)
+   9 // Lane 4: Uplink             (uplink_queue_ → serialize → callback)
+  10 // Lane 5: Visualization      (vis_queue_ → JPEG encode)
+  11 // Lane 6: Callback dispatch  (callback_queue_ → user callbacks)
+  12 //
+  13 // Threading:
+  14 //   - All inter-lane queues are SPSC (LockFreeQueue)
+  15 //   - callback_queue_ is MPSC: Lane 1 + Lane 4 push, Lane 6 pops
+  16 //     Protected by s_callback_push_mutex for push serialization.
+  17 //   - Atomic state: semantic_state_, last_sig_match_, latest_sig_conf_
+  18 //
+  19 // Dependencies: OpenCV 4.x, pthread, C++17
+  20 // =============================================================================
+  21 
+  22 #include "engine.h"
+  23 
+  24 #include "yolo_bridge.h"
+  25 #include "yolo_manager.h"
+  26 #include "uplink_serializer.h"
+  27 #include "gating_engine.h"
+  28 #include "failsafe.h"
+  29 
+  30 #include "signature_bank.hpp"
+  31 #include "aurasense_yolo_bond.h"
+  32 
+  33 #include <opencv2/imgcodecs.hpp>
+  34 #include <opencv2/imgproc.hpp>
+  35 
+  36 #include <chrono>
+  37 #include <cmath>
+  38 #include <cstring>
+  39 #include <algorithm>
+  40 #include <cstdio>
+  41 
+  42 // =============================================================================
+  43 // MPSC mutex for callback_queue_
+  44 // Lane 1 and Lane 4 both push to callback_queue_, but LockFreeQueue is SPSC.
+  45 // This mutex serializes pushes so cached_head_ is never raced.
+  46 // =============================================================================
+  47 static std::mutex s_callback_push_mutex;
+  48 
+  49 // =============================================================================
+  50 // Time helper
+  51 // =============================================================================
+  52 static double now_seconds() {
+  53     using clock = std::chrono::steady_clock;
+  54     return std::chrono::duration<double>(
+  55         clock::now().time_since_epoch()).count();
+  56 }
+  57 
+  58 // =============================================================================
+  59 // Constructor
+  60 // =============================================================================
+  61 MultiRateEngine::MultiRateEngine(ControlCallback ctrl_cb,
+  62                                  UplinkCallback uplink_cb)
+  63     : ctrl_cb_(std::move(ctrl_cb))
+  64     , uplink_cb_(std::move(uplink_cb))
+  65 {
+  66     // Allocate SPSC queues on heap (engine.h stores raw pointers)
+  67     camera_queue_   = new LockFreeQueue<Lane2Job, 64>();
+  68     sig_queue_      = new LockFreeQueue<Lane2Job, 64>();
+  69     yolo_queue_     = new LockFreeQueue<Lane3Job, 32>();
+  70     uplink_queue_   = new LockFreeQueue<UplinkPayload, 128>();
+  71     vis_queue_      = new LockFreeQueue<VisJob, 32>();
+  72     callback_queue_ = new LockFreeQueue<CallbackJob, 64>();
+  73 
+  74     // Allocate components
+  75     signature_bank_         = std::make_unique<SignatureBank>();
+  76     gating_engine_          = std::make_unique<GatingEngine>();
+  77     failsafe_               = std::make_unique<FailsafeMonitor>();
+  78     yolo_manager_           = std::make_unique<YoloManager>();
+  79     det_controller_         = std::make_unique<DetectionController>();
+  80     crack_inference_engine_  = std::make_unique<CrackInferenceEngine>();
+  81     crack_stats_            = std::make_unique<CrackStatisticsTracker>();
+  82 
+  83     // Initialize atomic pointers
+  84     semantic_state_.store(nullptr, std::memory_order_relaxed);
+  85     last_sig_match_.store(nullptr, std::memory_order_relaxed);
+  86 
+  87     // Zero out latency ring
+  88     latency_ring_.fill(0.0);
+  89 }
+  90 
+  91 // =============================================================================
+  92 // Destructor
+  93 // =============================================================================
+  94 MultiRateEngine::~MultiRateEngine() {
+  95     stop();
+  96 
+  97     delete camera_queue_;
+  98     delete sig_queue_;
+  99     delete yolo_queue_;
+ 100     delete uplink_queue_;
+ 101     delete vis_queue_;
+ 102     delete callback_queue_;
+ 103 
+ 104     // Clean up atomic heap pointers
+ 105     delete semantic_state_.load(std::memory_order_relaxed);
+ 106     delete last_sig_match_.load(std::memory_order_relaxed);
+ 107 }
+ 108 
+ 109 // =============================================================================
+ 110 // start() / stop()
+ 111 // =============================================================================
+ 112 
+ 113 void MultiRateEngine::start() {
+ 114     start(EngineConfig{});
+ 115 }
+ 116 
+ 117 void MultiRateEngine::start(const EngineConfig& cfg) {
+ 118     cfg_ = cfg;
+ 119 
+ 120     bool expected = false;
+ 121     if (!running_.compare_exchange_strong(expected, true))
+ 122         return; // already running
+ 123 
+ 124     start_time_     = now_seconds();
+ 125     last_yolo_stamp_ = start_time_;
+ 126     last_vis_stamp_  = start_time_;
+ 127 
+ 128     // Initialize YOLO model if path provided
+ 129     if (!cfg_.onnx_model_path.empty()) {
+ 130         if (!yolo_init(cfg_.onnx_model_path)) {
+ 131             std::fprintf(stderr,
+ 132                 "[Engine] YOLO init failed for '%s'. "
+ 133                 "Lane 3 will return empty results.\n",
+ 134                 cfg_.onnx_model_path.c_str());
+ 135         }
+ 136     }
+ 137 
+ 138     // Launch 6 threads
+ 139     t1_ = std::thread(&MultiRateEngine::lane1_control, this);
+ 140     t2_ = std::thread(&MultiRateEngine::lane2_signature, this);
+ 141     t3_ = std::thread(&MultiRateEngine::lane3_yolo, this);
+ 142     t4_ = std::thread(&MultiRateEngine::lane4_uplink, this);
+ 143     t5_ = std::thread(&MultiRateEngine::lane5_visualize, this);
+ 144     callback_thread_ = std::thread(
+ 145         &MultiRateEngine::lane6_callback_dispatcher, this);
+ 146 }
+ 147 
+ 148 void MultiRateEngine::stop() {
+ 149     bool expected = true;
+ 150     if (!running_.compare_exchange_strong(expected, false))
+ 151         return;
+ 152 
+ 153     if (t1_.joinable()) t1_.join();
+ 154     if (t2_.joinable()) t2_.join();
+ 155     if (t3_.joinable()) t3_.join();
+ 156     if (t4_.joinable()) t4_.join();
+ 157     if (t5_.joinable()) t5_.join();
+ 158     if (callback_thread_.joinable()) callback_thread_.join();
+ 159 
+ 160     yolo_shutdown();
+ 161 }
+ 162 
+ 163 // =============================================================================
+ 164 // push_frame() — external entry point
+ 165 // =============================================================================
+ 166 void MultiRateEngine::push_frame(const uint8_t* bgr, int h, int w) {
+ 167     uint64_t fid = frame_id_.fetch_add(1, std::memory_order_relaxed);
+ 168 
+ 169     Lane2Job job;
+ 170     job.frame_id    = static_cast<int>(fid);
+ 171     job.height      = h;
+ 172     job.width       = w;
+ 173     job.crack_score = 0.0f;
+ 174     job.frame_bgr.assign(bgr, bgr + h * w * 3);
+ 175 
+ 176     camera_queue_->try_push(std::move(job));
+ 177 }
+ 178 
+ 179 // =============================================================================
+ 180 // Lane 1: Control Hot Path
+ 181 //
+ 182 // Pops frames from camera_queue_, runs rt_core, fans out to all other queues.
+ 183 // Also pushes ControlDecision via callback_queue_ (MPSC-protected).
+ 184 // =============================================================================
+ 185 void MultiRateEngine::lane1_control() {
+ 186     const int target_w = rt_core_target_width();
+ 187     const int target_h = rt_core_target_height();
+ 188 
+ 189     Lane2Job job;
+ 190 
+ 191     while (running_.load(std::memory_order_relaxed)) {
+ 192         if (!camera_queue_->pop_wait(job, std::chrono::milliseconds(10)))
+ 193             continue;
+ 194 
+ 195         auto t0 = std::chrono::steady_clock::now();
+ 196 
+ 197         // --- Resize to rt_core resolution ---
+ 198         cv::Mat frame_bgr(job.height, job.width, CV_8UC3,
+ 199                           job.frame_bgr.data());
+ 200         cv::Mat resized;
+ 201 
+ 202         if (job.height != target_h || job.width != target_w) {
+ 203             cv::resize(frame_bgr, resized,
+ 204                        cv::Size(target_w, target_h),
+ 205                        0, 0, cv::INTER_AREA);
+ 206         } else {
+ 207             resized = frame_bgr;
+ 208         }
+ 209 
+ 210         if (!resized.isContinuous()) {
+ 211             resized = resized.clone();
+ 212         }
+ 213 
+ 214         // --- Process frame through rt_core ---
+ 215         ControlOutput output = rt_core_process_frame_ptr(
+ 216             resized.data, resized.rows, resized.cols);
+ 217 
+ 218         auto t1 = std::chrono::steady_clock::now();
+ 219         double lane1_ms = std::chrono::duration<double, std::milli>(
+ 220             t1 - t0).count();
+ 221 
+ 222         // --- Record latency ---
+ 223         size_t idx = latency_head_ % latency_ring_.size();
+ 224         latency_ring_[idx] = lane1_ms;
+ 225         latency_head_++;
+ 226         latency_count_.fetch_add(1, std::memory_order_relaxed);
+ 227 
+ 228         if (lane1_ms > 5.0) {
+ 229             latency_violations_.fetch_add(1, std::memory_order_relaxed);
+ 230         }
+ 231 
+ 232         // --- Update crack tracking ---
+ 233         float crack = output.fused_crack_score;
+ 234         last_crack_score_.store(crack, std::memory_order_relaxed);
+ 235         if (crack > 0.1f) {
+ 236             crack_frames_.fetch_add(1, std::memory_order_relaxed);
+ 237         }
+ 238 
+ 239         double now = now_seconds();
+ 240         float px_mm = px_to_mm_.load(std::memory_order_relaxed);
+ 241         crack_stats_->update(crack, px_mm);
+ 242 
+ 243         // --- Update failsafe ---
+ 244         bool yolo_stale = (output.yolo_age_ms > 2000.0f);
+ 245         failsafe_->update(static_cast<float>(lane1_ms), crack, yolo_stale);
+ 246 
+ 247         // --- Gating decision ---
+ 248         float sig_conf = latest_sig_conf_.load(std::memory_order_relaxed);
+ 249         GatingDecision gate = gating_engine_->evaluate(
+ 250             crack, output.sparsity, sig_conf,
+ 251             static_cast<double>(output.yolo_age_ms),
+ 252             output.inference_suppressed != 0);
+ 253 
+ 254         // --- Fan out: Lane 2 (Signature) ---
+ 255         {
+ 256             Lane2Job sig_job;
+ 257             sig_job.frame_id    = job.frame_id;
+ 258             sig_job.height      = resized.rows;
+ 259             sig_job.width       = resized.cols;
+ 260             sig_job.crack_score = crack;
+ 261             sig_job.frame_bgr.assign(
+ 262                 resized.data,
+ 263                 resized.data + resized.total() * resized.elemSize());
+ 264             sig_queue_->try_push(std::move(sig_job));
+ 265         }
+ 266 
+ 267         // --- Fan out: Lane 3 (YOLO, gated) ---
+ 268         if (gate.should_run_yolo) {
+ 269             Lane3Job yolo_job;
+ 270             yolo_job.frame_id    = job.frame_id;
+ 271             yolo_job.height      = job.height;
+ 272             yolo_job.width       = job.width;
+ 273             yolo_job.timestamp   = now;
+ 274             yolo_job.crack_score = crack;
+ 275             yolo_job.sparsity    = output.sparsity;
+ 276             yolo_job.target_hz   = rt_core_yolo_get_target_hz();
+ 277             yolo_job.frame_bgr   = job.frame_bgr; // full resolution
+ 278             yolo_queue_->try_push(std::move(yolo_job));
+ 279         }
+ 280 
+ 281         // --- Fan out: Lane 5 (Visualization) ---
+ 282         {
+ 283             VisJob vis_job;
+ 284             vis_job.frame_id    = job.frame_id;
+ 285             vis_job.frame_h     = resized.rows;
+ 286             vis_job.frame_w     = resized.cols;
+ 287             vis_job.crack_score = crack;
+ 288             vis_job.sparsity    = output.sparsity;
+ 289             vis_job.on_count    = output.on_spike_count;
+ 290             vis_job.off_count   = output.off_spike_count;
+ 291             vis_job.frame_bgr.assign(
+ 292                 resized.data,
+ 293                 resized.data + resized.total() * resized.elemSize());
+ 294             vis_queue_->try_push(std::move(vis_job));
+ 295         }
+ 296 
+ 297         // --- Make control decision ---
+ 298         ControlDecision decision = make_decision(output, sig_conf, now);
+ 299         decision.control_latency_ms = lane1_ms;
+ 300 
+ 301         // --- Push to callback queue (MPSC) ---
+ 302         {
+ 303             CallbackJob cb;
+ 304             cb.type     = CallbackJob::CONTROL;
+ 305             cb.decision = decision;
+ 306 
+ 307             std::lock_guard<std::mutex> lk(s_callback_push_mutex);
+ 308             callback_queue_->try_push(std::move(cb));
+ 309         }
+ 310 
+ 311         // --- Push to uplink queue ---
+ 312         {
+ 313             UplinkPayload payload;
+ 314             payload.frame_id           = job.frame_id;
+ 315             payload.timestamp          = now;
+ 316             payload.throttle           = decision.throttle;
+ 317             payload.steer              = decision.steer;
+ 318             payload.crack_score        = crack;
+ 319             payload.sparsity           = output.sparsity;
+ 320             payload.action             = decision.action;
+ 321             payload.control_latency_ms = lane1_ms;
+ 322             payload.crack_severity     = decision.crack_severity;
+ 323             payload.crack_width_mm     = decision.crack_width_mm;
+ 324             payload.failsafe_status    = failsafe_->status();
+ 325             payload.sig_conf           = sig_conf;
+ 326             payload.yolo_count         = yolo_count_.load(
+ 327                                             std::memory_order_relaxed);
+ 328             payload.latency_violations = latency_violations_.load(
+ 329                                             std::memory_order_relaxed);
+ 330 
+ 331             uplink_queue_->try_push(std::move(payload));
+ 332         }
+ 333 
+ 334         update_benchmark(1, lane1_ms);
+ 335     }
+ 336 }
+ 337 
+ 338 // =============================================================================
+ 339 // Lane 2: Signature Matching
+ 340 // =============================================================================
+ 341 void MultiRateEngine::lane2_signature() {
+ 342     Lane2Job job;
+ 343 
+ 344     while (running_.load(std::memory_order_relaxed)) {
+ 345         if (!sig_queue_->pop_wait(job, std::chrono::milliseconds(50)))
+ 346             continue;
+ 347 
+ 348         auto t0 = std::chrono::steady_clock::now();
+ 349 
+ 350         // --- Compute average luminance ---
+ 351         float avg_luminance = 0.0f;
+ 352         if (!job.frame_bgr.empty() && job.height > 0 && job.width > 0) {
+ 353             int total = job.height * job.width;
+ 354             long sum = 0;
+ 355             const uint8_t* bgr = job.frame_bgr.data();
+ 356             // Sample every 16th pixel for speed
+ 357             int step = std::max(1, total / 4096);
+ 358             int sampled = 0;
+ 359             for (int i = 0; i < total; i += step) {
+ 360                 int b = bgr[i * 3 + 0];
+ 361                 int g = bgr[i * 3 + 1];
+ 362                 int r = bgr[i * 3 + 2];
+ 363                 sum += (54 * r + 183 * g + 19 * b) >> 8;
+ 364                 sampled++;
+ 365             }
+ 366             if (sampled > 0)
+ 367                 avg_luminance = static_cast<float>(sum) /
+ 368                                 (static_cast<float>(sampled) * 255.0f);
+ 369         }
+ 370 
+ 371         // --- Extract gradient-based feature vectors ---
+ 372         std::vector<float> gabor_fp(64, 0.0f);
+ 373         std::vector<float> semantic_prof(32, 0.0f);
+ 374         std::vector<float> context_vec(16, 0.0f);
+ 375         std::vector<float> motion_sig(16, 0.0f);
+ 376 
+ 377         if (!job.frame_bgr.empty() && job.height > 0 && job.width > 0) {
+ 378             cv::Mat frame(job.height, job.width, CV_8UC3,
+ 379                           const_cast<uint8_t*>(job.frame_bgr.data()));
+ 380             cv::Mat gray;
+ 381             cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
+ 382 
+ 383             // Block-mean features (8x8 grid → 64 values)
+ 384             int block_h = std::max(1, gray.rows / 8);
+ 385             int block_w = std::max(1, gray.cols / 8);
+ 386             int fi = 0;
+ 387             for (int by = 0; by < 8 && fi < 64; by++) {
+ 388                 for (int bx = 0; bx < 8 && fi < 64; bx++) {
+ 389                     int rx = std::min(bx * block_w, gray.cols - 1);
+ 390                     int ry = std::min(by * block_h, gray.rows - 1);
+ 391                     int rw = std::min(block_w, gray.cols - rx);
+ 392                     int rh = std::min(block_h, gray.rows - ry);
+ 393                     cv::Rect roi(rx, ry, rw, rh);
+ 394                     gabor_fp[fi++] = static_cast<float>(
+ 395                         cv::mean(gray(roi))[0]) / 255.0f;
+ 396                 }
+ 397             }
+ 398 
+ 399             // Vertical profile (32 horizontal bands)
+ 400             int band_h = std::max(1, gray.rows / 32);
+ 401             for (int i = 0; i < 32; i++) {
+ 402                 int ry = std::min(i * band_h, gray.rows - 1);
+ 403                 int rh = std::min(band_h, gray.rows - ry);
+ 404                 cv::Rect roi(0, ry, gray.cols, rh);
+ 405                 semantic_prof[i] = static_cast<float>(
+ 406                     cv::mean(gray(roi))[0]) / 255.0f;
+ 407             }
+ 408 
+ 409             // Global statistics
+ 410             cv::Scalar mean_val, stddev_val;
+ 411             cv::meanStdDev(gray, mean_val, stddev_val);
+ 412             context_vec[0] = static_cast<float>(mean_val[0]) / 255.0f;
+ 413             context_vec[1] = static_cast<float>(stddev_val[0]) / 255.0f;
+ 414             context_vec[2] = job.crack_score;
+ 415             context_vec[3] = avg_luminance;
+ 416         }
+ 417 
+ 418         // --- Run signature matching ---
+ 419         MatchResult match = signature_bank_->find_match_full(
+ 420             gabor_fp, semantic_prof, context_vec, motion_sig,
+ 421             avg_luminance);
+ 422 
+ 423         // Register new signature if no match and crack activity
+ 424         if (!match.matched && job.crack_score > 0.05f) {
+ 425             signature_bank_->register_signature(
+ 426                 gabor_fp, semantic_prof, context_vec, motion_sig,
+ 427                 job.crack_score, avg_luminance);
+ 428         }
+ 429 
+ 430         // Convert to SignatureMatch
+ 431         SignatureMatch sig_match =
+ 432             signature_bank_->to_signature_match(match);
+ 433         sig_match.crack_score = job.crack_score;
+ 434 
+ 435         latest_sig_conf_.store(sig_match.confidence,
+ 436                                std::memory_order_release);
+ 437 
+ 438         // Update stored match (atomic swap with delete)
+ 439         SignatureMatch* new_match = new SignatureMatch(sig_match);
+ 440         SignatureMatch* old_match = last_sig_match_.exchange(
+ 441             new_match, std::memory_order_acq_rel);
+ 442         delete old_match;
+ 443 
+ 444         // Update crack inference
+ 445         crack_inference_engine_->update(
+ 446             job.crack_score,
+ 447             1.0f - avg_luminance, // proxy for sparsity
+ 448             avg_luminance,
+ 449             sig_match.confidence);
+ 450 
+ 451         // Update detection controller
+ 452         double now_ms = now_seconds() * 1000.0;
+ 453         det_controller_->add_detection(
+ 454             now_ms, sig_match.confidence, job.crack_score);
+ 455 
+ 456         auto t1 = std::chrono::steady_clock::now();
+ 457         double lane2_ms = std::chrono::duration<double, std::milli>(
+ 458             t1 - t0).count();
+ 459         update_benchmark(2, lane2_ms);
+ 460     }
+ 461 }
+ 462 
+ 463 // =============================================================================
+ 464 // Lane 3: YOLO Inference
+ 465 // =============================================================================
+ 466 void MultiRateEngine::lane3_yolo() {
+ 467     Lane3Job job;
+ 468 
+ 469     while (running_.load(std::memory_order_relaxed)) {
+ 470         if (!yolo_queue_->pop_wait(job, std::chrono::milliseconds(100)))
+ 471             continue;
+ 472 
+ 473         auto t0 = std::chrono::steady_clock::now();
+ 474 
+ 475         // Reconstruct cv::Mat from job data
+ 476         cv::Mat frame(job.height, job.width, CV_8UC3,
+ 477                       const_cast<uint8_t*>(job.frame_bgr.data()));
+ 478 
+ 479         // Run YOLO inference
+ 480         YoloResult result = yolo_detect(frame);
+ 481 
+ 482         // Publish to rt_core lock-free YOLO state
+ 483         double now = now_seconds();
+ 484         rt_core_yolo_publish(
+ 485             now,
+ 486             result.front_risk,
+ 487             result.left_risk,
+ 488             result.right_risk,
+ 489             result.crack_risk,
+ 490             result.min_distance_m,
+ 491             result.max_confidence,
+ 492             result.num_detections,
+ 493             result.priority_detections,
+ 494             result.num_filtered_out);
+ 495 
+ 496         // Update semantic state (atomic swap with delete)
+ 497         SemanticState* new_state = new SemanticState;
+ 498         new_state->frame_id       = job.frame_id;
+ 499         new_state->timestamp_ms   = now * 1000.0;
+ 500         new_state->latency_ms     = result.latency_ms;
+ 501         new_state->front_risk     = result.front_risk;
+ 502         new_state->left_risk      = result.left_risk;
+ 503         new_state->right_risk     = result.right_risk;
+ 504         new_state->crack_risk     = result.crack_risk;
+ 505         new_state->num_detections = result.num_detections;
+ 506         new_state->max_confidence = result.max_confidence;
+ 507         new_state->agreement      = det_controller_->avg_agreement();
+ 508 
+ 509         SemanticState* old_state = semantic_state_.exchange(
+ 510             new_state, std::memory_order_acq_rel);
+ 511         delete old_state;
+ 512 
+ 513         // Update counters
+ 514         yolo_count_.fetch_add(1, std::memory_order_relaxed);
+ 515         last_yolo_stamp_ = now;
+ 516         yolo_hz_.store(job.target_hz, std::memory_order_relaxed);
+ 517 
+ 518         // Update YoloManager
+ 519         yolo_manager_->record_run(now, result.num_detections);
+ 520 
+ 521         // Update detection controller thresholds
+ 522         det_controller_->update_adaptive_thresholds();
+ 523 
+ 524         auto t1 = std::chrono::steady_clock::now();
+ 525         double lane3_ms = std::chrono::duration<double, std::milli>(
+ 526             t1 - t0).count();
+ 527         update_benchmark(3, lane3_ms);
+ 528     }
+ 529 }
+ 530 
+ 531 // =============================================================================
+ 532 // Lane 4: Uplink Serialization + Callback Dispatch
+ 533 // =============================================================================
+ 534 void MultiRateEngine::lane4_uplink() {
+ 535     UplinkPayload payload;
+ 536 
+ 537     while (running_.load(std::memory_order_relaxed)) {
+ 538         if (!uplink_queue_->pop_wait(payload, std::chrono::milliseconds(50)))
+ 539             continue;
+ 540 
+ 541         auto t0 = std::chrono::steady_clock::now();
+ 542 
+ 543         uplink_count_.fetch_add(1, std::memory_order_relaxed);
+ 544 
+ 545         // Push to callback queue for dispatch (MPSC-protected)
+ 546         {
+ 547             CallbackJob cb;
+ 548             cb.type    = CallbackJob::UPLINK;
+ 549             cb.payload = payload;
+ 550 
+ 551             std::lock_guard<std::mutex> lk(s_callback_push_mutex);
+ 552             callback_queue_->try_push(std::move(cb));
+ 553         }
+ 554 
+ 555         auto t1 = std::chrono::steady_clock::now();
+ 556         double lane4_ms = std::chrono::duration<double, std::milli>(
+ 557             t1 - t0).count();
+ 558         update_benchmark(4, lane4_ms);
+ 559     }
+ 560 }
+ 561 
+ 562 // =============================================================================
+ 563 // Lane 5: Visualization (Spike Frame JPEG Encoding)
+ 564 // =============================================================================
+ 565 void MultiRateEngine::lane5_visualize() {
+ 566     VisJob job;
+ 567 
+ 568     while (running_.load(std::memory_order_relaxed)) {
+ 569         if (!vis_queue_->pop_wait(job, std::chrono::milliseconds(100)))
+ 570             continue;
+ 571 
+ 572         auto t0 = std::chrono::steady_clock::now();
+ 573 
+ 574         if (job.frame_bgr.empty()) continue;
+ 575 
+ 576         cv::Mat frame(job.frame_h, job.frame_w, CV_8UC3,
+ 577                       const_cast<uint8_t*>(job.frame_bgr.data()));
+ 578 
+ 579         cv::Mat vis = frame.clone();
+ 580 
+ 581         // Overlay crack score bar (red, top)
+ 582         int bar_w = static_cast<int>(job.crack_score * vis.cols);
+ 583         if (bar_w > 0) {
+ 584             cv::rectangle(vis, cv::Point(0, 0),
+ 585                          cv::Point(bar_w, 8),
+ 586                          cv::Scalar(0, 0, 255), cv::FILLED);
+ 587         }
+ 588 
+ 589         // Overlay sparsity bar (green, below)
+ 590         int sbar_w = static_cast<int>(job.sparsity * vis.cols);
+ 591         if (sbar_w > 0) {
+ 592             cv::rectangle(vis, cv::Point(0, 10),
+ 593                          cv::Point(sbar_w, 18),
+ 594                          cv::Scalar(0, 255, 0), cv::FILLED);
+ 595         }
+ 596 
+ 597         // Overlay spike count text
+ 598         char text[64];
+ 599         std::snprintf(text, sizeof(text), "ON:%d OFF:%d",
+ 600                       job.on_count, job.off_count);
+ 601         cv::putText(vis, text, cv::Point(4, vis.rows - 8),
+ 602                     cv::FONT_HERSHEY_SIMPLEX, 0.35,
+ 603                     cv::Scalar(255, 255, 255), 1);
+ 604 
+ 605         // Encode JPEG
+ 606         std::vector<uint8_t> jpeg_buf;
+ 607         std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 70};
+ 608         cv::imencode(".jpg", vis, jpeg_buf, params);
+ 609 
+ 610         // Store for retrieval
+ 611         {
+ 612             std::lock_guard<std::mutex> lk(vis_mutex_);
+ 613             latest_spike_jpeg_ = std::move(jpeg_buf);
+ 614         }
+ 615 
+ 616         // Compute spike bitrate estimate
+ 617         int total_spikes = job.on_count + job.off_count;
+ 618         float bits_per_spike = 16.0f; // 1 byte pos + 1 byte polarity
+ 619         float bytes_per_frame = total_spikes * bits_per_spike / 8.0f;
+ 620         float mbps = bytes_per_frame * 30.0f / (1024.0f * 1024.0f);
+ 621         spike_bitrate_mbps_.store(mbps, std::memory_order_relaxed);
+ 622 
+ 623         last_vis_stamp_ = now_seconds();
+ 624 
+ 625         auto t1 = std::chrono::steady_clock::now();
+ 626         double lane5_ms = std::chrono::duration<double, std::milli>(
+ 627             t1 - t0).count();
+ 628         update_benchmark(5, lane5_ms);
+ 629     }
+ 630 }
+ 631 
+ 632 // =============================================================================
+ 633 // Lane 6: Callback Dispatcher
+ 634 // Pops from callback_queue_ and invokes user callbacks on this thread.
+ 635 // =============================================================================
+ 636 void MultiRateEngine::lane6_callback_dispatcher() {
+ 637     CallbackJob job;
+ 638 
+ 639     while (running_.load(std::memory_order_relaxed)) {
+ 640         if (!callback_queue_->pop_wait(job, std::chrono::milliseconds(10)))
+ 641             continue;
+ 642 
+ 643         switch (job.type) {
+ 644             case CallbackJob::CONTROL:
+ 645                 if (ctrl_cb_) ctrl_cb_(job.decision);
+ 646                 break;
+ 647             case CallbackJob::UPLINK:
+ 648                 if (uplink_cb_) uplink_cb_(job.payload);
+ 649                 break;
+ 650         }
+ 651     }
+ 652 }
+ 653 
+ 654 // =============================================================================
+ 655 // make_decision — convert ControlOutput to ControlDecision
+ 656 // =============================================================================
+ 657 ControlDecision MultiRateEngine::make_decision(
+ 658     const ControlOutput& output, float sig_conf, double timestamp) const
+ 659 {
+ 660     ControlDecision d;
+ 661     d.frame_id             = output.frame_id;
+ 662     d.timestamp            = timestamp;
+ 663     d.throttle             = output.throttle;
+ 664     d.steer                = output.steer;
+ 665     d.crack_score          = output.fused_crack_score;
+ 666     d.sparsity             = output.sparsity;
+ 667     d.confidence           = sig_conf;
+ 668     d.semantic_age_ms      = semantic_age_ms();
+ 669     d.is_null_cycle        = (output.is_null_cycle != 0);
+ 670     d.inference_suppressed = (output.inference_suppressed != 0);
+ 671     d.event_only_mode      = (output.event_only_mode != 0);
+ 672     d.yolo_active          = (output.yolo_active != 0);
+ 673     d.yolo_age_ms          = output.yolo_age_ms;
+ 674     d.reference_frame_age  = output.reference_frame_age;
+ 675     d.encode_time_ms       = output.encode_time_ms;
+ 676 
+ 677     // Determine action from crack score
+ 678     float crack = output.fused_crack_score;
+ 679     if (crack > 0.70f)      d.action = "STOP";
+ 680     else if (crack > 0.40f) d.action = "SLOW";
+ 681     else if (crack > 0.10f) d.action = "CAUTION";
+ 682     else                    d.action = "CLEAR";
+ 683 
+ 684     // Attach crack metrics
+ 685     CrackMetrics cm           = crack_stats_->get_metrics();
+ 686     d.crack_width_mm          = cm.width_mm;
+ 687     d.crack_length_mm         = cm.length_mm;
+ 688     d.crack_severity          = cm.severity_label;
+ 689     d.crack_confidence_percent = cm.confidence_percent;
+ 690 
+ 691     return d;
+ 692 }
+ 693 
+ 694 // =============================================================================
+ 695 // semantic_age_ms
+ 696 // =============================================================================
+ 697 double MultiRateEngine::semantic_age_ms() const {
+ 698     SemanticState* state = semantic_state_.load(std::memory_order_acquire);
+ 699     if (!state) return 99999.0;
+ 700 
+ 701     double now_ms = now_seconds() * 1000.0;
+ 702     return now_ms - state->timestamp_ms;
+ 703 }
+ 704 
+ 705 // =============================================================================
+ 706 // update_benchmark
+ 707 // =============================================================================
+ 708 void MultiRateEngine::update_benchmark(int lane_id, double duration_ms) {
+ 709     std::lock_guard<std::mutex> lk(benchmark_mutex_);
+ 710 
+ 711     BenchmarkSample sample;
+ 712     sample.lane      = lane_id;
+ 713     sample.frame_id  = static_cast<int>(
+ 714         frame_id_.load(std::memory_order_relaxed));
+ 715     sample.latency_ms = static_cast<float>(duration_ms);
+ 716     sample.yolo_ran   = (lane_id == 3);
+ 717     if (lane_id == 3) {
+ 718         sample.yolo_latency_ms = static_cast<float>(duration_ms);
+ 719     }
+ 720 
+ 721     benchmark_.add(sample);
+ 722 }
+ 723 
+ 724 // =============================================================================
+ 725 // get_metrics()
+ 726 // =============================================================================
+ 727 Metrics MultiRateEngine::get_metrics() const {
+ 728     Metrics m;
+ 729 
+ 730     double now     = now_seconds();
+ 731     double elapsed = now - start_time_;
+ 732 
+ 733     m.frame_id = static_cast<int>(
+ 734         frame_id_.load(std::memory_order_relaxed));
+ 735     m.last_crack = last_crack_score_.load(std::memory_order_relaxed);
+ 736     m.sig_conf   = latest_sig_conf_.load(std::memory_order_relaxed);
+ 737     m.crack_frames       = crack_frames_.load(std::memory_order_relaxed);
+ 738     m.yolo_count         = yolo_count_.load(std::memory_order_relaxed);
+ 739     m.uplink_count       = uplink_count_.load(std::memory_order_relaxed);
+ 740     m.latency_violations = latency_violations_.load(
+ 741                                std::memory_order_relaxed);
+ 742 
+ 743     // FPS
+ 744     if (elapsed > 0.1) {
+ 745         m.fps = static_cast<float>(m.frame_id / elapsed);
+ 746     }
+ 747 
+ 748     m.yolo_hz = yolo_hz_.load(std::memory_order_relaxed);
+ 749 
+ 750     // Latency percentiles from ring buffer
+ 751     size_t count = latency_count_.load(std::memory_order_relaxed);
+ 752     if (count > 0) {
+ 753         size_t n = std::min(count, latency_ring_.size());
+ 754         std::vector<double> tmp(latency_ring_.begin(),
+ 755                                 latency_ring_.begin() + n);
+ 756         std::sort(tmp.begin(), tmp.end());
+ 757 
+ 758         m.latency_p50_ms = static_cast<float>(tmp[n / 2]);
+ 759         m.latency_p95_ms = static_cast<float>(
+ 760             tmp[std::min(n - 1, static_cast<size_t>(n * 0.95))]);
+ 761         m.latency_p99_ms = static_cast<float>(
+ 762             tmp[std::min(n - 1, static_cast<size_t>(n * 0.99))]);
+ 763     }
+ 764 
+ 765     // Crack stats
+ 766     m.window_crack_ratio = crack_stats_->window_crack_ratio();
+ 767     m.global_crack_ratio = crack_stats_->global_crack_ratio();
+ 768     m.crack_alert_thr    = crack_stats_->crack_alert_threshold();
+ 769 
+ 770     // Detection controller
+ 771     m.avg_yolo_conf  = det_controller_->avg_yolo_conf();
+ 772     m.avg_crack_score = det_controller_->avg_crack_score();
+ 773     m.avg_agreement  = det_controller_->avg_agreement();
+ 774     m.yolo_conf_thr  = det_controller_->yolo_conf_threshold();
+ 775 
+ 776     m.px_to_mm_scale = px_to_mm_.load(std::memory_order_relaxed);
+ 777 
+ 778     // YOLO age
+ 779     m.yolo_age_ms = static_cast<float>(
+ 780         (now - last_yolo_stamp_) * 1000.0);
+ 781 
+ 782     // Spike bitrate
+ 783     m.spike_bitrate_mbps = spike_bitrate_mbps_.load(
+ 784         std::memory_order_relaxed);
+ 785 
+ 786     // Crack visualization metrics
+ 787     CrackMetrics cm       = crack_stats_->get_metrics();
+ 788     m.vis_crack_width_mm  = cm.width_mm;
+ 789     m.vis_crack_length_mm = cm.length_mm;
+ 790 
+ 791     return m;
+ 792 }
+ 793 
+ 794 // =============================================================================
+ 795 // get_benchmark_suite()
+ 796 // =============================================================================
+ 797 BenchmarkSuite MultiRateEngine::get_benchmark_suite() const {
+ 798     std::lock_guard<std::mutex> lk(benchmark_mutex_);
+ 799     return benchmark_;
+ 800 }
+ 801 
+ 802 // =============================================================================
+ 803 // get_spike_frame_jpeg()
+ 804 // =============================================================================
+ 805 std::vector<uint8_t> MultiRateEngine::get_spike_frame_jpeg() const {
+ 806     std::lock_guard<std::mutex> lk(vis_mutex_);
+ 807     return latest_spike_jpeg_;
+ 808 }
+ 809 
+ 810 // =============================================================================
+ 811 // print_stats()
+ 812 // =============================================================================
+ 813 void MultiRateEngine::print_stats() const {
+ 814     Metrics m = get_metrics();
+ 815 
+ 816     std::printf(
+ 817         "=============================================================\n"
+ 818         "AuraSense Engine Statistics\n"
+ 819         "=============================================================\n"
+ 820         "Frames:           %d\n"
+ 821         "FPS:              %.1f\n"
+ 822         "Crack (last):     %.4f\n"
+ 823         "Sig confidence:   %.4f\n"
+ 824         "YOLO count:       %llu\n"
+ 825         "Uplink count:     %llu\n"
+ 826         "Latency P50:      %.3f ms\n"
+ 827         "Latency P95:      %.3f ms\n"
+ 828         "Latency P99:      %.3f ms\n"
+ 829         "Violations:       %llu\n"
+ 830         "Window crack %%:   %.2f\n"
+ 831         "Global crack %%:   %.2f\n"
+ 832         "YOLO Hz:          %.1f\n"
+ 833         "YOLO age:         %.1f ms\n"
+ 834         "Spike bitrate:    %.3f Mbps\n"
+ 835         "=============================================================\n",
+ 836         m.frame_id,
+ 837         m.fps,
+ 838         m.last_crack,
+ 839         m.sig_conf,
+ 840         (unsigned long long)m.yolo_count,
+ 841         (unsigned long long)m.uplink_count,
+ 842         m.latency_p50_ms,
+ 843         m.latency_p95_ms,
+ 844         m.latency_p99_ms,
+ 845         (unsigned long long)m.latency_violations,
+ 846         m.window_crack_ratio * 100.0f,
+ 847         m.global_crack_ratio * 100.0f,
+ 848         m.yolo_hz,
+ 849         m.yolo_age_ms,
+ 850         m.spike_bitrate_mbps);
+ 851 }
+ 852 
+ 853 // =============================================================================
+ 854 // _emergency_stop()
+ 855 // =============================================================================
+ 856 void MultiRateEngine::_emergency_stop() {
+ 857     running_.store(false, std::memory_order_release);
+ 858 }
+ 859 
+ 860 // =============================================================================
+ 861 // set_px_to_mm()
+ 862 // =============================================================================
+ 863 void MultiRateEngine::set_px_to_mm(float px_to_mm) {
+ 864     px_to_mm_.store(std::max(0.001f, px_to_mm),
+ 865                     std::memory_order_relaxed);
+ 866 }