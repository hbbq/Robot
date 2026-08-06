#pragma once

#include <cstdint>

struct DistanceSensorPanConfig
{
    float centerAngle;
    float leftAngle;
    float rightAngle;
    uint32_t settleTimeMs;
    uint32_t readingTimeoutMs;
    uint32_t sampleFreshnessMs;
};
