#pragma once

#include <cstdint>

struct Vl53l0xDistanceSensorConfig
{
    uint8_t sdaPin;
    uint8_t sclPin;

    uint32_t i2cFrequencyHz = 400000;
    uint16_t measurementPeriodMs = 50;
    uint16_t measurementTimeoutMs = 25;
    uint32_t measurementTimingBudgetUs = 20000;
    uint16_t readingFreshnessMs = 150;
};
