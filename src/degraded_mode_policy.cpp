/**
 * degraded_mode_policy.cpp - 3-Level Graceful Degradation Implementation
 *
 * Progressive fallback strategy with deterministic transitions:
 * OK → WARNING → CRITICAL → EMERGENCY
 *
 * Author: AuraSense Failsafe Team
 * Date: 2026-02-23
 */

#include "degraded_mode_policy.hpp"
#include <chrono>
#include <cstdio>

using Clock = std::chrono::steady_clock;

// =============================================================================
// Constructor
// =============================================================================

DegradedModePolicy::DegradedModePolicy()
    : current_mode_(OperatingMode::FULL_PERFORMANCE)
    , last_status_(FailsafeStatus::OK)
    , transition_count_(0)
    , mode_start_time_ms_(0)
{
    // LEVEL 1: Full Performance Configuration
    full_config_.rt_core_enabled = true;
    full_config_.rt_core_target_fps = 60.0f;
    full_config_.inference_enabled = true;
    full_config_.inference_max_skip_frames = 10;
    full_config_.yolo_enabled = true;
    full_config_.yolo_target_hz = 5.0f;
    full_config_.uplink_enabled = true;
    full_config_.uplink_throttle_ms = 50;
    full_config_.visualization_enabled = true;
    full_config_.viz_decimation_factor = 1;

    // LEVEL 2: Reduced Performance Configuration (WARNING state)
    reduced_config_.rt_core_enabled = true;
    reduced_config_.rt_core_target_fps = 30.0f;  // Half frame rate
    reduced_config_.inference_enabled = true;
    reduced_config_.inference_max_skip_frames = 20;  // More aggressive skipping
    reduced_config_.yolo_enabled = true;
    reduced_config_.yolo_target_hz = 2.0f;  // Reduced YOLO frequency
    reduced_config_.uplink_enabled = true;
    reduced_config_.uplink_throttle_ms = 100;  // Less frequent uplink
    reduced_config_.visualization_enabled = false;  // Disable visualization
    reduced_config_.viz_decimation_factor = 0;

    // LEVEL 3: Minimal Safe Mode Configuration (CRITICAL state)
    minimal_config_.rt_core_enabled = true;
    minimal_config_.rt_core_target_fps = 15.0f;  // Minimal frame rate
    minimal_config_.inference_enabled = false;  // Disable inference
    minimal_config_.inference_max_skip_frames = 0;
    minimal_config_.yolo_enabled = false;  // Disable YOLO
    minimal_config_.yolo_target_hz = 0.0f;
    minimal_config_.uplink_enabled = true;  // Keep uplink for telemetry
    minimal_config_.uplink_throttle_ms = 200;
    minimal_config_.visualization_enabled = false;  // Disable visualization
    minimal_config_.viz_decimation_factor = 0;

    mode_start_time_ms_.store(get_current_time_ms(), std::memory_order_relaxed);
}

// =============================================================================
// Update Policy Based on Failsafe Status
// =============================================================================

void DegradedModePolicy::update(FailsafeStatus status) {
    last_status_.store(status, std::memory_order_relaxed);

    OperatingMode current = current_mode_.load(std::memory_order_relaxed);
    OperatingMode target = current;

    // Determine target mode based on failsafe status
    switch (status) {
        case FailsafeStatus::OK:
            target = OperatingMode::FULL_PERFORMANCE;
            break;

        case FailsafeStatus::WARNING:
            target = OperatingMode::REDUCED_PERFORMANCE;
            break;

        case FailsafeStatus::CRITICAL:
            target = OperatingMode::MINIMAL_SAFE_MODE;
            break;

        default:
            target = OperatingMode::EMERGENCY_SHUTDOWN;
            break;
    }

    // Transition if mode changed
    if (target != current) {
        transition_to(target);
    }
}

// =============================================================================
// Mode Transition Logic
// =============================================================================

