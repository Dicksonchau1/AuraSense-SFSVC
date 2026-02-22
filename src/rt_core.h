#pragma once

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Core Output — stack only, POD, no heap
// =============================================================================
typedef struct
{
    int   frame_id;

    float crack_score;          // Raw crack score from edge detection
    float fused_crack_score;    // After YOLO fusion

    float sparsity;             // 1.0 = no spikes, 0.0 = all spikes

    float throttle;             // 0.0 to 1.0
    float steer;                // -1.0 to 1.0

    int   is_null_cycle;        // First frame (no temporal reference)
    int   inference_suppressed; // Sparsity > 0.95
    int   event_only_mode;      // Sparsity > 0.98

    int   reference_frame_age;  // Frames since reference update

    int   yolo_active;          // YOLO data is valid and recent
    float yolo_age_ms;          // Milliseconds since last YOLO update

    float encode_time_ms;       // Encoding latency (future use)

    int   on_spike_count;       // Number of ON spikes
    int   off_spike_count;      // Number of OFF spikes

    float yolo_target_hz;       // Adaptive YOLO inference rate

    float global_saliency;      // Global saliency score (future use)
    int   roi_count;            // Number of ROI candidates (future use)

} ControlOutput;

// =============================================================================
// Core API
// =============================================================================

// Get target resolution for rt_core processing
int   rt_core_target_width(void);
int   rt_core_target_height(void);

// Pin current thread to a CPU core (0-based index).
// Returns non-zero on success, 0 on failure.
int   rt_core_pin_thread(int core_id);

// Main processing function (HOT PATH - must be deterministic)
// Input: BGR image at target resolution (TARGET_HEIGHT x TARGET_WIDTH)
// Output: ControlOutput struct with all computed metrics
ControlOutput rt_core_process_frame_ptr(
    const uint8_t* bgr,
    int            height,
    int            width);

// =============================================================================
// YOLO Bonding Layer - Lane 3 publishes detection results to rt_core
// =============================================================================

// Publish YOLO detection results to the C++ bonding layer
// timestamp_s: Unified clock timestamp (from rt_core_get_time_s())
// risks: Spatial risk values [0.0, 1.0] for each zone
// detections: Detection counts and statistics
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

// Reset YOLO state (call on engine start/stop)
void rt_core_yolo_reset(void);

// Get adaptive YOLO target Hz (5-20 Hz based on vehicle speed)
float rt_core_yolo_get_target_hz(void);

// Get YOLO period in milliseconds (1000 / target_hz)
float rt_core_yolo_get_period_ms(void);

// =============================================================================
// Vehicle Speed Input (for adaptive YOLO scheduling)
// =============================================================================

// Update vehicle speed (m/s) - used for adaptive YOLO Hz
// Higher speed → higher Hz (up to 20 Hz)
// Lower speed → lower Hz (down to 5 Hz)
void rt_core_set_vehicle_speed(float speed_mps);

// =============================================================================
// Unified Clock (for YOLO timestamp synchronization)
// =============================================================================

// Get current time in seconds (monotonic clock)
// Use this for timestamp_s in rt_core_yolo_publish()
double rt_core_get_time_s(void);

#ifdef __cplusplus
}
#endif