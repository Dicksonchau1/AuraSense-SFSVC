/**
 * failsafe.cpp - Complete Failsafe Monitor Implementation
 *
 * Features:
 * - Lock-free atomic signal monitoring
 * - Timeout detection (stale signals)
 * - Value range validation
 * - Multi-signal worst-case aggregation
 * - Thread-safe state evaluation
 *
 * Author: AuraSense Failsafe Team
 * Date: 2026-02-22
 */

#include "failsafe.hpp"
#include <cstdio>
#include <cstring>

// =============================================================================
// FailsafeMonitor Implementation
// =============================================================================

FailsafeMonitor::FailsafeMonitor()
    : configs_(nullptr)
    , count_(0)
{
    state_.store(FailsafeStatus::OK, std::memory_order_relaxed);
}

FailsafeMonitor::FailsafeMonitor(
    const FailsafeSignalConfig* configs,
    size_t count
)
    : configs_(configs)
    , count_(count)
{
    last_update_.reset(new std::atomic<int64_t>[count_]);
    last_value_.reset(new std::atomic<float>[count_]);

    for (size_t i = 0; i < count_; ++i) {
        last_update_[i].store(0, std::memory_order_relaxed);
        last_value_[i].store(0.0f, std::memory_order_relaxed);
    }

    state_.store(FailsafeStatus::OK, std::memory_order_relaxed);
}

void FailsafeMonitor::update(size_t idx, float value) {
    if (idx >= count_) return;

    auto now = Clock::now().time_since_epoch();
    int64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();

    last_update_[idx].store(ms, std::memory_order_relaxed);
    last_value_[idx].store(value, std::memory_order_relaxed);
}

void FailsafeMonitor::reset(size_t idx) {
    if (idx >= count_) return;

    last_update_[idx].store(0, std::memory_order_relaxed);
    last_value_[idx].store(0.0f, std::memory_order_relaxed);
}

void FailsafeMonitor::reset_all() {
    for (size_t i = 0; i < count_; ++i) {
        reset(i);
    }
    state_.store(FailsafeStatus::OK, std::memory_order_relaxed);
}

FailsafeStatus FailsafeMonitor::evaluate() {
    auto now = Clock::now().time_since_epoch();
    int64_t now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();

    FailsafeStatus worst = FailsafeStatus::OK;

    for (size_t i = 0; i < count_; ++i) {
        const auto& cfg = configs_[i];
        int64_t last = last_update_[i].load(std::memory_order_relaxed);
        float val = last_value_[i].load(std::memory_order_relaxed);

        // Check timeout (staleness)
        if (last > 0) {
            float age_ms = static_cast<float>(now_ms - last);

            if (age_ms > cfg.crit_timeout_ms) {
                worst = FailsafeStatus::CRITICAL;
            } else if (age_ms > cfg.warn_timeout_ms && worst == FailsafeStatus::OK) {
                worst = FailsafeStatus::WARNING;
            }
        } else {
            // Signal never received - critical if timeout is defined
            if (cfg.crit_timeout_ms > 0.0f) {
                worst = FailsafeStatus::CRITICAL;
            }
        }

        // Check value range
        if (cfg.enable_value_checks) {
            if (val < cfg.crit_low || val > cfg.crit_high) {
                worst = FailsafeStatus::CRITICAL;
            } else if ((val < cfg.warn_low || val > cfg.warn_high) &&
                       worst == FailsafeStatus::OK) {
                worst = FailsafeStatus::WARNING;
            }
        }

        // Early exit on critical
        if (worst == FailsafeStatus::CRITICAL) {
            break;
        }
    }

    state_.store(worst, std::memory_order_relaxed);
    return worst;
}

FailsafeStatus FailsafeMonitor::evaluate_signal(size_t idx) const {
    if (idx >= count_) return FailsafeStatus::CRITICAL;

    auto now = Clock::now().time_since_epoch();
    int64_t now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();

    const auto& cfg = configs_[idx];
    int64_t last = last_update_[idx].load(std::memory_order_relaxed);
    float val = last_value_[idx].load(std::memory_order_relaxed);

    FailsafeStatus status = FailsafeStatus::OK;

    // Timeout check
    if (last > 0) {
        float age_ms = static_cast<float>(now_ms - last);

        if (age_ms > cfg.crit_timeout_ms) {
            status = FailsafeStatus::CRITICAL;
        } else if (age_ms > cfg.warn_timeout_ms) {
            status = FailsafeStatus::WARNING;
        }
    } else if (cfg.crit_timeout_ms > 0.0f) {
        status = FailsafeStatus::CRITICAL;
    }

    // Value range check
    if (cfg.enable_value_checks) {
        if (val < cfg.crit_low || val > cfg.crit_high) {
            status = FailsafeStatus::CRITICAL;
        } else if (val < cfg.warn_low || val > cfg.warn_high) {
            if (status == FailsafeStatus::OK) {
                status = FailsafeStatus::WARNING;
            }
        }
    }

    return status;
}

FailsafeStatus FailsafeMonitor::state() const {
    return state_.load(std::memory_order_relaxed);
}

float FailsafeMonitor::get_value(size_t idx) const {
    if (idx >= count_) return 0.0f;
    return last_value_[idx].load(std::memory_order_relaxed);
}

int64_t FailsafeMonitor::get_age_ms(size_t idx) const {
    if (idx >= count_) return -1;

    int64_t last = last_update_[idx].load(std::memory_order_relaxed);
    if (last == 0) return -1;

    auto now = Clock::now().time_since_epoch();
    int64_t now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();

    return now_ms - last;
}

const char* FailsafeMonitor::get_signal_name(size_t idx) const {
    if (idx >= count_) return nullptr;
    return configs_[idx].name;
}

size_t FailsafeMonitor::get_signal_count() const {
    return count_;
}

void FailsafeMonitor::print_status() const {
    printf("=============================================================\n");
    printf("FAILSAFE STATUS: %s\n", failsafe_status_to_string(state()));
    printf("=============================================================\n");

    for (size_t i = 0; i < count_; ++i) {
        const auto& cfg = configs_[i];
        float val = get_value(i);
        int64_t age_ms = get_age_ms(i);
        FailsafeStatus sig_status = evaluate_signal(i);

        const char* status_str = failsafe_status_to_string(sig_status);

        printf("[%zu] %-20s | Status: %-8s | Value: %7.2f | Age: ",
               i, cfg.name, status_str, val);

        if (age_ms >= 0) {
            printf("%5ld ms\n", age_ms);
        } else {
            printf("  NEVER\n");
        }
    }

    printf("=============================================================\n");
}

const char* failsafe_status_to_string(FailsafeStatus status) {
    switch (status) {
        case FailsafeStatus::OK:       return "OK";
        case FailsafeStatus::WARNING:  return "WARNING";
        case FailsafeStatus::CRITICAL: return "CRITICAL";
        default:                        return "UNKNOWN";
    }
}
