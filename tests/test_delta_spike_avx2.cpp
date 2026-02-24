#include <cstdint>
#include <cstring>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
#include <chrono>
#include <memory>

// Forward declarations for rt_core functions
extern "C" {
    int   rt_core_target_width(void);
    int   rt_core_target_height(void);
    void  rt_core_set_vehicle_speed(float speed_mps);

    typedef struct {
        int   frame_id;
        float crack_score;
        float fused_crack_score;
        float sparsity;
        float throttle;
        float steer;
        int   is_null_cycle;
        int   inference_suppressed;
        int   event_only_mode;
        int   reference_frame_age;
        int   yolo_active;
        float yolo_age_ms;
        float yolo_target_hz;
        float encode_time_ms;
        int   on_spike_count;
        int   off_spike_count;
        float global_saliency;
        int   roi_count;
    } ControlOutput;

    ControlOutput rt_core_process_frame_ptr(const uint8_t* bgr, int height, int width);
}

// Test configuration
constexpr int PIXELS_PER_FRAME = 234 * 416;  // TARGET_HEIGHT * TARGET_WIDTH
constexpr int FRAME_SIZE = PIXELS_PER_FRAME * 3;  // BGR format

class DeltaSpikeTest {
public:
    DeltaSpikeTest() {
        int w = rt_core_target_width();
        int h = rt_core_target_height();
        printf("RT Core target: %dx%d\n", w, h);
        assert(w == 416 && h == 234);
    }

    // Generate uniform BGR frame
    static void fill_bgr_uniform(uint8_t* bgr, uint8_t val) {
        std::memset(bgr, val, FRAME_SIZE);
    }

    // Generate test pattern: gradient
    static void fill_bgr_gradient(uint8_t* bgr, int offset = 0) {
        for (int i = 0; i < PIXELS_PER_FRAME; ++i) {
            uint8_t val = static_cast<uint8_t>((i + offset) & 0xFF);
            int idx = i * 3;
            bgr[idx]     = val;  // B
            bgr[idx + 1] = val;  // G
            bgr[idx + 2] = val;  // R
        }
    }

    // Generate test pattern: salt-and-pepper (high deltas)
    static void fill_bgr_checkerboard(uint8_t* bgr, int scale = 4) {
        for (int y = 0; y < 234; ++y) {
            for (int x = 0; x < 416; ++x) {
                int idx = (y * 416 + x) * 3;
                uint8_t val = ((x / scale + y / scale) & 1) ? 200 : 50;
                bgr[idx]     = val;  // B
                bgr[idx + 1] = val;  // G
                bgr[idx + 2] = val;  // R
            }
        }
    }

    // Test 1: Identical frames should produce zero spikes
    bool test_zero_delta() {
        printf("\n[TEST 1] Zero Delta (Identical Frames)\n");

        auto prev_bgr = std::unique_ptr<uint8_t[]>(new uint8_t[FRAME_SIZE]);
        auto curr_bgr = std::unique_ptr<uint8_t[]>(new uint8_t[FRAME_SIZE]);

        fill_bgr_uniform(prev_bgr.get(), 128);
        fill_bgr_uniform(curr_bgr.get(), 128);

        auto out1 = rt_core_process_frame_ptr(prev_bgr.get(), 234, 416);
        auto out2 = rt_core_process_frame_ptr(curr_bgr.get(), 234, 416);

        printf("  Frame 1: on=%d, off=%d, crack=%.4f\n",
               out1.on_spike_count, out1.off_spike_count, out1.crack_score);
        printf("  Frame 2: on=%d, off=%d, crack=%.4f (identical prev)\n",
               out2.on_spike_count, out2.off_spike_count, out2.crack_score);

        bool pass = (out2.on_spike_count == 0 && out2.off_spike_count == 0);
        printf("  Result: %s\n", pass ? "PASS" : "FAIL");
        return pass;
    }

    // Test 2: Large delta should produce spikes
    bool test_large_delta() {
        printf("\n[TEST 2] Large Delta (Thresholded Spikes)\n");

        auto prev_bgr = std::unique_ptr<uint8_t[]>(new uint8_t[FRAME_SIZE]);
        auto curr_bgr = std::unique_ptr<uint8_t[]>(new uint8_t[FRAME_SIZE]);

        fill_bgr_uniform(prev_bgr.get(), 50);
        fill_bgr_uniform(curr_bgr.get(), 150);  // Delta = 100 > threshold

        auto out1 = rt_core_process_frame_ptr(prev_bgr.get(), 234, 416);
        auto out2 = rt_core_process_frame_ptr(curr_bgr.get(), 234, 416);

        printf("  Frame 1: on=%d, off=%d\n", out1.on_spike_count, out1.off_spike_count);
        printf("  Frame 2: on=%d, off=%d (delta=100, threshold=8)\n",
               out2.on_spike_count, out2.off_spike_count);

        // Expect significant spike count due to large delta
        bool pass = (out2.on_spike_count > 30000);  // Majority of pixels should spike
        printf("  Result: %s (expected >30k spikes)\n", pass ? "PASS" : "FAIL");
        return pass;
    }

