#pragma once

#include <cstdint>

struct RandomDriveBehaviorConfig
{
    uint8_t forwardChancePercent;
    uint32_t minimumWaitMs;
    uint32_t maximumWaitMs;
    float minimumForwardDistanceMeters;
    float maximumForwardDistanceMeters;
    float minimumTurnDegrees;
    float maximumTurnDegrees;

    uint16_t obstacleThresholdMillimeters;
    float backupDistanceMeters;
    float minimumAvoidanceTurnDegrees;
    float maximumAvoidanceTurnDegrees;
    uint32_t sensorLossTimeoutMs;
};
