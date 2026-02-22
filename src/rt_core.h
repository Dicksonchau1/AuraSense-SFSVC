#pragma once

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// Core Output — stack only, POD, no heap
// ----------------------------------------------------------------------------
typedef struct
{
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

    float encode_time_ms;

    int   on_spike_count;
    int   off_spike_count;

    float yolo_target_hz;

} ControlOutput;

// ----------------------------------------------------------------------------
// Core API
// ----------------------------------------------------------------------------

// Target input resolution for rt_core
int   rt_core_target_width(void);
int   rt_core_target_height(void);

// Pin current thread to a CPU core (0-based index).
// Returns non-zero on success, 0 on failure.
int   rt_core_pin_thread(int core_id);

ControlOutput rt_core_process_frame_ptr(
    const uint8_t* bgr,
    int            height,
    int            width);

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

#ifdef __cplusplus
}
#endif