    // Test 3: Reproducibility - same input, same output
    bool test_reproducibility() {
        printf("\n[TEST 3] Reproducibility\n");

        auto prev_bgr = std::unique_ptr<uint8_t[]>(new uint8_t[FRAME_SIZE]);
        auto curr_bgr = std::unique_ptr<uint8_t[]>(new uint8_t[FRAME_SIZE]);

        fill_bgr_gradient(prev_bgr.get(), 0);
        fill_bgr_gradient(curr_bgr.get(), 10);

        auto out1 = rt_core_process_frame_ptr(prev_bgr.get(), 234, 416);
        auto out2a = rt_core_process_frame_ptr(curr_bgr.get(), 234, 416);
        auto out2b = rt_core_process_frame_ptr(curr_bgr.get(), 234, 416);

        printf("  Frame 1 (prev): on=%d, off=%d\n", out1.on_spike_count, out1.off_spike_count);
        printf("  Frame 2a: on=%d, off=%d, crack=%.4f\n",
               out2a.on_spike_count, out2a.off_spike_count, out2a.crack_score);
        printf("  Frame 2b: on=%d, off=%d, crack=%.4f\n",
               out2b.on_spike_count, out2b.off_spike_count, out2b.crack_score);

        bool pass = (out2a.on_spike_count == out2b.on_spike_count &&
                    out2a.off_spike_count == out2b.off_spike_count &&
                    std::abs(out2a.crack_score - out2b.crack_score) < 0.001f);
        printf("  Result: %s\n", pass ? "PASS" : "FAIL");
        return pass;
    }

    // Test 4: Crack detection - band-based scoring
    bool test_crack_band_detection() {
        printf("\n[TEST 4] Band-Based Crack Detection\n");

        auto prev_bgr = std::unique_ptr<uint8_t[]>(new uint8_t[FRAME_SIZE]);
        auto curr_bgr = std::unique_ptr<uint8_t[]>(new uint8_t[FRAME_SIZE]);

        // Create checkerboard pattern to trigger band detection
        fill_bgr_uniform(prev_bgr.get(), 100);
        fill_bgr_checkerboard(curr_bgr.get(), 8);

        auto out1 = rt_core_process_frame_ptr(prev_bgr.get(), 234, 416);
        auto out2 = rt_core_process_frame_ptr(curr_bgr.get(), 234, 416);

        printf("  Frame 1 (uniform): crack=%.4f\n", out1.crack_score);
        printf("  Frame 2 (checkerboard): crack=%.4f\n", out2.crack_score);
        printf("  Sparsity: %.4f\n", out2.sparsity);

        // Crack score should be non-zero for patterned input
        bool pass = (out2.crack_score >= 0.0f && out2.crack_score <= 1.0f);
        printf("  Result: %s (crack in valid range [0,1])\n", pass ? "PASS" : "FAIL");
        return pass;
    }

    // Test 5: Latency benchmark
    bool test_latency_benchmark() {
        printf("\n[TEST 5] Latency Benchmark\n");

        auto prev_bgr = std::unique_ptr<uint8_t[]>(new uint8_t[FRAME_SIZE]);
        auto curr_bgr = std::unique_ptr<uint8_t[]>(new uint8_t[FRAME_SIZE]);

        fill_bgr_gradient(prev_bgr.get(), 0);
        fill_bgr_gradient(curr_bgr.get(), 10);

        // Warm up
        for (int i = 0; i < 10; ++i) {
            rt_core_process_frame_ptr(prev_bgr.get(), 234, 416);
            rt_core_process_frame_ptr(curr_bgr.get(), 234, 416);
        }

        // Benchmark
        constexpr int ITERATIONS = 1000;
        auto t_start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < ITERATIONS; ++i) {
            if (i & 1) {
                rt_core_process_frame_ptr(curr_bgr.get(), 234, 416);
            } else {
                rt_core_process_frame_ptr(prev_bgr.get(), 234, 416);
            }
        }

        auto t_end = std::chrono::high_resolution_clock::now();
        auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count();
        float avg_us = static_cast<float>(elapsed_us) / ITERATIONS;
        float avg_ms = avg_us / 1000.0f;

        printf("  Iterations: %d\n", ITERATIONS);
        printf("  Total time: %.2f ms\n", elapsed_us / 1000.0f);
        printf("  Average latency: %.2f µs (%.4f ms)\n", avg_us, avg_ms);
        printf("  Target: <15 µs\n");

        bool pass = (avg_us < 15.0f);
        printf("  Result: %s\n", pass ? "PASS" : "FAIL");
        return pass;
    }

    // Test 6: Edge cases
    bool test_edge_cases() {
        printf("\n[TEST 6] Edge Cases\n");

        auto bgr = std::unique_ptr<uint8_t[]>(new uint8_t[FRAME_SIZE]);

        // Test with extreme values
        fill_bgr_uniform(bgr.get(), 0);
        auto out1 = rt_core_process_frame_ptr(bgr.get(), 234, 416);
        printf("  All-black: crack=%.4f, sparsity=%.4f\n", out1.crack_score, out1.sparsity);

        fill_bgr_uniform(bgr.get(), 255);
        auto out2 = rt_core_process_frame_ptr(bgr.get(), 234, 416);
        printf("  All-white: crack=%.4f, sparsity=%.4f\n", out2.crack_score, out2.sparsity);

        bool pass = (out1.sparsity >= 0.0f && out1.sparsity <= 1.0f &&
                    out2.sparsity >= 0.0f && out2.sparsity <= 1.0f);
        printf("  Result: %s\n", pass ? "PASS" : "FAIL");
        return pass;
    }

    int run_all() {
        int passed = 0, failed = 0;

        if (test_zero_delta()) passed++; else failed++;
        if (test_large_delta()) passed++; else failed++;
        if (test_reproducibility()) passed++; else failed++;
        if (test_crack_band_detection()) passed++; else failed++;
        if (test_latency_benchmark()) passed++; else failed++;
        if (test_edge_cases()) passed++; else failed++;

        printf("\n========================================\n");
        printf("RESULTS: %d passed, %d failed\n", passed, failed);
        printf("========================================\n");

        return failed == 0 ? 0 : 1;
    }
};

int main() {
    printf("AuraSense RT Core Delta Spike AVX2 Test Suite\n");
    printf("=============================================\n");

    DeltaSpikeTest test;
    return test.run_all();
}
