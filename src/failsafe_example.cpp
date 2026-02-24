/**
 * failsafe_example.cpp - Complete Failsafe System Usage Example
 *
 * Demonstrates:
 * - Failsafe monitor configuration
 * - 3-level graceful degradation
 * - Drone middleware integration
 * - Emergency protocols
 * - Health monitoring
 *
 * Author: AuraSense Failsafe Team
 * Date: 2026-02-23
 */

#include "drone_middleware.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>

using namespace std::chrono_literals;

// =============================================================================
// Simulate Sensor Data
// =============================================================================

class SensorSimulator {
public:
    SensorSimulator() : time_(0.0f), failure_mode_(0) {}

    void set_failure_mode(int mode) { failure_mode_ = mode; }

    float get_camera_signal() {
        // Simulate camera frame rate (normal: 60 fps)
        if (failure_mode_ == 1) return 10.0f;  // Camera degraded
        return 60.0f;
    }

    float get_imu_signal() {
        // Simulate IMU update rate (normal: 1000 Hz)
        if (failure_mode_ == 2) return 100.0f;  // IMU degraded
        return 1000.0f;
    }

    float get_gps_signal() {
        // Simulate GPS fix quality (normal: 10 satellites)
        if (failure_mode_ == 3) return 2.0f;  // GPS degraded
        return 10.0f;
    }

    float get_battery_signal() {
        // Simulate battery voltage (normal: 11.1V, critical: <10.0V)
        float voltage = 11.1f - (time_ / 100.0f);  // Slowly draining
        if (failure_mode_ == 4) return 9.5f;  // Critical battery
        return std::max(voltage, 10.0f);
    }

    float get_motor_signal() {
        // Simulate motor RPM (normal: 5000 RPM)
        if (failure_mode_ == 5) return 2000.0f;  // Motor failure
        return 5000.0f + 100.0f * std::sin(time_ * 0.1f);
    }

    float get_network_signal() {
        // Simulate network signal strength (normal: -50 dBm)
        if (failure_mode_ == 6) return -90.0f;  // Network weak
        return -50.0f + 10.0f * std::sin(time_ * 0.05f);
    }

    void update(float dt) { time_ += dt; }

private:
    float time_;
    int failure_mode_;
};

// =============================================================================
// Main Example
// =============================================================================

