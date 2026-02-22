#include "rt_core.h"

#include <immintrin.h>
#include <cstring>
#include <cmath>
#include <cstdio>

#ifdef _WIN32
    #include <windows.h>
#else
    #define _GNU_SOURCE
    #include <pthread.h>
    #include <sched.h>
    #include <unistd.h>
#endif

// -----------------------------------------------------------------------------
// Compile-time constants
// -----------------------------------------------------------------------------
static constexpr int FRAME_W   = 416;
static constexpr int FRAME_H   = 234;
static constexpr int TARGET_HZ = 60;

// -----------------------------------------------------------------------------
// Internal state (no heap)
// -----------------------------------------------------------------------------
static int    s_frame_id         = 0;
static float  s_last_yolo_crack  = 0.0f;
static double s_last_yolo_time_s = 0.0;

// -----------------------------------------------------------------------------
// Public API: target dimensions
// -----------------------------------------------------------------------------

int rt_core_target_width(void)
{
    return FRAME_W;
}

int rt_core_target_height(void)
{
    return FRAME_H;
}

// -----------------------------------------------------------------------------
// Public API: RT pinning
// -----------------------------------------------------------------------------

int rt_core_pin_thread(int core_id)
{
    if (core_id < 0) {
        return 0;
    }

#ifdef _WIN32
    DWORD_PTR mask    = (DWORD_PTR)1 << core_id;
    HANDLE    hThread = GetCurrentThread();
    DWORD_PTR prev    = SetThreadAffinityMask(hThread, mask);
    if (prev == 0) {
        std::fprintf(stderr,
            "[rt_core] SetThreadAffinityMask failed for core %d\n",
            core_id);
        return 0;
    }
    return 1;
#else
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    pthread_t thread = pthread_self();
    int s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (s != 0) {
        std::fprintf(stderr,
            "[rt_core] pthread_setaffinity_np failed for core %d (err=%d)\n",
            core_id, s);
        return 0;
    }
    return 1;
#endif
}

// -----------------------------------------------------------------------------
// Ultra-fast spike + crack compute (example AVX2 thresholding)
// -----------------------------------------------------------------------------

static inline float compute_crack_score_avx2(
    const uint8_t* img,
    int            size)
{
    const int step = 32;
    int i      = 0;
    int count  = 0;

    __m256i threshold = _mm256_set1_epi8(32);

    for (; i + step <= size; i += step)
    {
        __m256i pixels =
            _mm256_loadu_si256(
                (const __m256i*)(img + i));

        __m256i cmp =
            _mm256_cmpgt_epi8(pixels, threshold);

        count += __builtin_popcount(
            _mm256_movemask_epi8(cmp));
    }

    return (float)count / (float)size;
}

// -----------------------------------------------------------------------------
// Core processing — deterministic
// -----------------------------------------------------------------------------

ControlOutput rt_core_process_frame_ptr(
    const uint8_t* bgr,
    int            height,
    int            width)
{
    ControlOutput out{};
    out.frame_id = ++s_frame_id;

    const int pixel_count = height * width * 3;

    float crack =
        compute_crack_score_avx2(
            bgr,
            pixel_count);

    out.crack_score        = crack;
    out.fused_crack_score  =
        crack * (1.0f + s_last_yolo_crack);
    out.sparsity           = 1.0f - crack;

    out.throttle =
        (crack > 0.5f) ? 0.3f : 1.0f;
    out.steer    = 0.0f;

    out.yolo_active =
        (s_last_yolo_time_s > 0.0);
    out.yolo_age_ms = 0.0f;

    out.on_spike_count  = 0;
    out.off_spike_count = 0;

    out.yolo_target_hz = TARGET_HZ;

    return out;
}

// -----------------------------------------------------------------------------
// YOLO Publish Bridge
// -----------------------------------------------------------------------------

void rt_core_yolo_publish(
    double timestamp_s,
    float  front_risk,
    float  left_risk,
    float  right_risk,
    float  crack_risk,
    float  min_distance_m,
    float  max_confidence,
    int    num_detections,
    int    priority_detections,
    int    num_filtered_out)
{
    (void)front_risk;
    (void)left_risk;
    (void)right_risk;
    (void)min_distance_m;
    (void)max_confidence;
    (void)num_detections;
    (void)priority_detections;
    (void)num_filtered_out;

    s_last_yolo_crack   = crack_risk;
    s_last_yolo_time_s  = timestamp_s;
}