#pragma once

#include <cstdint>
#include <vector>

// =============================================================================
// SpikeEncoding
// Must match encoder.cpp expectations
// =============================================================================
struct SpikeEncoding
{
    std::vector<uint8_t> spikes;

    int height = 0;
    int width  = 0;

    float avg_luminance = 0.0f;
    float lum_contrast  = 0.0f;

    float sparsity = 0.0f;
    int   yolo_count = 0;

    SpikeEncoding() = default;
};

// =============================================================================
// FeatureEncoder
// =============================================================================
class FeatureEncoder
{
public:
    FeatureEncoder(int height, int width);

    SpikeEncoding encode_frame(const uint8_t* bgr);

    int height() const noexcept { return height_; }
    int width()  const noexcept { return width_;  }

private:
    int height_;
    int width_;
};

// =============================================================================
// C wrapper
// =============================================================================
SpikeEncoding cpp_encoder_encode_frame(
    const uint8_t* bgr,
    int height,
    int width);