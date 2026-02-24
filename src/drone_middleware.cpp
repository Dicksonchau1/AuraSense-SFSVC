/**
 * drone_middleware.cpp - Complete Drone Inspection Middleware Implementation
 *
 * Full integration of failsafe monitoring, graceful degradation,
 * and emergency protocols for drone inspection system.
 *
 * Author: AuraSense Failsafe Team
 * Date: 2026-02-23
 */

#include "drone_middleware.hpp"
#include <cstdio>
#include <cstring>
#include <sstream>
#include <iomanip>

// =============================================================================
// Constructor
// =============================================================================

DroneMiddleware::DroneMiddleware(
    const FailsafeSignalConfig* configs,
    size_t count
)
    : failsafe_(configs, count)
    , policy_()
    , rt_core_fps_(0.0f)
    , rt_core_latency_ms_(0.0f)
    , yolo_hz_(0.0f)
    , uplink_hz_(0.0f)
    , cpu_usage_(0.0f)
    , memory_usage_(0.0f)
    , temperature_(0.0f)
    , emergency_land_active_(false)
    , emergency_action_(EmergencyAction::NONE)
    , total_cracks_(0)
    , inspection_coverage_(0.0f)
{
    // Find signal indices
    camera_idx_ = find_signal_index("camera");
    imu_idx_ = find_signal_index("imu");
    gps_idx_ = find_signal_index("gps");
    battery_idx_ = find_signal_index("battery");
    motor_idx_ = find_signal_index("motor");
    network_idx_ = find_signal_index("network");
}

// =============================================================================
// Signal Updates
// =============================================================================

void DroneMiddleware::update_camera_signal(float value) {
    if (camera_idx_ < failsafe_.get_signal_count()) {
        failsafe_.update(camera_idx_, value);
    }
}

void DroneMiddleware::update_imu_signal(float value) {
    if (imu_idx_ < failsafe_.get_signal_count()) {
        failsafe_.update(imu_idx_, value);
    }
}

void DroneMiddleware::update_gps_signal(float value) {
    if (gps_idx_ < failsafe_.get_signal_count()) {
        failsafe_.update(gps_idx_, value);
    }
}

void DroneMiddleware::update_battery_signal(float value) {
    if (battery_idx_ < failsafe_.get_signal_count()) {
        failsafe_.update(battery_idx_, value);
    }
}

void DroneMiddleware::update_motor_signal(float value) {
    if (motor_idx_ < failsafe_.get_signal_count()) {
        failsafe_.update(motor_idx_, value);
    }
}

void DroneMiddleware::update_network_signal(float value) {
    if (network_idx_ < failsafe_.get_signal_count()) {
        failsafe_.update(network_idx_, value);
    }
}

void DroneMiddleware::update_signal(const char* name, float value) {
    size_t idx = find_signal_index(name);
    if (idx < failsafe_.get_signal_count()) {
        failsafe_.update(idx, value);
    }
}

// =============================================================================
// Processing Updates
// =============================================================================

void DroneMiddleware::update_rt_core_metrics(float fps, float latency_ms) {
    rt_core_fps_.store(fps, std::memory_order_relaxed);
    rt_core_latency_ms_.store(latency_ms, std::memory_order_relaxed);
}

void DroneMiddleware::update_yolo_metrics(float hz) {
    yolo_hz_.store(hz, std::memory_order_relaxed);
}

void DroneMiddleware::update_uplink_metrics(float hz) {
    uplink_hz_.store(hz, std::memory_order_relaxed);
}

void DroneMiddleware::update_resource_metrics(float cpu_percent, float mem_mb, float temp_celsius) {
    cpu_usage_.store(cpu_percent, std::memory_order_relaxed);
    memory_usage_.store(mem_mb, std::memory_order_relaxed);
    temperature_.store(temp_celsius, std::memory_order_relaxed);
}

// =============================================================================
// Main Update Cycle
// =============================================================================

