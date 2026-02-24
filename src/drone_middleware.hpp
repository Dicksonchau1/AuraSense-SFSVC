#pragma once

/**
 * drone_middleware.hpp - Complete Drone Inspection Middleware System
 *
 * Integrates:
 * - Multi-lane vision processing (RT core, inference, YOLO, uplink, viz)
 * - Failsafe monitoring with signal health tracking
 * - 3-level graceful degradation policy
 * - Emergency protocols and recovery mechanisms
 * - Health monitoring and telemetry
 *
 * Author: AuraSense Failsafe Team
 * Date: 2026-02-23
 */

#include "failsafe.hpp"
#include "degraded_mode_policy.hpp"
#include <atomic>
#include <cstdint>
#include <string>
#include <vector>

// =============================================================================
// System Health Status
// =============================================================================

struct SystemHealth {
    // Overall health
    FailsafeStatus overall_status;
    OperatingMode operating_mode;
    bool emergency_land_active;

    // Signal health
    FailsafeStatus camera_health;
    FailsafeStatus imu_health;
    FailsafeStatus gps_health;
    FailsafeStatus battery_health;
    FailsafeStatus motor_health;
    FailsafeStatus network_health;

    // Performance metrics
    float rt_core_fps;
    float rt_core_latency_ms;
    float yolo_hz;
    float uplink_hz;

    // Resource usage
    float cpu_usage_percent;
    float memory_usage_mb;
    float temperature_celsius;

    // Degradation statistics
    uint64_t mode_transition_count;
    double time_in_current_mode_ms;

    // Inspection statistics
    uint64_t total_frames_processed;
    uint64_t total_cracks_detected;
    float inspection_coverage_percent;
};

// =============================================================================
// Emergency Action
// =============================================================================

enum class EmergencyAction {
    NONE,
    LAND_IMMEDIATELY,
    RETURN_TO_HOME,
    HOVER_IN_PLACE,
    CONTROLLED_DESCENT
};

// =============================================================================
// Drone Middleware
// =============================================================================

class DroneMiddleware {
public:
    /**
     * Initialize middleware with failsafe configuration
     *
     * @param configs Array of failsafe signal configurations
     * @param count Number of signals to monitor
     */
    DroneMiddleware(const FailsafeSignalConfig* configs, size_t count);

    // =============================================================================
    // Signal Updates (from sensors/components)
    // =============================================================================

    void update_camera_signal(float value);
    void update_imu_signal(float value);
    void update_gps_signal(float value);
    void update_battery_signal(float value);
    void update_motor_signal(float value);
    void update_network_signal(float value);

    // Generic update by signal name
    void update_signal(const char* name, float value);

    // =============================================================================
    // Processing Updates (from lanes)
    // =============================================================================

    void update_rt_core_metrics(float fps, float latency_ms);
    void update_yolo_metrics(float hz);
    void update_uplink_metrics(float hz);
    void update_resource_metrics(float cpu_percent, float mem_mb, float temp_celsius);

    // =============================================================================
    // Main Update Cycle
    // =============================================================================

    /**
     * Evaluate all failsafe signals and update operating mode
     * Call this periodically (e.g., 10 Hz)
     */
    void evaluate_and_update();

    // =============================================================================
    // Status Queries
    // =============================================================================

    SystemHealth get_system_health() const;
    OperatingMode get_operating_mode() const;
    LaneConfig get_lane_config() const;
    bool is_safe_to_operate() const;
    EmergencyAction get_emergency_action() const;

    // =============================================================================
    // Emergency Control
    // =============================================================================

    void trigger_emergency_landing();
    void cancel_emergency_landing();
    bool is_emergency_landing_active() const;

    // =============================================================================
    // Inspection Tracking
    // =============================================================================

    void report_crack_detected(float severity);
    void update_inspection_coverage(float percent);
    uint64_t get_total_cracks_detected() const;

    // =============================================================================
    // Recovery
    // =============================================================================

    void attempt_recovery();
    void reset_all();

    // =============================================================================
    // Diagnostics
    // =============================================================================

    void print_status() const;
    void print_detailed_diagnostics() const;
    std::string get_telemetry_json() const;

private:
    // Components
    FailsafeMonitor failsafe_;
    DegradedModePolicy policy_;

    // Signal indices
    size_t camera_idx_;
    size_t imu_idx_;
    size_t gps_idx_;
    size_t battery_idx_;
    size_t motor_idx_;
    size_t network_idx_;

    // Performance metrics (atomic for thread-safety)
    std::atomic<float> rt_core_fps_;
    std::atomic<float> rt_core_latency_ms_;
    std::atomic<float> yolo_hz_;
    std::atomic<float> uplink_hz_;
    std::atomic<float> cpu_usage_;
    std::atomic<float> memory_usage_;
    std::atomic<float> temperature_;

    // Emergency state
    std::atomic<bool> emergency_land_active_;
    std::atomic<EmergencyAction> emergency_action_;

    // Inspection tracking
    std::atomic<uint64_t> total_cracks_;
    std::atomic<float> inspection_coverage_;

    // Helper functions
    size_t find_signal_index(const char* name) const;
    EmergencyAction determine_emergency_action() const;
};

// =============================================================================
// Helper Functions
// =============================================================================

const char* emergency_action_to_string(EmergencyAction action);