void DegradedModePolicy::transition_to(OperatingMode new_mode) {
    OperatingMode old_mode = current_mode_.exchange(new_mode, std::memory_order_relaxed);

    if (old_mode != new_mode) {
        transition_count_.fetch_add(1, std::memory_order_relaxed);
        mode_start_time_ms_.store(get_current_time_ms(), std::memory_order_relaxed);

        // Log transition
        printf("[DegradedMode] Transition: %s -> %s\n",
               operating_mode_to_string(old_mode),
               operating_mode_to_string(new_mode));
    }
}

// =============================================================================
// Get Current Mode
// =============================================================================

OperatingMode DegradedModePolicy::get_mode() const {
    return current_mode_.load(std::memory_order_relaxed);
}

// =============================================================================
// Get Lane Configuration
// =============================================================================

LaneConfig DegradedModePolicy::get_lane_config() const {
    OperatingMode mode = current_mode_.load(std::memory_order_relaxed);

    switch (mode) {
        case OperatingMode::FULL_PERFORMANCE:
            return full_config_;

        case OperatingMode::REDUCED_PERFORMANCE:
            return reduced_config_;

        case OperatingMode::MINIMAL_SAFE_MODE:
        case OperatingMode::EMERGENCY_SHUTDOWN:
            return minimal_config_;

        default:
            return minimal_config_;  // Safe default
    }
}

// =============================================================================
// Emergency Landing Check
// =============================================================================

bool DegradedModePolicy::should_emergency_land() const {
    OperatingMode mode = current_mode_.load(std::memory_order_relaxed);
    return (mode == OperatingMode::EMERGENCY_SHUTDOWN);
}

// =============================================================================
// Get Mode Description
// =============================================================================

const char* DegradedModePolicy::get_mode_description() const {
    OperatingMode mode = current_mode_.load(std::memory_order_relaxed);

    switch (mode) {
        case OperatingMode::FULL_PERFORMANCE:
            return "Full Performance - All systems operational";

        case OperatingMode::REDUCED_PERFORMANCE:
            return "Reduced Performance - Non-critical features disabled";

        case OperatingMode::MINIMAL_SAFE_MODE:
            return "Minimal Safe Mode - Essential control only";

        case OperatingMode::EMERGENCY_SHUTDOWN:
            return "EMERGENCY SHUTDOWN - Immediate landing required";

        default:
            return "UNKNOWN MODE";
    }
}

// =============================================================================
// Reset to Full Performance
// =============================================================================

void DegradedModePolicy::reset() {
    transition_to(OperatingMode::FULL_PERFORMANCE);
    last_status_.store(FailsafeStatus::OK, std::memory_order_relaxed);
}

// =============================================================================
// Force Mode Override
// =============================================================================

void DegradedModePolicy::force_mode(OperatingMode mode) {
    transition_to(mode);
}

// =============================================================================
// Get Statistics
// =============================================================================

uint64_t DegradedModePolicy::get_mode_transition_count() const {
    return transition_count_.load(std::memory_order_relaxed);
}

double DegradedModePolicy::get_time_in_current_mode_ms() const {
    int64_t start = mode_start_time_ms_.load(std::memory_order_relaxed);
    int64_t now = get_current_time_ms();
    return static_cast<double>(now - start);
}

// =============================================================================
// Helper: Get Current Time
// =============================================================================

int64_t DegradedModePolicy::get_current_time_ms() const {
    auto now = Clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

// =============================================================================
// Helper: Mode to String
// =============================================================================

const char* operating_mode_to_string(OperatingMode mode) {
    switch (mode) {
        case OperatingMode::FULL_PERFORMANCE:    return "FULL_PERFORMANCE";
        case OperatingMode::REDUCED_PERFORMANCE: return "REDUCED_PERFORMANCE";
        case OperatingMode::MINIMAL_SAFE_MODE:   return "MINIMAL_SAFE_MODE";
        case OperatingMode::EMERGENCY_SHUTDOWN:  return "EMERGENCY_SHUTDOWN";
        default:                                  return "UNKNOWN";
    }
}