void DroneMiddleware::evaluate_and_update() {
    // Evaluate all failsafe signals
    FailsafeStatus overall = failsafe_.evaluate();

    // Update degradation policy
    policy_.update(overall);

    // Determine emergency action if critical
    if (overall == FailsafeStatus::CRITICAL) {
        EmergencyAction action = determine_emergency_action();
        emergency_action_.store(action, std::memory_order_relaxed);

        // Auto-trigger emergency landing for critical failures
        if (action == EmergencyAction::LAND_IMMEDIATELY && !emergency_land_active_.load(std::memory_order_relaxed)) {
            trigger_emergency_landing();
        }
    }
}

// =============================================================================
// Status Queries
// =============================================================================

SystemHealth DroneMiddleware::get_system_health() const {
    SystemHealth health;

    // Overall status
    health.overall_status = failsafe_.state();
    health.operating_mode = policy_.get_mode();
    health.emergency_land_active = emergency_land_active_.load(std::memory_order_relaxed);

    // Signal health
    health.camera_health = (camera_idx_ < failsafe_.get_signal_count()) ?
                          failsafe_.evaluate_signal(camera_idx_) : FailsafeStatus::CRITICAL;
    health.imu_health = (imu_idx_ < failsafe_.get_signal_count()) ?
                       failsafe_.evaluate_signal(imu_idx_) : FailsafeStatus::CRITICAL;
    health.gps_health = (gps_idx_ < failsafe_.get_signal_count()) ?
                       failsafe_.evaluate_signal(gps_idx_) : FailsafeStatus::CRITICAL;
    health.battery_health = (battery_idx_ < failsafe_.get_signal_count()) ?
                           failsafe_.evaluate_signal(battery_idx_) : FailsafeStatus::CRITICAL;
    health.motor_health = (motor_idx_ < failsafe_.get_signal_count()) ?
                         failsafe_.evaluate_signal(motor_idx_) : FailsafeStatus::CRITICAL;
    health.network_health = (network_idx_ < failsafe_.get_signal_count()) ?
                           failsafe_.evaluate_signal(network_idx_) : FailsafeStatus::CRITICAL;

    // Performance metrics
    health.rt_core_fps = rt_core_fps_.load(std::memory_order_relaxed);
    health.rt_core_latency_ms = rt_core_latency_ms_.load(std::memory_order_relaxed);
    health.yolo_hz = yolo_hz_.load(std::memory_order_relaxed);
    health.uplink_hz = uplink_hz_.load(std::memory_order_relaxed);

    // Resource usage
    health.cpu_usage_percent = cpu_usage_.load(std::memory_order_relaxed);
    health.memory_usage_mb = memory_usage_.load(std::memory_order_relaxed);
    health.temperature_celsius = temperature_.load(std::memory_order_relaxed);

    // Degradation statistics
    health.mode_transition_count = policy_.get_mode_transition_count();
    health.time_in_current_mode_ms = policy_.get_time_in_current_mode_ms();

    // Inspection statistics
    health.total_frames_processed = failsafe_.get_signal_count() > 0 ?
                                   failsafe_.get_age_ms(0) : 0;  // Approximation
    health.total_cracks_detected = total_cracks_.load(std::memory_order_relaxed);
    health.inspection_coverage_percent = inspection_coverage_.load(std::memory_order_relaxed);

    return health;
}

OperatingMode DroneMiddleware::get_operating_mode() const {
    return policy_.get_mode();
}

LaneConfig DroneMiddleware::get_lane_config() const {
    return policy_.get_lane_config();
}

bool DroneMiddleware::is_safe_to_operate() const {
    FailsafeStatus status = failsafe_.state();
    return (status == FailsafeStatus::OK || status == FailsafeStatus::WARNING);
}

EmergencyAction DroneMiddleware::get_emergency_action() const {
    return emergency_action_.load(std::memory_order_relaxed);
}

// =============================================================================
// Emergency Control
// =============================================================================

