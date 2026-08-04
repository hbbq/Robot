#pragma once

#include <cstdint>

struct LedHardwareConfig
{
    uint8_t pin;
    uint8_t pwmChannel;
    bool activeHigh;
    uint32_t pwmFrequencyHz;
    uint8_t pwmResolutionBits;
};
