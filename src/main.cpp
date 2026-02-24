// =============================================================================
// main.cpp — AuraSense Benchmark Runner
//
// Clean benchmark harness for the 6-lane MultiRateEngine.
// Reads a video file, pushes frames into the engine, prints metrics.
//
// CLI:
//   --video <path>      Video file (default: demo.mp4)
//   --frames <N>        Max frames to process (-1 = all)
//   --onnx <path>       YOLOv8 ONNX model path
//   --rt-priority       Enable SCHED_FIFO (requires root)
//   --cpu <N>           Pin main thread to CPU core N
//   --verbose           Print per-frame progress
//   --help              Show usage
//
// Dependencies: OpenCV 4.x, pthread, C++17
// =============================================================================

#include "engine.h"
#include "uplink_serializer.h"

#include <opencv2/opencv.hpp>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>

#ifdef __linux__
#include <sched.h>
#include <pthread.h>
#endif

// =============================================================================
// Globals
// =============================================================================
static std::atomic<bool> g_running{true};

// =============================================================================
// CLI Arguments
// =============================================================================
struct CliArgs {
    std::string video_file   = "demo.mp4";
    std::string onnx_path    = "yolov8n.onnx";
    int   max_frames         = -1;
    bool  rt_priority        = false;
    int   cpu_core           = -1;
    bool  verbose            = false;
};

static void print_usage(const char* prog) {
    std::printf(
        "Usage: %s [options]\n\n"
        "Options:\n"
        "  --video <path>    Video file (default: demo.mp4)\n"
        "  --frames <N>      Max frames (-1 = all)\n"
        "  --onnx <path>     YOLOv8 ONNX model (default: yolov8n.onnx)\n"
        "  --rt-priority     Enable SCHED_FIFO real-time priority\n"
        "  --cpu <N>         Pin main thread to CPU core N\n"
        "  --verbose, -v     Per-frame progress output\n"
        "  --help, -h        Show this help\n\n"
        "Examples:\n"
        "  %s --video test.mp4 --frames 1000\n"
        "  sudo %s --video demo.mp4 --rt-priority --cpu 2\n\n",
        prog, prog, prog);
}

static CliArgs parse_args(int argc, char** argv) {
    CliArgs args;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];

        if (a == "--help" || a == "-h") {
            print_usage(argv[0]);
            std::exit(0);
        }
        else if (a == "--video" && i + 1 < argc) {
            args.video_file = argv[++i];
        }
        else if (a == "--frames" && i + 1 < argc) {
            args.max_frames = std::atoi(argv[++i]);
        }
        else if (a == "--onnx" && i + 1 < argc) {
            args.onnx_path = argv[++i];
        }
        else if (a == "--rt-priority") {
            args.rt_priority = true;
        }
        else if (a == "--cpu" && i + 1 < argc) {
            args.cpu_core = std::atoi(argv[++i]);
        }
        else if (a == "--verbose" || a == "-v") {
            args.verbose = true;
        }
        else {
            std::fprintf(stderr, "Unknown argument: %s\n", a.c_str());
            print_usage(argv[0]);
            std::exit(1);
        }
    }

    return args;
}

// =============================================================================
// RT Priority Setup (Linux)
// =============================================================================
static bool setup_rt_priority() {
#ifdef __linux__
    struct sched_param param;
    param.sched_priority = 99;

    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
        std::fprintf(stderr,
            "[RT] Failed to set SCHED_FIFO: %s\n"
            "[RT] Run with sudo for real-time priority\n",
            std::strerror(errno));
        return false;
    }
    std::printf("[RT] SCHED_FIFO priority 99 enabled\n");
    return true;
#else
    std::fprintf(stderr,
        "[RT] Real-time priority not supported on this platform\n");
    return false;
#endif
}

static bool pin_cpu(int core_id) {
#ifdef __linux__
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    if (pthread_setaffinity_np(pthread_self(),
                               sizeof(cpu_set_t), &cpuset) != 0) {
        std::fprintf(stderr, "[CPU] Failed to pin to core %d\n", core_id);
        return false;
    }
    std::printf("[CPU] Pinned to core %d\n", core_id);
    return true;
#else
    (void)core_id;
    return false;
#endif
}

// =============================================================================
// Signal Handler
// =============================================================================
static void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        std::printf("\n[Main] Caught signal %d, shutting down...\n", sig);
        g_running.store(false, std::memory_order_relaxed);
    }
}