void DroneMiddleware::trigger_emergency_landing() {
    emergency_land_active_.store(true, std::memory_order_relaxed);
    emergency_action_.store(EmergencyAction::LAND_IMMEDIATELY, std::memory_order_relaxed);

    printf("\n");
    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printf("!! EMERGENCY LANDING TRIGGERED                                 !!\n");
    printf("!! All non-essential systems will be disabled                  !!\n");
    printf("!! Initiating controlled descent                               !!\n");
    printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printf("\n");

    // Force minimal safe mode
    policy_.force_mode(OperatingMode::EMERGENCY_SHUTDOWN);
}

void DroneMiddleware::cancel_emergency_landing() {
    emergency_land_active_.store(false, std::memory_order_relaxed);
    emergency_action_.store(EmergencyAction::NONE, std::memory_order_relaxed);

    printf("[DroneMiddleware] Emergency landing cancelled - resuming normal operation\n");
}

bool DroneMiddleware::is_emergency_landing_active() const {
    return emergency_land_active_.load(std::memory_order_relaxed);
}

// =============================================================================
// Inspection Tracking
// =============================================================================

void DroneMiddleware::report_crack_detected(float severity) {
    if (severity > 0.5f) {  // Only count significant cracks
        total_cracks_.fetch_add(1, std::memory_order_relaxed);
    }
}

void DroneMiddleware::update_inspection_coverage(float percent) {
    inspection_coverage_.store(percent, std::memory_order_relaxed);
}

uint64_t DroneMiddleware::get_total_cracks_detected() const {
    return total_cracks_.load(std::memory_order_relaxed);
}

// =============================================================================
// Recovery
// =============================================================================

void DroneMiddleware::attempt_recovery() {
    printf("[DroneMiddleware] Attempting system recovery...\n");

    // Reset failsafe monitors
    failsafe_.reset_all();

    // Reset degradation policy
    policy_.reset();

    // Clear emergency state
    emergency_land_active_.store(false, std::memory_order_relaxed);
    emergency_action_.store(EmergencyAction::NONE, std::memory_order_relaxed);

    printf("[DroneMiddleware] Recovery complete - system reset to full performance\n");
}

void DroneMiddleware::reset_all() {
    failsafe_.reset_all();
    policy_.reset();
    emergency_land_active_.store(false, std::memory_order_relaxed);
    emergency_action_.store(EmergencyAction::NONE, std::memory_order_relaxed);
    total_cracks_.store(0, std::memory_order_relaxed);
    inspection_coverage_.store(0.0f, std::memory_order_relaxed);
}

// =============================================================================
// Diagnostics
// =============================================================================

void DroneMiddleware::print_status() const {
    SystemHealth health = get_system_health();

    printf("\n");
    printf("=============================================================\n");
    printf("DRONE MIDDLEWARE STATUS\n");
    printf("=============================================================\n");
    printf("Overall Status:      %s\n", failsafe_status_to_string(health.overall_status));
    printf("Operating Mode:      %s\n", operating_mode_to_string(health.operating_mode));
    printf("Emergency Landing:   %s\n", health.emergency_land_active ? "ACTIVE" : "Inactive");
    printf("\n");

    printf("--- Signal Health ---\n");
    printf("Camera:              %s\n", failsafe_status_to_string(health.camera_health));
    printf("IMU:                 %s\n", failsafe_status_to_string(health.imu_health));
    printf("GPS:                 %s\n", failsafe_status_to_string(health.gps_health));
    printf("Battery:             %s\n", failsafe_status_to_string(health.battery_health));
    printf("Motor:               %s\n", failsafe_status_to_string(health.motor_health));
    printf("Network:             %s\n", failsafe_status_to_string(health.network_health));
    printf("\n");

    printf("--- Performance ---\n");
    printf("RT Core FPS:         %.1f\n", health.rt_core_fps);
    printf("RT Core Latency:     %.2f ms\n", health.rt_core_latency_ms);
    printf("YOLO Frequency:      %.1f Hz\n", health.yolo_hz);
    printf("Uplink Frequency:    %.1f Hz\n", health.uplink_hz);
    printf("\n");

    printf("--- Resources ---\n");
    printf("CPU Usage:           %.1f %%\n", health.cpu_usage_percent);
    printf("Memory Usage:        %.1f MB\n", health.memory_usage_mb);
    printf("Temperature:         %.1f °C\n", health.temperature_celsius);
    printf("\n");

    printf("--- Inspection ---\n");
    printf("Cracks Detected:     %lu\n", health.total_cracks_detected);
    printf("Coverage:            %.1f %%\n", health.inspection_coverage_percent);
    printf("\n");

    printf("=============================================================\n");
}

