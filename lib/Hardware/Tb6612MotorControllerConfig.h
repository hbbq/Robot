#pragma once

#include <cstdint>

struct Tb6612MotorControllerConfig
{
    uint8_t in1Pin;
    uint8_t in2Pin;
    uint8_t pwmPin;

    bool inverted = false;

    uint32_t pwmFrequency = 20000;
    uint8_t pwmResolutionBits = 8;

    float minimumSpeed = 0.0f;
};