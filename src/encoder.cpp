#include "encoder.hpp"
#include <cmath>
#include <algorithm>

// BT.601 grayscale conversion
static inline float gray_bt601(const uint8_t* px)
{
    return (0.114f * px[0] +
            0.587f * px[1] +
            0.299f * px[2]) / 255.0f;
}

FeatureEncoder::FeatureEncoder(int h, int w)
    : height_(h), width_(w)
{}

SpikeEncoding FeatureEncoder::encode_frame(const uint8_t* bgr)
{
    SpikeEncoding enc{};
    enc.height = height_;
    enc.width  = width_;

    const int N = height_ * width_;

    float sum = 0.0f;
    float sum_sq = 0.0f;

    for (int i = 0; i < N; ++i)
    {
        const uint8_t* px = bgr + i * 3;
        float g = gray_bt601(px);
        sum += g;
        sum_sq += g * g;
    }

    if (N > 0)
    {
        float mean = sum / N;
        float var  = std::max(0.0f, sum_sq / N - mean * mean);

        enc.avg_luminance = mean;
        enc.lum_contrast  = std::sqrt(var);
    }

    enc.sparsity   = 0.0f;   // rt_core handles sparsity
    enc.yolo_count = 0;

    return enc;
}

SpikeEncoding cpp_encoder_encode_frame(const uint8_t* bgr, int h, int w)
{
    static FeatureEncoder* encoder = nullptr;
    static int last_h = 0, last_w = 0;

    if (!encoder || h != last_h || w != last_w) {
        delete encoder;
        encoder = new FeatureEncoder(h, w);
        last_h = h;
        last_w = w;
    }

    return encoder->encode_frame(bgr);
}