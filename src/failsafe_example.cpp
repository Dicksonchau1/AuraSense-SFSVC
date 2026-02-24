// =============================================================================
// failsafe_example.cpp — AuraSense Failsafe Monitor Demo
// =============================================================================

#include <cstdio>
#include <thread>
#include <chrono>

#include "failsafe.hpp"

int main() {
    static const FailsafeSignalConfig configs[] = {
        { "imu",      500.0f, 1000.0f,  -2.0f,  2.0f, -5.0f,  5.0f, true  },
        { "gps",     1000.0f, 2000.0f,   0.0f, 90.0f,  0.0f, 90.0f, true  },
        { "battery",  200.0f,  500.0f,  20.0f, 85.0f, 10.0f, 90.0f, true  },
    };

    FailsafeMonitor monitor(configs, 3);

    // Simulate signal updates
    monitor.update(0, 0.1f);   // imu OK
    monitor.update(1, 45.0f);  // gps OK
    monitor.update(2, 75.0f);  // battery OK

    FailsafeStatus status = monitor.evaluate();
    monitor.print_status();

    printf("\nOverall status: %s\n", failsafe_status_to_string(status));
    return (status == FailsafeStatus::OK) ? 0 : 1;
}
