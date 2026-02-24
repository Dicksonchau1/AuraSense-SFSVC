+   1 // =============================================================================
+   2 // main.cpp — AuraSense Benchmark Runner
+   3 //
+   4 // Clean benchmark harness for the 6-lane MultiRateEngine.
+   5 // Reads a video file, pushes frames into the engine, prints metrics.
+   6 //
+   7 // CLI:
+   8 //   --video <path>      Video file (default: demo.mp4)
+   9 //   --frames <N>        Max frames to process (-1 = all)
+  10 //   --onnx <path>       YOLOv8 ONNX model path
+  11 //   --rt-priority       Enable SCHED_FIFO (requires root)
+  12 //   --cpu <N>           Pin main thread to CPU core N
+  13 //   --verbose           Print per-frame progress
+  14 //   --help              Show usage
+  15 //
+  16 // Dependencies: OpenCV 4.x, pthread, C++17
+  17 // =============================================================================
+  18 
+  19 #include "engine.h"
+  20 #include "uplink_serializer.h"
+  21 
+  22 #include <opencv2/opencv.hpp>
+  23 
+  24 #include <atomic>
+  25 #include <chrono>
+  26 #include <csignal>
+  27 #include <cstdio>
+  28 #include <cstdlib>
+  29 #include <cstring>
+  30 #include <string>
+  31 #include <thread>
+  32 
+  33 #ifdef __linux__
+  34 #include <sched.h>
+  35 #include <pthread.h>
+  36 #endif
+  37 
+  38 // =============================================================================
+  39 // Globals
+  40 // =============================================================================
+  41 static std::atomic<bool> g_running{true};
+  42 
+  43 // =============================================================================
+  44 // CLI Arguments
+  45 // =============================================================================
+  46 struct CliArgs {
+  47     std::string video_file   = "demo.mp4";
+  48     std::string onnx_path    = "yolov8n.onnx";
+  49     int   max_frames         = -1;
+  50     bool  rt_priority        = false;
+  51     int   cpu_core           = -1;
+  52     bool  verbose            = false;
+  53 };
+  54 
+  55 static void print_usage(const char* prog) {
+  56     std::printf(
+  57         "Usage: %s [options]\n\n"
+  58         "Options:\n"
+  59         "  --video <path>    Video file (default: demo.mp4)\n"
+  60         "  --frames <N>      Max frames (-1 = all)\n"
+  61         "  --onnx <path>     YOLOv8 ONNX model (default: yolov8n.onnx)\n"
+  62         "  --rt-priority     Enable SCHED_FIFO real-time priority\n"
+  63         "  --cpu <N>         Pin main thread to CPU core N\n"
+  64         "  --verbose, -v     Per-frame progress output\n"
+  65         "  --help, -h        Show this help\n\n"
+  66         "Examples:\n"
+  67         "  %s --video test.mp4 --frames 1000\n"
+  68         "  sudo %s --video demo.mp4 --rt-priority --cpu 2\n\n",
+  69         prog, prog, prog);
+  70 }
+  71 
+  72 static CliArgs parse_args(int argc, char** argv) {
+  73     CliArgs args;
+  74 
+  75     for (int i = 1; i < argc; ++i) {
+  76         std::string a = argv[i];
+  77 
+  78         if (a == "--help" || a == "-h") {
+  79             print_usage(argv[0]);
+  80             std::exit(0);
+  81         }
+  82         else if (a == "--video" && i + 1 < argc) {
+  83             args.video_file = argv[++i];
+  84         }
+  85         else if (a == "--frames" && i + 1 < argc) {
+  86             args.max_frames = std::atoi(argv[++i]);
+  87         }
+  88         else if (a == "--onnx" && i + 1 < argc) {
+  89             args.onnx_path = argv[++i];
+  90         }
+  91         else if (a == "--rt-priority") {
+  92             args.rt_priority = true;
+  93         }
+  94         else if (a == "--cpu" && i + 1 < argc) {
+  95             args.cpu_core = std::atoi(argv[++i]);
+  96         }
+  97         else if (a == "--verbose" || a == "-v") {
+  98             args.verbose = true;
+  99         }
+ 100         else {
+ 101             std::fprintf(stderr, "Unknown argument: %s\n", a.c_str());
+ 102             print_usage(argv[0]);
+ 103             std::exit(1);
+ 104         }
+ 105     }
+ 106 
+ 107     return args;
+ 108 }
+ 109 
+ 110 // =============================================================================
+ 111 // RT Priority Setup (Linux)
+ 112 // =============================================================================
+ 113 static bool setup_rt_priority() {
+ 114 #ifdef __linux__
+ 115     struct sched_param param;
+ 116     param.sched_priority = 99;
+ 117 
+ 118     if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
+ 119         std::fprintf(stderr,
+ 120             "[RT] Failed to set SCHED_FIFO: %s\n"
+ 121             "[RT] Run with sudo for real-time priority\n",
+ 122             std::strerror(errno));
+ 123         return false;
+ 124     }
+ 125     std::printf("[RT] SCHED_FIFO priority 99 enabled\n");
+ 126     return true;
+ 127 #else
+ 128     std::fprintf(stderr,
+ 129         "[RT] Real-time priority not supported on this platform\n");
+ 130     return false;
+ 131 #endif
+ 132 }
+ 133 
+ 134 static bool pin_cpu(int core_id) {
+ 135 #ifdef __linux__
+ 136     cpu_set_t cpuset;
+ 137     CPU_ZERO(&cpuset);
+ 138     CPU_SET(core_id, &cpuset);
+ 139 
+ 140     if (pthread_setaffinity_np(pthread_self(),
+ 141                                sizeof(cpu_set_t), &cpuset) != 0) {
+ 142         std::fprintf(stderr, "[CPU] Failed to pin to core %d\n", core_id);
+ 143         return false;
+ 144     }
+ 145     std::printf("[CPU] Pinned to core %d\n", core_id);
+ 146     return true;
+ 147 #else
+ 148     (void)core_id;
+ 149     return false;
+ 150 #endif
+ 151 }
+ 152 
+ 153 // =============================================================================
+ 154 // Signal Handler
+ 155 // =============================================================================
+ 156 static void signal_handler(int sig) {
+ 157     if (sig == SIGINT || sig == SIGTERM) {
+ 158         std::printf("\n[Main] Caught signal %d, shutting down...\n", sig);
+ 159         g_running.store(false, std::memory_order_relaxed);
+ 160     }
+ 161 }
+ 162 
+ 163 // =============================================================================
+ 164 // Main
+ 165 // =============================================================================
+ 166 int main(int argc, char** argv) {
+ 167     CliArgs args = parse_args(argc, argv);
+ 168 
+ 169     std::signal(SIGINT,  signal_handler);
+ 170     std::signal(SIGTERM, signal_handler);
+ 171 
+ 172     // CPU affinity
+ 173     if (args.cpu_core >= 0) {
+ 174         pin_cpu(args.cpu_core);
+ 175     }
+ 176 
+ 177     // RT priority
+ 178     if (args.rt_priority) {
+ 179         if (!setup_rt_priority()) {
+ 180             std::fprintf(stderr, "[Main] Continuing without RT priority\n");
+ 181         }
+ 182     }
+ 183 
+ 184     // Counters for callbacks
+ 185     std::atomic<int> ctrl_count{0};
+ 186     std::atomic<int> uplink_count{0};
+ 187 
+ 188     // Create engine with callbacks
+ 189     MultiRateEngine engine(
+ 190         [&](const ControlDecision& d) {
+ 191             ctrl_count.fetch_add(1, std::memory_order_relaxed);
+ 192             if (args.verbose && (ctrl_count.load() % 100 == 0)) {
+ 193                 std::printf("[CB] frame=%d action=%s crack=%.4f "
+ 194                             "latency=%.3fms\n",
+ 195                             d.frame_id, d.action.c_str(),
+ 196                             d.crack_score, d.control_latency_ms);
+ 197             }
+ 198         },
+ 199         [&](const UplinkPayload& p) {
+ 200             uplink_count.fetch_add(1, std::memory_order_relaxed);
+ 201         }
+ 202     );
+ 203 
+ 204     // Configure and start engine
+ 205     EngineConfig cfg;
+ 206     cfg.onnx_model_path = args.onnx_path;
+ 207     cfg.rt_priority     = args.rt_priority;
+ 208     cfg.cpu_core        = args.cpu_core;
+ 209     cfg.verbose         = args.verbose;
+ 210     cfg.max_frames      = args.max_frames;
+ 211 
+ 212     engine.start(cfg);
+ 213 
+ 214     // Open video
+ 215     cv::VideoCapture cap(args.video_file);
+ 216     if (!cap.isOpened()) {
+ 217         std::fprintf(stderr,
+ 218             "[Main] Failed to open video: %s\n", args.video_file.c_str());
+ 219         engine.stop();
+ 220         return 1;
+ 221     }
+ 222 
+ 223     double fps = cap.get(cv::CAP_PROP_FPS);
+ 224     int total_video_frames = static_cast<int>(
+ 225         cap.get(cv::CAP_PROP_FRAME_COUNT));
+ 226 
+ 227     std::printf(
+ 228         "=============================================================\n"
+ 229         "AuraSense 6-Lane Engine Benchmark\n"
+ 230         "=============================================================\n"
+ 231         "Video:       %s\n"
+ 232         "Resolution:  %dx%d\n"
+ 233         "FPS:         %.1f\n"
+ 234         "Frames:      %d (processing %s)\n"
+ 235         "ONNX model:  %s\n"
+ 236         "=============================================================\n\n",
+ 237         args.video_file.c_str(),
+ 238         static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH)),
+ 239         static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT)),
+ 240         fps,
+ 241         total_video_frames,
+ 242         (args.max_frames > 0)
+ 243             ? std::to_string(args.max_frames).c_str()
+ 244             : "ALL",
+ 245         args.onnx_path.c_str());
+ 246 
+ 247     // Feed frames
+ 248     auto bench_start = std::chrono::steady_clock::now();
+ 249     cv::Mat frame;
+ 250     int frames_pushed = 0;
+ 251 
+ 252     while (g_running.load(std::memory_order_relaxed)) {
+ 253         if (!cap.read(frame) || frame.empty()) {
+ 254             std::printf("\n[Main] End of video\n");
+ 255             break;
+ 256         }
+ 257 
+ 258         if (!frame.isContinuous()) {
+ 259             frame = frame.clone();
+ 260         }
+ 261 
+ 262         engine.push_frame(frame.data, frame.rows, frame.cols);
+ 263         frames_pushed++;
+ 264 
+ 265         if (args.verbose && (frames_pushed % 100 == 0)) {
+ 266             Metrics m = engine.get_metrics();
+ 267             std::printf("\r[%d/%d] FPS=%.1f P95=%.3fms YOLO=%llu crack=%.4f",
+ 268                 frames_pushed,
+ 269                 (args.max_frames > 0) ? args.max_frames : total_video_frames,
+ 270                 m.fps, m.latency_p95_ms,
+ 271                 (unsigned long long)m.yolo_count, m.last_crack);
+ 272             std::fflush(stdout);
+ 273         }
+ 274 
+ 275         if (args.max_frames > 0 && frames_pushed >= args.max_frames) {
+ 276             std::printf("\n[Main] Reached frame limit (%d)\n",
+ 277                         args.max_frames);
+ 278             break;
+ 279         }
+ 280     }
+ 281 
+ 282     // Allow pipeline to drain
+ 283     std::this_thread::sleep_for(std::chrono::milliseconds(500));
+ 284 
+ 285     auto bench_end = std::chrono::steady_clock::now();
+ 286     double bench_ms = std::chrono::duration<double, std::milli>(
+ 287         bench_end - bench_start).count();
+ 288 
+ 289     // Print final statistics
+ 290     std::printf("\n");
+ 291     engine.print_stats();
+ 292 
+ 293     std::printf(
+ 294         "\nBenchmark Summary:\n"
+ 295         "  Frames pushed:      %d\n"
+ 296         "  Wall-clock time:    %.2f s\n"
+ 297         "  Push throughput:    %.1f fps\n"
+ 298         "  Control callbacks:  %d\n"
+ 299         "  Uplink callbacks:   %d\n",
+ 300         frames_pushed,
+ 301         bench_ms / 1000.0,
+ 302         frames_pushed / (bench_ms / 1000.0),
+ 303         ctrl_count.load(),
+ 304         uplink_count.load());
+ 305 
+ 306     // Latency target check
+ 307     Metrics final_m = engine.get_metrics();
+ 308     bool p95_pass = (final_m.latency_p95_ms < 5.0f);
+ 309     std::printf("\nTarget P95 <5.0ms: %s (%.3f ms)\n",
+ 310                 p95_pass ? "PASS" : "FAIL",
+ 311                 final_m.latency_p95_ms);
+ 312 
+ 313     engine.stop();
+ 314 
+ 315     return p95_pass ? 0 : 1;
+ 316 }