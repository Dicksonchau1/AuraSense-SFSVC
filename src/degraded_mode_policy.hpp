#pragma once

/**
 * degraded_mode_policy.hpp - 3-Level Graceful Degradation Policy
 *
 * Implements progressive fallback strategy for drone inspection system:
 * - Level 1 (OK):       Full functionality, all lanes active
 * - Level 2 (WARNING):  Reduced functionality, lower frame rates, disable non-critical features
 * - Level 3 (CRITICAL): Minimal safe mode, emergency protocols, essential control only
 *
 * Author: AuraSense Failsafe Team
 * Date: 2026-02-23
 */

#include "failsafe.hpp"
#include <atomic>
#include <cstdint>

// =============================================================================
// Operating Modes
// =============================================================================

enum class OperatingMode {
    FULL_PERFORMANCE,    // All lanes active, maximum quality
    REDUCED_PERFORMANCE, // Lower frame rates, disable visualization, reduce YOLO frequency
    MINIMAL_SAFE_MODE,   // Only essential control, emergency landing ready
    EMERGENCY_SHUTDOWN   // Critical failure, immediate landing protocol
};

// =============================================================================
// Lane Configuration
// =============================================================================

struct LaneConfig {
    // Lane 1: RT Core
    bool rt_core_enabled;
    float rt_core_target_fps;

    // Lane 2: Inference Engine
    bool inference_enabled;
    int inference_max_skip_frames;

    // Lane 3: YOLO Detection
    bool yolo_enabled;
    float yolo_target_hz;

    // Lane 4: Uplink
    bool uplink_enabled;
    int uplink_throttle_ms;

    // Lane 5: Visualization
    bool visualization_enabled;
    int viz_decimation_factor;
};

// =============================================================================
// Degradation Policy
// =============================================================================

class DegradedModePolicy {
public:
    DegradedModePolicy();

    // Update policy based on current failsafe status
    void update(FailsafeStatus status);

    // Get current operating mode
    OperatingMode get_mode() const;

    // Get lane configuration for current mode
    LaneConfig get_lane_config() const;

    // Check if emergency landing should be initiated
    bool should_emergency_land() const;

    // Get human-readable status
    const char* get_mode_description() const;

    // Reset to full performance
    void reset();

    // Manual mode override (for testing/debugging)
    void force_mode(OperatingMode mode);

    // Get statistics
    uint64_t get_mode_transition_count() const;
    double get_time_in_current_mode_ms() const;

private:
    std::atomic<OperatingMode> current_mode_;
    std::atomic<FailsafeStatus> last_status_;
    std::atomic<uint64_t> transition_count_;
    std::atomic<int64_t> mode_start_time_ms_;

    LaneConfig full_config_;
    LaneConfig reduced_config_;
    LaneConfig minimal_config_;

    void transition_to(OperatingMode new_mode);
    int64_t get_current_time_ms() const;
};

// =============================================================================
// Helper Functions
// =============================================================================

const char* operating_mode_to_string(OperatingMode mode);
