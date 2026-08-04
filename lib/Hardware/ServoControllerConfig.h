#pragma once

#include <cstdint>

struct ServoControllerConfig
{
    uint8_t pin;
    uint8_t pwmChannel;
    uint16_t minimumPulseMicroseconds;
    uint16_t maximumPulseMicroseconds;
    float minimumAngleDegrees;
    float maximumAngleDegrees;
    float centerAngleDegrees;
};
