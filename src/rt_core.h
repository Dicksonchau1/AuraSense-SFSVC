#pragma once

/**
 * RT Core - Enhanced C API with Metrics
 *
 * Version: 2.0-enhanced
 * All enhancements integrated
 */

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Core Output Structure
// =============================================================================

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

    float encode_time_ms;   // Actual measured latency

    int   on_spike_count;
    int   off_spike_count;

    float global_saliency;
    int   roi_count;

} ControlOutput;

// =============================================================================
// Core API
// =============================================================================

int   rt_core_target_width(void);
int   rt_core_target_height(void);

int   rt_core_pin_thread(int core_id);

ControlOutput rt_core_process_frame_ptr(
    const uint8_t* bgr,
    int            height,
    int            width);

// =============================================================================
// YOLO Bonding Layer
// =============================================================================

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
    int    num_filtered_out);

void rt_core_yolo_reset(void);

float rt_core_yolo_get_target_hz(void);
float rt_core_yolo_get_period_ms(void);

// =============================================================================
// Vehicle Speed Input
// =============================================================================

void rt_core_set_vehicle_speed(float speed_mps);

// =============================================================================
// Unified Clock
// =============================================================================

double rt_core_get_time_s(void);

// =============================================================================
// Metrics API (DAY 4 & DAY 6)
// =============================================================================

// Fast O(n) percentiles using nth_element
float    rt_core_get_p50_latency_ms(void);
float    rt_core_get_p95_latency_ms(void);
float    rt_core_get_p99_latency_ms(void);

uint64_t rt_core_get_total_frames(void);
uint64_t rt_core_get_total_spike_events(void);

#ifdef __cplusplus
}
#endif