// =============================================================================
// uplink_serializer.cpp — snprintf-based JSON serialization
// No nlohmann/json dependency. All serialization uses fixed-size stack buffers.
// =============================================================================

#include "uplink_serializer.h"
#include <cstdio>
#include <cstring>

static const char* failsafe_str(FailsafeStatus s) {
    switch (s) {
        case FailsafeStatus::OK:       return "OK";
        case FailsafeStatus::WARNING:  return "WARNING";
        case FailsafeStatus::CRITICAL: return "CRITICAL";
        default:                       return "UNKNOWN";
    }
}

// Escape a string for JSON output (handles quotes and backslashes)
static void json_escape(const char* src, char* dst, size_t dst_size) {
    size_t j = 0;
    for (size_t i = 0; src[i] && j + 2 < dst_size; i++) {
        char c = src[i];
        if (c == '"' || c == '\\') {
            dst[j++] = '\\';
        }
        dst[j++] = c;
    }
    dst[j] = '\0';
}

std::string UplinkSerializer::to_json(const UplinkPayload& p) {
    char buf[2048];
    char action_esc[128];
    char severity_esc[128];

    json_escape(p.action.c_str(), action_esc, sizeof(action_esc));
    json_escape(p.crack_severity.c_str(), severity_esc, sizeof(severity_esc));

    int n = std::snprintf(buf, sizeof(buf),
        "{"
        "\"frame_id\":%d,"
        "\"timestamp\":%.6f,"
        "\"throttle\":%.4f,"
        "\"steer\":%.4f,"
        "\"crack_score\":%.6f,"
        "\"sparsity\":%.4f,"
        "\"action\":\"%s\","
        "\"control_latency_ms\":%.3f,"
        "\"crack_severity\":\"%s\","
        "\"crack_width_mm\":%.3f,"
        "\"failsafe\":\"%s\","
        "\"sig_conf\":%.4f,"
        "\"yolo_count\":%llu,"
        "\"latency_violations\":%llu"
        "}",
        p.frame_id,
        p.timestamp,
        p.throttle,
        p.steer,
        p.crack_score,
        p.sparsity,
        action_esc,
        p.control_latency_ms,
        severity_esc,
        p.crack_width_mm,
        failsafe_str(p.failsafe_status),
        p.sig_conf,
        (unsigned long long)p.yolo_count,
        (unsigned long long)p.latency_violations);

    if (n < 0 || n >= static_cast<int>(sizeof(buf))) {
        return "{}";
    }
    return std::string(buf, static_cast<size_t>(n));
}

std::string UplinkSerializer::metrics_to_json(const Metrics& m) {
    char buf[2048];

    int n = std::snprintf(buf, sizeof(buf),
        "{"
        "\"frame_id\":%d,"
        "\"fps\":%.1f,"
        "\"hz\":%.1f,"
        "\"last_crack\":%.6f,"
        "\"sig_conf\":%.4f,"
        "\"yolo_count\":%llu,"
        "\"uplink_count\":%llu,"
        "\"latency_violations\":%llu,"
        "\"p50\":%.3f,"
        "\"p95\":%.3f,"
        "\"p99\":%.3f,"
        "\"yolo_hz\":%.1f,"
        "\"yolo_age_ms\":%.1f,"
        "\"yolo_conf_thr\":%.3f,"
        "\"window_crack_ratio\":%.4f,"
        "\"global_crack_ratio\":%.4f,"
        "\"crack_alert_thr\":%.3f,"
        "\"spike_bitrate_mbps\":%.3f,"
        "\"avg_yolo_conf\":%.4f,"
        "\"avg_crack_score\":%.4f,"
        "\"avg_agreement\":%.4f,"
        "\"px_to_mm\":%.4f,"
        "\"crack_width_mm\":%.3f,"
        "\"crack_length_mm\":%.3f"
        "}",
        m.frame_id,
        m.fps,
        m.fps,
        m.last_crack,
        m.sig_conf,
        (unsigned long long)m.yolo_count,
        (unsigned long long)m.uplink_count,
        (unsigned long long)m.latency_violations,
        m.latency_p50_ms,
        m.latency_p95_ms,
        m.latency_p99_ms,
        m.yolo_hz,
        m.yolo_age_ms,
        m.yolo_conf_thr,
        m.window_crack_ratio,
        m.global_crack_ratio,
        m.crack_alert_thr,
        m.spike_bitrate_mbps,
        m.avg_yolo_conf,
        m.avg_crack_score,
        m.avg_agreement,
        m.px_to_mm_scale,
        m.vis_crack_width_mm,
        m.vis_crack_length_mm);

    if (n < 0 || n >= static_cast<int>(sizeof(buf))) {
        return "{}";
    }
    return std::string(buf, static_cast<size_t>(n));
}

std::string UplinkSerializer::decision_to_json(const ControlDecision& d) {
    char buf[2048];
    char action_esc[128];
    char severity_esc[128];

    json_escape(d.action.c_str(), action_esc, sizeof(action_esc));
    json_escape(d.crack_severity.c_str(), severity_esc, sizeof(severity_esc));

    int n = std::snprintf(buf, sizeof(buf),
        "{"
        "\"frame_id\":%d,"
        "\"timestamp\":%.6f,"
        "\"throttle\":%.4f,"
        "\"steer\":%.4f,"
        "\"action\":\"%s\","
        "\"crack_score\":%.6f,"
        "\"sparsity\":%.4f,"
        "\"confidence\":%.4f,"
        "\"semantic_age_ms\":%.1f,"
        "\"is_null_cycle\":%s,"
        "\"inference_suppressed\":%s,"
        "\"event_only_mode\":%s,"
        "\"yolo_active\":%s,"
        "\"yolo_age_ms\":%.1f,"
        "\"encode_time_ms\":%.3f,"
        "\"control_latency_ms\":%.3f,"
        "\"crack_severity\":\"%s\","
        "\"crack_width_mm\":%.3f,"
        "\"crack_length_mm\":%.3f,"
        "\"crack_confidence_pct\":%.1f"
        "}",
        d.frame_id,
        d.timestamp,
        d.throttle,
        d.steer,
        action_esc,
        d.crack_score,
        d.sparsity,
        d.confidence,
        d.semantic_age_ms,
        d.is_null_cycle ? "true" : "false",
        d.inference_suppressed ? "true" : "false",
        d.event_only_mode ? "true" : "false",
        d.yolo_active ? "true" : "false",
        d.yolo_age_ms,
        d.encode_time_ms,
        d.control_latency_ms,
        severity_esc,
        d.crack_width_mm,
        d.crack_length_mm,
        d.crack_confidence_percent);

    if (n < 0 || n >= static_cast<int>(sizeof(buf))) {
        return "{}";
    }
    return std::string(buf, static_cast<size_t>(n));
}