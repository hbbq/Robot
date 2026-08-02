#pragma once

#include <cstdint>

struct BrightnessControllerConfig
{
    uint8_t minimumPercent;
    uint8_t maximumPercent;
    uint32_t fadeDurationMs;
};
