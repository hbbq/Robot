#pragma once

#include <cstdint>

struct CalibrationBehaviorConfig
{
    float maximumDistanceMeters;
    float maximumTurnDegrees;
    uint16_t obstacleThresholdMillimeters;
    uint32_t sensorLossTimeoutMs;
    uint32_t sessionLeaseMs;
};
