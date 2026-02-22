// hardware_profiler.hpp
#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>
#include <mutex>
#include <cstdio>
#include <cstring>

// =============================================================
// Hardware Statistics Snapshot
// =============================================================

struct HardwareStats {
    double cpu_usage_percent     = 0.0;
    double memory_usage_percent  = 0.0;
    double memory_used_mb        = 0.0;
    double temperature_c         = 0.0;   // 0.0 if not available
    bool   has_temperature       = false;
};

// =============================================================
// HardwareProfiler
//
// Thread Model:
//   - start()/stop() controlled externally
//   - Worker thread updates stats periodically
//   - get_stats() may be called concurrently
//   - Mutex protects latest_stats_
//   - Not used in RT path
// =============================================================

class HardwareProfiler {
public:

    explicit HardwareProfiler(double interval_sec = 1.0)
        : interval_ms_(static_cast<int>(interval_sec * 1000.0)),
          running_(false)
    {}

    // ---------------------------------------------------------
    // Start background sampling thread
    // ---------------------------------------------------------
    void start() {
        bool expected = false;
        if (!running_.compare_exchange_strong(expected, true))
            return; // already running

        worker_ = std::thread(&HardwareProfiler::loop, this);
    }

    // ---------------------------------------------------------
    // Stop background thread
    // ---------------------------------------------------------
    void stop() {
        if (!running_.exchange(false))
            return;

        if (worker_.joinable())
            worker_.join();
    }

    // ---------------------------------------------------------
    // Thread-safe snapshot retrieval
    // ---------------------------------------------------------
    HardwareStats get_stats() const {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        return latest_stats_;
    }

    ~HardwareProfiler() {
        stop();
    }

private:

    int interval_ms_;
    std::atomic<bool> running_;
    std::thread worker_;

    mutable std::mutex stats_mutex_;
    HardwareStats latest_stats_;

    // =========================================================
    // Worker Loop
    // =========================================================
    void loop() {

        uint64_t prev_idle  = 0;
        uint64_t prev_total = 0;

        read_cpu_times(prev_idle, prev_total);

        while (running_.load(std::memory_order_relaxed)) {

            HardwareStats s;

            s.cpu_usage_percent =
                read_cpu_usage(prev_idle, prev_total);

            read_memory(s);
            read_temperature(s);

            {
                std::lock_guard<std::mutex> lock(stats_mutex_);
                latest_stats_ = s;
            }

            std::this_thread::sleep_for(
                std::chrono::milliseconds(interval_ms_));
        }
    }

    // =========================================================
    // CPU Usage (Linux /proc/stat)
    // =========================================================

    static bool read_cpu_times(uint64_t& idle,
                               uint64_t& total)
    {
        FILE* fp = std::fopen("/proc/stat", "r");
        if (!fp) return false;

        char cpu[5] = {0};
        uint64_t user=0, nice=0, system=0,
                 idle_t=0, iowait=0,
                 irq=0, softirq=0, steal=0;

        int n = std::fscanf(fp,
                            "%4s %lu %lu %lu %lu %lu %lu %lu %lu",
                            cpu, &user, &nice, &system,
                            &idle_t, &iowait,
                            &irq, &softirq, &steal);

        std::fclose(fp);

        if (n < 5) return false;

        idle  = idle_t + iowait;
        total = user + nice + system +
                idle_t + iowait + irq +
                softirq + steal;

        return true;
    }

    static double read_cpu_usage(uint64_t& prev_idle,
                                 uint64_t& prev_total)
    {
        uint64_t idle = 0;
        uint64_t total = 0;

        if (!read_cpu_times(idle, total))
            return 0.0;

        uint64_t idle_delta  = idle  - prev_idle;
        uint64_t total_delta = total - prev_total;

        prev_idle  = idle;
        prev_total = total;

        if (total_delta == 0)
            return 0.0;

        return 100.0 *
               (1.0 - (double)idle_delta /
                      (double)total_delta);
    }

    // =========================================================
    // Memory Usage (Linux /proc/meminfo)
    // =========================================================

    static void read_memory(HardwareStats& s)
    {
        FILE* fp = std::fopen("/proc/meminfo", "r");
        if (!fp) return;

        long mem_total_kb     = 0;
        long mem_available_kb = 0;

        char key[64];
        long value;
        char unit[32];

        while (std::fscanf(fp,
                           "%63s %ld %31s\n",
                           key, &value, unit) == 3)
        {
            if (std::strcmp(key, "MemTotal:") == 0)
                mem_total_kb = value;
            else if (std::strcmp(key, "MemAvailable:") == 0)
                mem_available_kb = value;

            if (mem_total_kb > 0 && mem_available_kb > 0)
                break;
        }

        std::fclose(fp);

        if (mem_total_kb > 0) {
            long used_kb = mem_total_kb - mem_available_kb;
            s.memory_used_mb =
                used_kb / 1024.0;

            s.memory_usage_percent =
                100.0 *
                (double)used_kb /
                (double)mem_total_kb;
        }
    }

    // =========================================================
    // Temperature (Linux common paths)
    // =========================================================

    static void read_temperature(HardwareStats& s)
    {
        const char* paths[] = {
            "/sys/class/thermal/thermal_zone0/temp",
            "/sys/class/hwmon/hwmon0/temp1_input"
        };

        for (const char* p : paths) {

            FILE* fp = std::fopen(p, "r");
            if (!fp) continue;

            long milli_c = 0;
            if (std::fscanf(fp, "%ld", &milli_c) == 1) {
                std::fclose(fp);
                s.temperature_c =
                    milli_c / 1000.0;
                s.has_temperature = true;
                return;
            }

            std::fclose(fp);
        }

        s.has_temperature = false;
    }
};