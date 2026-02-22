// =============================================================================
// main.cpp — AuraSense Production WebSocket + Engine Host
// =============================================================================

#include "engine.h"

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <nlohmann/json.hpp>

#include <thread>
#include <iostream>
#include <atomic>
#include <memory>
#include <vector>
#include <chrono>

#include <opencv2/opencv.hpp>

using tcp        = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;
using json       = nlohmann::json;

static std::atomic<bool> g_running{true};

// =============================================================================
// WebSocket Session
// =============================================================================

class WsSession : public std::enable_shared_from_this<WsSession>
{
public:
    explicit WsSession(tcp::socket socket)
        : ws_(std::move(socket)) {}

    void start()
    {
        ws_.accept();
    }

    void send_text(const std::string& msg)
    {
        try {
            ws_.text(true);
            ws_.write(boost::asio::buffer(msg));
        } catch (const boost::system::system_error&) {
            // Client dropped; ignore so engine keeps running
        }
    }

    void send_binary(const std::vector<uint8_t>& data)
    {
        try {
            ws_.binary(true);
            ws_.write(boost::asio::buffer(data));
        } catch (const boost::system::system_error&) {
            // Client dropped; ignore
        }
    }

private:
    websocket::stream<tcp::socket> ws_;
};

// =============================================================================
// WebSocket Server
// =============================================================================

class WsServer
{
public:
    WsServer(boost::asio::io_context& ioc, int port)
        : acceptor_(ioc, tcp::endpoint(tcp::v4(), port))
    {
        do_accept();
    }

    std::vector<std::shared_ptr<WsSession>> sessions;

private:
    void do_accept()
    {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                {
                    auto session =
                        std::make_shared<WsSession>(std::move(socket));
                    session->start();
                    sessions.push_back(session);
                }
                do_accept();
            });
    }

    tcp::acceptor acceptor_;
};

// =============================================================================
// Performance Benchmark Harness (video-driven)
// =============================================================================

class BenchmarkHarness
{
public:
    explicit BenchmarkHarness(MultiRateEngine& engine)
        : engine_(engine) {}

    // Play a video file and feed frames into the engine
    void run_from_video(const std::string& filename)
    {
        cv::VideoCapture cap(filename);
        if (!cap.isOpened()) {
            std::cerr << "[Benchmark] Failed to open video: " << filename << "\n";
            return;
        }

        int frames = 0;
        cv::Mat frame_bgr;

        std::cout << "[Benchmark] Playing video: " << filename << "\n";

        while (g_running.load(std::memory_order_relaxed))
        {
            if (!cap.read(frame_bgr)) {
                std::cout << "[Benchmark] End of video\n";
                break;
            }
            if (frame_bgr.empty()) {
                continue;
            }

            const int h = frame_bgr.rows;
            const int w = frame_bgr.cols;

            if (!frame_bgr.isContinuous()) {
                frame_bgr = frame_bgr.clone();
            }

            engine_.push_frame(frame_bgr.data, h, w);
            ++frames;

            // ~60 Hz playback
            std::this_thread::sleep_for(
                std::chrono::milliseconds(16));
        }

        std::cout << "\nVideo playback complete.\n";
        std::cout << "Frames pushed: " << frames << "\n";
        engine_.print_stats();
    }

private:
    MultiRateEngine& engine_;
};

// =============================================================================
// Main
// =============================================================================

int main()
{
    boost::asio::io_context ioc;

    // WebSocket servers
    WsServer metrics_server(ioc, 9001);
    WsServer spike_server  (ioc, 9002);

    // Engine
    MultiRateEngine engine(
        [](const ControlDecision&) {},
        [](const UplinkPayload&) {}
    );

    engine.start();

    // Metrics Broadcast Thread (10 Hz)
    std::thread metrics_thread([&]() {
        while (g_running.load(std::memory_order_relaxed))
        {
            Metrics m = engine.get_metrics();

            json j;
            // React dashboard expects "hz" here
            j["hz"]                 = m.fps;
            j["p95"]                = m.latency_p95_ms;
            j["p99"]                = m.latency_p99_ms;
            j["last_crack"]         = m.last_crack;
            j["yolo_hz"]            = m.yolo_hz;
            j["spike_bitrate_mbps"] = m.spike_bitrate_mbps;
            j["window_crack_ratio"] = m.window_crack_ratio;
            j["global_crack_ratio"] = m.global_crack_ratio;

            std::string msg = j.dump();

            for (auto& s : metrics_server.sessions)
                if (s) s->send_text(msg);

            std::this_thread::sleep_for(
                std::chrono::milliseconds(100));
        }
    });

    // Spike JPEG Broadcast Thread (~30 Hz)
    std::thread spike_thread([&]() {
        while (g_running.load(std::memory_order_relaxed))
        {
            auto jpeg = engine.get_spike_frame_jpeg();
            if (!jpeg.empty())
            {
                for (auto& s : spike_server.sessions)
                    if (s) s->send_binary(jpeg);
            }

            std::this_thread::sleep_for(
                std::chrono::milliseconds(33));
        }
    });

    // Benchmark Harness: play demo.mp4 next to engine.exe
    BenchmarkHarness bench(engine);
    std::thread bench_thread([&]() {
        bench.run_from_video("demo.mp4");
    });

    // Run IO context (accepts WebSocket connections)
    ioc.run();

    // Shutdown
    g_running.store(false, std::memory_order_relaxed);

    if (metrics_thread.joinable()) metrics_thread.join();
    if (spike_thread.joinable())   spike_thread.join();
    if (bench_thread.joinable())   bench_thread.join();

    engine.stop();

    return 0;
}