void DroneMiddleware::print_detailed_diagnostics() const {
    printf("\n");
    printf("=============================================================\n");
    printf("DETAILED SYSTEM DIAGNOSTICS\n");
    printf("=============================================================\n");

    // Print overall status
    print_status();

    // Print failsafe monitor details
    printf("\n--- Failsafe Monitor Details ---\n");
    failsafe_.print_status();

    // Print degradation policy details
    printf("\n--- Degradation Policy ---\n");
    printf("Current Mode:        %s\n", operating_mode_to_string(policy_.get_mode()));
    printf("Mode Description:    %s\n", policy_.get_mode_description());
    printf("Time in Mode:        %.2f s\n", policy_.get_time_in_current_mode_ms() / 1000.0);
    printf("Transition Count:    %lu\n", policy_.get_mode_transition_count());

    // Print lane configuration
    printf("\n--- Lane Configuration ---\n");
    LaneConfig config = policy_.get_lane_config();
    printf("RT Core:             %s (%.1f FPS)\n",
           config.rt_core_enabled ? "ENABLED" : "DISABLED",
           config.rt_core_target_fps);
    printf("Inference:           %s (skip %d frames)\n",
           config.inference_enabled ? "ENABLED" : "DISABLED",
           config.inference_max_skip_frames);
    printf("YOLO:                %s (%.1f Hz)\n",
           config.yolo_enabled ? "ENABLED" : "DISABLED",
           config.yolo_target_hz);
    printf("Uplink:              %s (%d ms throttle)\n",
           config.uplink_enabled ? "ENABLED" : "DISABLED",
           config.uplink_throttle_ms);
    printf("Visualization:       %s (decimation %d)\n",
           config.visualization_enabled ? "ENABLED" : "DISABLED",
           config.viz_decimation_factor);

    printf("\n=============================================================\n");
}

