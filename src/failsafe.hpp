#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>

#include"types.h"

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

class FailsafeMonitor {
public:
    using Clock = std::chrono::steady_clock;

    FailsafeMonitor(const FailsafeSignalConfig* configs,
                    size_t count)
        : configs_(configs),
          count_(count)
    {
        last_update_.reset(new std::atomic<int64_t>[count_]);
        last_value_.reset(new std::atomic<float>[count_]);

        for (size_t i = 0; i < count_; ++i) {
            last_update_[i].store(0, std::memory_order_relaxed);
            last_value_[i].store(0.0f, std::memory_order_relaxed);
        }
    }

    void update(size_t idx, float value) {
        if (idx >= count_) return;

        auto now = Clock::now().time_since_epoch();
        int64_t ms =
            std::chrono::duration_cast<
                std::chrono::milliseconds>(now).count();

        last_update_[idx].store(ms, std::memory_order_relaxed);
        last_value_[idx].store(value, std::memory_order_relaxed);
    }

    FailsafeStatus evaluate() {
        auto now = Clock::now().time_since_epoch();
        int64_t now_ms =
            std::chrono::duration_cast<
                std::chrono::milliseconds>(now).count();

        FailsafeStatus worst = FailsafeStatus::OK;

        for (size_t i = 0; i < count_; ++i) {

            const auto& cfg = configs_[i];
            int64_t last = last_update_[i].load(std::memory_order_relaxed);
            float   val  = last_value_[i].load(std::memory_order_relaxed);

            if (last > 0) {
                float age =
                    static_cast<float>(now_ms - last);

                if (age > cfg.crit_timeout_ms)
                    worst = FailsafeStatus::CRITICAL;
                else if (age > cfg.warn_timeout_ms &&
                         worst == FailsafeStatus::OK)
                    worst = FailsafeStatus::WARNING;
            }

            if (cfg.enable_value_checks) {
                if (val < cfg.crit_low ||
                    val > cfg.crit_high)
                    worst = FailsafeStatus::CRITICAL;
                else if ((val < cfg.warn_low ||
                          val > cfg.warn_high) &&
                         worst == FailsafeStatus::OK)
                    worst = FailsafeStatus::WARNING;
            }
        }

        state_.store(worst, std::memory_order_relaxed);
        return worst;
    }

    FailsafeStatus state() const {
        return state_.load(std::memory_order_relaxed);
    }

private:
    const FailsafeSignalConfig* configs_;
    size_t                      count_;

    std::unique_ptr<std::atomic<int64_t>[]> last_update_;
    std::unique_ptr<std::atomic<float>[]>   last_value_;

    std::atomic<FailsafeStatus> state_{FailsafeStatus::OK};
};
