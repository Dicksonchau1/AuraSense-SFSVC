#pragma once

#include "types.h"
#include <string>

// =============================================================================
// UplinkSerializer — snprintf-based JSON serialization
// No nlohmann/json dependency. Fixed-buffer serialization.
// =============================================================================
class UplinkSerializer {
public:
    // Serialize UplinkPayload to JSON string.
    // Uses snprintf into a stack-allocated buffer.
    static std::string to_json(const UplinkPayload& payload);

    // Serialize Metrics to JSON string.
    static std::string metrics_to_json(const Metrics& metrics);

    // Serialize ControlDecision to JSON string.
    static std::string decision_to_json(const ControlDecision& decision);
};