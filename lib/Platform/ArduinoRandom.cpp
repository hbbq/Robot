#include "ArduinoRandom.h"

#include <esp_random.h>


int32_t ArduinoRandom::next(
    int32_t min,
    int32_t max)
{
    const uint32_t value =
        esp_random();

    return min +
        static_cast<int32_t>(
            value % static_cast<uint32_t>(max - min));
}

float ArduinoRandom::nextFloat(
    float min,
    float max)
{
    const float value =
        static_cast<float>(
            esp_random()) /
        static_cast<float>(
            UINT32_MAX);

    return min +
        value * (max - min);
}
