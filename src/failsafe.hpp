#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>

#include "types.h"

// Public status enum used by engine
struct FailsafeSignalConfig {
    const char* name;

    float warn_timeout_ms;
    float crit_timeout_ms;

    float warn_low;
    float warn_high;

    float crit_low;
    float crit_high;

    bool  enable_value_checks;
};

const char* failsafe_status_to_string(FailsafeStatus status);

class FailsafeMonitor {
public:
    using Clock = std::chrono::steady_clock;

    // Default constructor: creates a monitor with zero signals (no-op operation)
    FailsafeMonitor();

    FailsafeMonitor(const FailsafeSignalConfig* configs, size_t count);

    void update(size_t idx, float value);
    void reset(size_t idx);
    void reset_all();

    FailsafeStatus evaluate();
    FailsafeStatus evaluate_signal(size_t idx) const;
    FailsafeStatus state() const;

    float   get_value(size_t idx) const;
    int64_t get_age_ms(size_t idx) const;
    const char* get_signal_name(size_t idx) const;
    size_t  get_signal_count() const;

    void print_status() const;

private:
    const FailsafeSignalConfig* configs_ = nullptr;
    size_t                      count_   = 0;

    std::unique_ptr<std::atomic<int64_t>[]> last_update_;
    std::unique_ptr<std::atomic<float>[]>   last_value_;

    std::atomic<FailsafeStatus> state_{FailsafeStatus::OK};
};