std::string DroneMiddleware::get_telemetry_json() const {
    SystemHealth health = get_system_health();
    LaneConfig config = policy_.get_lane_config();

    std::ostringstream json;
    json << std::fixed << std::setprecision(2);

    json << "{\n";
    json << "  \"timestamp\": " << (policy_.get_time_in_current_mode_ms() / 1000.0) << ",\n";
    json << "  \"status\": {\n";
    json << "    \"overall\": \"" << failsafe_status_to_string(health.overall_status) << "\",\n";
    json << "    \"mode\": \"" << operating_mode_to_string(health.operating_mode) << "\",\n";
    json << "    \"emergency_land\": " << (health.emergency_land_active ? "true" : "false") << "\n";
    json << "  },\n";
    json << "  \"signals\": {\n";
    json << "    \"camera\": \"" << failsafe_status_to_string(health.camera_health) << "\",\n";
    json << "    \"imu\": \"" << failsafe_status_to_string(health.imu_health) << "\",\n";
    json << "    \"gps\": \"" << failsafe_status_to_string(health.gps_health) << "\",\n";
    json << "    \"battery\": \"" << failsafe_status_to_string(health.battery_health) << "\",\n";
    json << "    \"motor\": \"" << failsafe_status_to_string(health.motor_health) << "\",\n";
    json << "    \"network\": \"" << failsafe_status_to_string(health.network_health) << "\"\n";
    json << "  },\n";
    json << "  \"performance\": {\n";
    json << "    \"rt_core_fps\": " << health.rt_core_fps << ",\n";
    json << "    \"rt_core_latency_ms\": " << health.rt_core_latency_ms << ",\n";
    json << "    \"yolo_hz\": " << health.yolo_hz << ",\n";
    json << "    \"uplink_hz\": " << health.uplink_hz << "\n";
    json << "  },\n";
    json << "  \"resources\": {\n";
    json << "    \"cpu_percent\": " << health.cpu_usage_percent << ",\n";
    json << "    \"memory_mb\": " << health.memory_usage_mb << ",\n";
    json << "    \"temperature_c\": " << health.temperature_celsius << "\n";
    json << "  },\n";
    json << "  \"inspection\": {\n";
    json << "    \"cracks_detected\": " << health.total_cracks_detected << ",\n";
    json << "    \"coverage_percent\": " << health.inspection_coverage_percent << "\n";
    json << "  },\n";
    json << "  \"lane_config\": {\n";
    json << "    \"rt_core_enabled\": " << (config.rt_core_enabled ? "true" : "false") << ",\n";
    json << "    \"inference_enabled\": " << (config.inference_enabled ? "true" : "false") << ",\n";
    json << "    \"yolo_enabled\": " << (config.yolo_enabled ? "true" : "false") << ",\n";
    json << "    \"uplink_enabled\": " << (config.uplink_enabled ? "true" : "false") << ",\n";
    json << "    \"viz_enabled\": " << (config.visualization_enabled ? "true" : "false") << "\n";
    json << "  }\n";
    json << "}\n";

    return json.str();
}

// =============================================================================
// Helper: Find Signal Index
// =============================================================================

size_t DroneMiddleware::find_signal_index(const char* name) const {
    for (size_t i = 0; i < failsafe_.get_signal_count(); ++i) {
        const char* sig_name = failsafe_.get_signal_name(i);
        if (sig_name && std::strcmp(sig_name, name) == 0) {
            return i;
        }
    }
    return static_cast<size_t>(-1);  // Not found
}

// =============================================================================
// Helper: Determine Emergency Action
// =============================================================================

EmergencyAction DroneMiddleware::determine_emergency_action() const {
    // Check individual signal health to determine best emergency action
    FailsafeStatus battery = (battery_idx_ < failsafe_.get_signal_count()) ?
                            failsafe_.evaluate_signal(battery_idx_) : FailsafeStatus::OK;
    FailsafeStatus motor = (motor_idx_ < failsafe_.get_signal_count()) ?
                          failsafe_.evaluate_signal(motor_idx_) : FailsafeStatus::OK;
    FailsafeStatus gps = (gps_idx_ < failsafe_.get_signal_count()) ?
                        failsafe_.evaluate_signal(gps_idx_) : FailsafeStatus::OK;

    // Critical battery or motor failure → land immediately
    if (battery == FailsafeStatus::CRITICAL || motor == FailsafeStatus::CRITICAL) {
        return EmergencyAction::LAND_IMMEDIATELY;
    }

    // GPS failure → hover in place
    if (gps == FailsafeStatus::CRITICAL) {
        return EmergencyAction::HOVER_IN_PLACE;
    }

    // Other critical failures → controlled descent
    return EmergencyAction::CONTROLLED_DESCENT;
}

// =============================================================================
// Helper: Emergency Action to String
// =============================================================================

const char* emergency_action_to_string(EmergencyAction action) {
    switch (action) {
        case EmergencyAction::NONE:              return "NONE";
        case EmergencyAction::LAND_IMMEDIATELY:  return "LAND_IMMEDIATELY";
        case EmergencyAction::RETURN_TO_HOME:    return "RETURN_TO_HOME";
        case EmergencyAction::HOVER_IN_PLACE:    return "HOVER_IN_PLACE";
        case EmergencyAction::CONTROLLED_DESCENT:return "CONTROLLED_DESCENT";
        default:                                  return "UNKNOWN";
    }
}
