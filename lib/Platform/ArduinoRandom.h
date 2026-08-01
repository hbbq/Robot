#pragma once

#include <cstdint>
#include <random>

#include <IRandom.h>

class ArduinoRandom : public IRandom
{
public:
    int32_t next(
        int32_t min,
        int32_t max) override;

    float nextFloat(
        float min,
        float max) override;
};