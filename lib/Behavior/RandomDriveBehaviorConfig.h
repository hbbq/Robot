#pragma once

#include <cstdint>

struct RandomDriveBehaviorConfig
{
    uint16_t obstacleThresholdMillimeters;
    float backupDistanceMeters;
    float minimumAvoidanceTurnDegrees;
    float maximumAvoidanceTurnDegrees;
    uint32_t sensorLossTimeoutMs;
};
