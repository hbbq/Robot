#pragma once

#include <cstdint>

class IDistanceSensor
{
public:
    virtual ~IDistanceSensor() = default;

    virtual bool begin() = 0;
    virtual void update() = 0;

    virtual bool hasValidReading() const = 0;
    virtual uint16_t distanceMillimeters() const = 0;
    virtual uint32_t readingSequence() const = 0;
};