int main() {
    std::cout << "=============================================================\n";
    std::cout << "AuraSense Drone Inspection Middleware - Failsafe Example\n";
    std::cout << "=============================================================\n\n";

    // -------------------------------------------------------------------------
    // STEP 1: Configure Failsafe Signals
    // -------------------------------------------------------------------------

    FailsafeSignalConfig configs[6];

    // Camera signal (fps)
    std::strcpy(configs[0].name, "camera");
    configs[0].warn_timeout_ms = 100.0f;   // Warning if >100ms stale
    configs[0].crit_timeout_ms = 500.0f;   // Critical if >500ms stale
    configs[0].enable_value_checks = true;
    configs[0].warn_low = 20.0f;           // Warning if <20 fps
    configs[0].warn_high = 1000.0f;
    configs[0].crit_low = 5.0f;            // Critical if <5 fps
    configs[0].crit_high = 1000.0f;

    // IMU signal (Hz)
    std::strcpy(configs[1].name, "imu");
    configs[1].warn_timeout_ms = 50.0f;
    configs[1].crit_timeout_ms = 200.0f;
    configs[1].enable_value_checks = true;
    configs[1].warn_low = 200.0f;
    configs[1].warn_high = 2000.0f;
    configs[1].crit_low = 50.0f;
    configs[1].crit_high = 2000.0f;

    // GPS signal (satellites)
    std::strcpy(configs[2].name, "gps");
    configs[2].warn_timeout_ms = 1000.0f;
    configs[2].crit_timeout_ms = 5000.0f;
    configs[2].enable_value_checks = true;
    configs[2].warn_low = 4.0f;            // Warning if <4 satellites
    configs[2].warn_high = 50.0f;
    configs[2].crit_low = 3.0f;            // Critical if <3 satellites
    configs[2].crit_high = 50.0f;

    // Battery signal (voltage)
    std::strcpy(configs[3].name, "battery");
    configs[3].warn_timeout_ms = 500.0f;
    configs[3].crit_timeout_ms = 2000.0f;
    configs[3].enable_value_checks = true;
    configs[3].warn_low = 10.2f;           // Warning if <10.2V
    configs[3].warn_high = 12.6f;
    configs[3].crit_low = 10.0f;           // Critical if <10.0V
    configs[3].crit_high = 12.6f;

    // Motor signal (RPM)
    std::strcpy(configs[4].name, "motor");
    configs[4].warn_timeout_ms = 100.0f;
    configs[4].crit_timeout_ms = 500.0f;
    configs[4].enable_value_checks = true;
    configs[4].warn_low = 3000.0f;
    configs[4].warn_high = 8000.0f;
    configs[4].crit_low = 1000.0f;
    configs[4].crit_high = 8000.0f;

    // Network signal (dBm)
    std::strcpy(configs[5].name, "network");
    configs[5].warn_timeout_ms = 2000.0f;
    configs[5].crit_timeout_ms = 10000.0f;
    configs[5].enable_value_checks = true;
    configs[5].warn_low = -80.0f;
    configs[5].warn_high = -30.0f;
    configs[5].crit_low = -95.0f;
    configs[5].crit_high = -30.0f;

    // -------------------------------------------------------------------------
    // STEP 2: Initialize Drone Middleware
    // -------------------------------------------------------------------------

    DroneMiddleware middleware(configs, 6);
    SensorSimulator sim;

    std::cout << "Drone middleware initialized with 6 monitored signals\n";
    std::cout << "Starting simulation...\n\n";

    // -------------------------------------------------------------------------
    // STEP 3: Simulation Loop - Normal Operation
    // -------------------------------------------------------------------------

    std::cout << "=== PHASE 1: Normal Operation (5 seconds) ===\n\n";

    for (int i = 0; i < 50; ++i) {
        // Update sensor values
        middleware.update_camera_signal(sim.get_camera_signal());
        middleware.update_imu_signal(sim.get_imu_signal());
        middleware.update_gps_signal(sim.get_gps_signal());
        middleware.update_battery_signal(sim.get_battery_signal());
        middleware.update_motor_signal(sim.get_motor_signal());
        middleware.update_network_signal(sim.get_network_signal());

        // Update performance metrics
        middleware.update_rt_core_metrics(60.0f, 0.15f);
        middleware.update_yolo_metrics(5.0f);
        middleware.update_uplink_metrics(20.0f);
        middleware.update_resource_metrics(45.0f, 128.0f, 55.0f);

        // Evaluate and update
        middleware.evaluate_and_update();

        // Print status every second
        if (i % 10 == 0) {
            middleware.print_status();
        }

        sim.update(0.1f);
        std::this_thread::sleep_for(100ms);
    }

    // -------------------------------------------------------------------------
    // STEP 4: Simulate Warning Condition
    // -------------------------------------------------------------------------

    std::cout << "\n=== PHASE 2: Simulating GPS Degradation (WARNING) ===\n\n";
    sim.set_failure_mode(3);  // GPS degradation

    for (int i = 0; i < 20; ++i) {
        middleware.update_camera_signal(sim.get_camera_signal());
        middleware.update_imu_signal(sim.get_imu_signal());
        middleware.update_gps_signal(sim.get_gps_signal());
        middleware.update_battery_signal(sim.get_battery_signal());
        middleware.update_motor_signal(sim.get_motor_signal());
        middleware.update_network_signal(sim.get_network_signal());

        middleware.update_rt_core_metrics(30.0f, 0.18f);  // Reduced performance
        middleware.update_yolo_metrics(2.0f);
        middleware.update_uplink_metrics(10.0f);

        middleware.evaluate_and_update();

        if (i % 5 == 0) {
            middleware.print_status();
            std::cout << "\nCurrent Mode: " << operating_mode_to_string(middleware.get_operating_mode()) << "\n";
        }

        sim.update(0.1f);
        std::this_thread::sleep_for(100ms);
    }

    // -------------------------------------------------------------------------
    // STEP 5: Simulate Critical Condition
    // -------------------------------------------------------------------------

    std::cout << "\n=== PHASE 3: Simulating Battery Critical (CRITICAL) ===\n\n";
    sim.set_failure_mode(4);  // Critical battery

    for (int i = 0; i < 20; ++i) {
        middleware.update_camera_signal(sim.get_camera_signal());
        middleware.update_imu_signal(sim.get_imu_signal());
        middleware.update_gps_signal(sim.get_gps_signal());
        middleware.update_battery_signal(sim.get_battery_signal());
        middleware.update_motor_signal(sim.get_motor_signal());
        middleware.update_network_signal(sim.get_network_signal());

        middleware.update_rt_core_metrics(15.0f, 0.25f);  // Minimal performance
        middleware.update_yolo_metrics(0.0f);  // YOLO disabled
        middleware.update_uplink_metrics(5.0f);

        middleware.evaluate_and_update();

        if (i % 5 == 0) {
            middleware.print_status();
            std::cout << "\nCurrent Mode: " << operating_mode_to_string(middleware.get_operating_mode()) << "\n";
            std::cout << "Emergency Action: " << emergency_action_to_string(middleware.get_emergency_action()) << "\n";

            if (middleware.is_emergency_landing_active()) {
                std::cout << "⚠️  EMERGENCY LANDING IN PROGRESS ⚠️\n";
            }
        }

        sim.update(0.1f);
        std::this_thread::sleep_for(100ms);
    }

    // -------------------------------------------------------------------------
    // STEP 6: Recovery
    // -------------------------------------------------------------------------

    std::cout << "\n=== PHASE 4: System Recovery ===\n\n";
    sim.set_failure_mode(0);  // Clear failure
    middleware.attempt_recovery();

    for (int i = 0; i < 20; ++i) {
        middleware.update_camera_signal(sim.get_camera_signal());
        middleware.update_imu_signal(sim.get_imu_signal());
        middleware.update_gps_signal(sim.get_gps_signal());
        middleware.update_battery_signal(11.1f);  // Battery recharged
        middleware.update_motor_signal(sim.get_motor_signal());
        middleware.update_network_signal(sim.get_network_signal());

        middleware.update_rt_core_metrics(60.0f, 0.15f);
        middleware.update_yolo_metrics(5.0f);
        middleware.update_uplink_metrics(20.0f);

        middleware.evaluate_and_update();

        if (i % 5 == 0) {
            middleware.print_status();
        }

        sim.update(0.1f);
        std::this_thread::sleep_for(100ms);
    }

    // -------------------------------------------------------------------------
    // STEP 7: Final Diagnostics
    // -------------------------------------------------------------------------

    std::cout << "\n=== Final System Diagnostics ===\n";
    middleware.print_detailed_diagnostics();

    std::cout << "\n=== Telemetry JSON ===\n";
    std::cout << middleware.get_telemetry_json() << "\n";

    std::cout << "\n=============================================================\n";
    std::cout << "Simulation Complete\n";
    std::cout << "=============================================================\n";

    return 0;
}
