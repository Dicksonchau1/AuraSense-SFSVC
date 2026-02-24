+  1 #pragma once
+  2 
+  3 #include "types.h"
+  4 #include <string>
+  5 
+  6 // =============================================================================
+  7 // UplinkSerializer — snprintf-based JSON serialization
+  8 // No nlohmann/json dependency. Fixed-buffer serialization.
+  9 // =============================================================================
+ 10 class UplinkSerializer {
+ 11 public:
+ 12     // Serialize UplinkPayload to JSON string.
+ 13     // Uses snprintf into a stack-allocated buffer.
+ 14     static std::string to_json(const UplinkPayload& payload);
+ 15 
+ 16     // Serialize Metrics to JSON string.
+ 17     static std::string metrics_to_json(const Metrics& metrics);
+ 18 
+ 19     // Serialize ControlDecision to JSON string.
+ 20     static std::string decision_to_json(const ControlDecision& decision);
+ 21 };