// =============================================================================
// Main
// =============================================================================
int main(int argc, char** argv) {
    CliArgs args = parse_args(argc, argv);

    std::signal(SIGINT,  signal_handler);
    std::signal(SIGTERM, signal_handler);

    // CPU affinity
    if (args.cpu_core >= 0) {
        pin_cpu(args.cpu_core);
    }

    // RT priority
    if (args.rt_priority) {
        if (!setup_rt_priority()) {
            std::fprintf(stderr, "[Main] Continuing without RT priority\n");
        }
    }

    // Counters for callbacks
    std::atomic<int> ctrl_count{0};
    std::atomic<int> uplink_count{0};

    // Create engine with callbacks
    MultiRateEngine engine(
        [&](const ControlDecision& d) {
            ctrl_count.fetch_add(1, std::memory_order_relaxed);
            if (args.verbose && (ctrl_count.load() % 100 == 0)) {
                std::printf("[CB] frame=%d action=%s crack=%.4f "
                            "latency=%.3fms\n",
                            d.frame_id, d.action.c_str(),
                            d.crack_score, d.control_latency_ms);
            }
        },
        [&](const UplinkPayload& p) {
            uplink_count.fetch_add(1, std::memory_order_relaxed);
        }
    );

    // Configure and start engine
    EngineConfig cfg;
    cfg.onnx_model_path = args.onnx_path;
    cfg.rt_priority     = args.rt_priority;
    cfg.cpu_core        = args.cpu_core;
    cfg.verbose         = args.verbose;
    cfg.max_frames      = args.max_frames;

    engine.start(cfg);

    // Open video
    cv::VideoCapture cap(args.video_file);
    if (!cap.isOpened()) {
        std::fprintf(stderr,
            "[Main] Failed to open video: %s\n", args.video_file.c_str());
        engine.stop();
        return 1;
    }

    double fps = cap.get(cv::CAP_PROP_FPS);
    int total_video_frames = static_cast<int>(
        cap.get(cv::CAP_PROP_FRAME_COUNT));

    std::printf(
        "=============================================================\n"
        "AuraSense 6-Lane Engine Benchmark\n"
        "=============================================================\n"
        "Video:       %s\n"
        "Resolution:  %dx%d\n"
        "FPS:         %.1f\n"
        "Frames:      %d (processing %s)\n"
        "ONNX model:  %s\n"
        "=============================================================\n\n",
        args.video_file.c_str(),
        static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH)),
        static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT)),
        fps,
        total_video_frames,
        (args.max_frames > 0)
            ? std::to_string(args.max_frames).c_str()
            : "ALL",
        args.onnx_path.c_str());

    // Feed frames
    auto bench_start = std::chrono::steady_clock::now();
    cv::Mat frame;
    int frames_pushed = 0;

    while (g_running.load(std::memory_order_relaxed)) {
        if (!cap.read(frame) || frame.empty()) {
            std::printf("\n[Main] End of video\n");
            break;
        }

        if (!frame.isContinuous()) {
            frame = frame.clone();
        }

        engine.push_frame(frame.data, frame.rows, frame.cols);
        frames_pushed++;

        if (args.verbose && (frames_pushed % 100 == 0)) {
            Metrics m = engine.get_metrics();
            std::printf("\r[%d/%d] FPS=%.1f P95=%.3fms YOLO=%llu crack=%.4f",
                frames_pushed,
                (args.max_frames > 0) ? args.max_frames : total_video_frames,
                m.fps, m.latency_p95_ms,
                (unsigned long long)m.yolo_count, m.last_crack);
            std::fflush(stdout);
        }

        if (args.max_frames > 0 && frames_pushed >= args.max_frames) {
            std::printf("\n[Main] Reached frame limit (%d)\n",
                        args.max_frames);
            break;
        }
    }

    // Allow pipeline to drain
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    auto bench_end = std::chrono::steady_clock::now();
    double bench_ms = std::chrono::duration<double, std::milli>(
        bench_end - bench_start).count();

    // Print final statistics
    std::printf("\n");
    engine.print_stats();

    std::printf(
        "\nBenchmark Summary:\n"
        "  Frames pushed:      %d\n"
        "  Wall-clock time:    %.2f s\n"
        "  Push throughput:    %.1f fps\n"
        "  Control callbacks:  %d\n"
        "  Uplink callbacks:   %d\n",
        frames_pushed,
        bench_ms / 1000.0,
        frames_pushed / (bench_ms / 1000.0),
        ctrl_count.load(),
        uplink_count.load());

    // Latency target check
    Metrics final_m = engine.get_metrics();
    bool p95_pass = (final_m.latency_p95_ms < 5.0f);
    std::printf("\nTarget P95 <5.0ms: %s (%.3f ms)\n",
                p95_pass ? "PASS" : "FAIL",
                final_m.latency_p95_ms);

    engine.stop();

    return p95_pass ? 0 : 1;
}