#pragma once

#include <cstdint>

struct LedAnimationConfig
{
    uint32_t updateIntervalMs;
    uint32_t slowBlinkIntervalMs;
    uint32_t fastBlinkIntervalMs;
    uint32_t pulseDurationMs;
    float maximumBrightness;
    float minimumPulseBrightness;
};
