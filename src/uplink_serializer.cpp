+   1 // =============================================================================
+   2 // uplink_serializer.cpp — snprintf-based JSON serialization
+   3 // No nlohmann/json dependency. All serialization uses fixed-size stack buffers.
+   4 // =============================================================================
+   5 
+   6 #include "uplink_serializer.h"
+   7 #include <cstdio>
+   8 #include <cstring>
+   9 
+  10 static const char* failsafe_str(FailsafeStatus s) {
+  11     switch (s) {
+  12         case FailsafeStatus::OK:       return "OK";
+  13         case FailsafeStatus::WARNING:  return "WARNING";
+  14         case FailsafeStatus::CRITICAL: return "CRITICAL";
+  15         default:                       return "UNKNOWN";
+  16     }
+  17 }
+  18 
+  19 // Escape a string for JSON output (handles quotes and backslashes)
+  20 static void json_escape(const char* src, char* dst, size_t dst_size) {
+  21     size_t j = 0;
+  22     for (size_t i = 0; src[i] && j + 2 < dst_size; i++) {
+  23         char c = src[i];
+  24         if (c == '"' || c == '\\') {
+  25             dst[j++] = '\\';
+  26         }
+  27         dst[j++] = c;
+  28     }
+  29     dst[j] = '\0';
+  30 }
+  31 
+  32 std::string UplinkSerializer::to_json(const UplinkPayload& p) {
+  33     char buf[2048];
+  34     char action_esc[128];
+  35     char severity_esc[128];
+  36 
+  37     json_escape(p.action.c_str(), action_esc, sizeof(action_esc));
+  38     json_escape(p.crack_severity.c_str(), severity_esc, sizeof(severity_esc));
+  39 
+  40     int n = std::snprintf(buf, sizeof(buf),
+  41         "{"
+  42         "\"frame_id\":%d,"
+  43         "\"timestamp\":%.6f,"
+  44         "\"throttle\":%.4f,"
+  45         "\"steer\":%.4f,"
+  46         "\"crack_score\":%.6f,"
+  47         "\"sparsity\":%.4f,"
+  48         "\"action\":\"%s\","
+  49         "\"control_latency_ms\":%.3f,"
+  50         "\"crack_severity\":\"%s\","
+  51         "\"crack_width_mm\":%.3f,"
+  52         "\"failsafe\":\"%s\","
+  53         "\"sig_conf\":%.4f,"
+  54         "\"yolo_count\":%llu,"
+  55         "\"latency_violations\":%llu"
+  56         "}",
+  57         p.frame_id,
+  58         p.timestamp,
+  59         p.throttle,
+  60         p.steer,
+  61         p.crack_score,
+  62         p.sparsity,
+  63         action_esc,
+  64         p.control_latency_ms,
+  65         severity_esc,
+  66         p.crack_width_mm,
+  67         failsafe_str(p.failsafe_status),
+  68         p.sig_conf,
+  69         (unsigned long long)p.yolo_count,
+  70         (unsigned long long)p.latency_violations);
+  71 
+  72     if (n < 0 || n >= static_cast<int>(sizeof(buf))) {
+  73         return "{}";
+  74     }
+  75     return std::string(buf, static_cast<size_t>(n));
+  76 }
+  77 
+  78 std::string UplinkSerializer::metrics_to_json(const Metrics& m) {
+  79     char buf[2048];
+  80 
+  81     int n = std::snprintf(buf, sizeof(buf),
+  82         "{"
+  83         "\"frame_id\":%d,"
+  84         "\"fps\":%.1f,"
+  85         "\"hz\":%.1f,"
+  86         "\"last_crack\":%.6f,"
+  87         "\"sig_conf\":%.4f,"
+  88         "\"yolo_count\":%llu,"
+  89         "\"uplink_count\":%llu,"
+  90         "\"latency_violations\":%llu,"
+  91         "\"p50\":%.3f,"
+  92         "\"p95\":%.3f,"
+  93         "\"p99\":%.3f,"
+  94         "\"yolo_hz\":%.1f,"
+  95         "\"yolo_age_ms\":%.1f,"
+  96         "\"yolo_conf_thr\":%.3f,"
+  97         "\"window_crack_ratio\":%.4f,"
+  98         "\"global_crack_ratio\":%.4f,"
+  99         "\"crack_alert_thr\":%.3f,"
+ 100         "\"spike_bitrate_mbps\":%.3f,"
+ 101         "\"avg_yolo_conf\":%.4f,"
+ 102         "\"avg_crack_score\":%.4f,"
+ 103         "\"avg_agreement\":%.4f,"
+ 104         "\"px_to_mm\":%.4f,"
+ 105         "\"crack_width_mm\":%.3f,"
+ 106         "\"crack_length_mm\":%.3f"
+ 107         "}",
+ 108         m.frame_id,
+ 109         m.fps,
+ 110         m.fps,
+ 111         m.last_crack,
+ 112         m.sig_conf,
+ 113         (unsigned long long)m.yolo_count,
+ 114         (unsigned long long)m.uplink_count,
+ 115         (unsigned long long)m.latency_violations,
+ 116         m.latency_p50_ms,
+ 117         m.latency_p95_ms,
+ 118         m.latency_p99_ms,
+ 119         m.yolo_hz,
+ 120         m.yolo_age_ms,
+ 121         m.yolo_conf_thr,
+ 122         m.window_crack_ratio,
+ 123         m.global_crack_ratio,
+ 124         m.crack_alert_thr,
+ 125         m.spike_bitrate_mbps,
+ 126         m.avg_yolo_conf,
+ 127         m.avg_crack_score,
+ 128         m.avg_agreement,
+ 129         m.px_to_mm_scale,
+ 130         m.vis_crack_width_mm,
+ 131         m.vis_crack_length_mm);
+ 132 
+ 133     if (n < 0 || n >= static_cast<int>(sizeof(buf))) {
+ 134         return "{}";
+ 135     }
+ 136     return std::string(buf, static_cast<size_t>(n));
+ 137 }
+ 138 
+ 139 std::string UplinkSerializer::decision_to_json(const ControlDecision& d) {
+ 140     char buf[2048];
+ 141     char action_esc[128];
+ 142     char severity_esc[128];
+ 143 
+ 144     json_escape(d.action.c_str(), action_esc, sizeof(action_esc));
+ 145     json_escape(d.crack_severity.c_str(), severity_esc, sizeof(severity_esc));
+ 146 
+ 147     int n = std::snprintf(buf, sizeof(buf),
+ 148         "{"
+ 149         "\"frame_id\":%d,"
+ 150         "\"timestamp\":%.6f,"
+ 151         "\"throttle\":%.4f,"
+ 152         "\"steer\":%.4f,"
+ 153         "\"action\":\"%s\","
+ 154         "\"crack_score\":%.6f,"
+ 155         "\"sparsity\":%.4f,"
+ 156         "\"confidence\":%.4f,"
+ 157         "\"semantic_age_ms\":%.1f,"
+ 158         "\"is_null_cycle\":%s,"
+ 159         "\"inference_suppressed\":%s,"
+ 160         "\"event_only_mode\":%s,"
+ 161         "\"yolo_active\":%s,"
+ 162         "\"yolo_age_ms\":%.1f,"
+ 163         "\"encode_time_ms\":%.3f,"
+ 164         "\"control_latency_ms\":%.3f,"
+ 165         "\"crack_severity\":\"%s\","
+ 166         "\"crack_width_mm\":%.3f,"
+ 167         "\"crack_length_mm\":%.3f,"
+ 168         "\"crack_confidence_pct\":%.1f"
+ 169         "}",
+ 170         d.frame_id,
+ 171         d.timestamp,
+ 172         d.throttle,
+ 173         d.steer,
+ 174         action_esc,
+ 175         d.crack_score,
+ 176         d.sparsity,
+ 177         d.confidence,
+ 178         d.semantic_age_ms,
+ 179         d.is_null_cycle ? "true" : "false",
+ 180         d.inference_suppressed ? "true" : "false",
+ 181         d.event_only_mode ? "true" : "false",
+ 182         d.yolo_active ? "true" : "false",
+ 183         d.yolo_age_ms,
+ 184         d.encode_time_ms,
+ 185         d.control_latency_ms,
+ 186         severity_esc,
+ 187         d.crack_width_mm,
+ 188         d.crack_length_mm,
+ 189         d.crack_confidence_percent);
+ 190 
+ 191     if (n < 0 || n >= static_cast<int>(sizeof(buf))) {
+ 192         return "{}";
+ 193     }
+ 194     return std::string(buf, static_cast<size_t>(n));
+ 195 }