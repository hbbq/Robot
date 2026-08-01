#pragma once

#include <cstdint>

struct LedControllerConfig
{
    uint8_t pin;
    bool activeHigh = true;
};