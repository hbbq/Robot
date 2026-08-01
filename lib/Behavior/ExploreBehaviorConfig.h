#pragma once

#include <cstdint>

struct ExploreBehaviorConfig
{
    uint16_t obstacleThresholdMillimeters;
    float backupDistanceMeters;
    float minimumAvoidanceTurnDegrees;
    float maximumAvoidanceTurnDegrees;
    uint32_t sensorLossTimeoutMs;

    float minimumForwardDistanceMeters;
    float maximumForwardDistanceMeters;
    uint8_t courseCorrectionChancePercent;
    float minimumCourseCorrectionDegrees;
    float maximumCourseCorrectionDegrees;
    uint32_t pauseBetweenMovesMs;
};
