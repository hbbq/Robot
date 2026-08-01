#pragma once

#include <cstdint>

#include <IDistanceSensor.h>

class FakeDistanceSensor : public IDistanceSensor
{
public:
    bool begin() override
    {
        return _beginResult;
    }

    void update() override
    {
    }

    bool hasValidReading() const override
    {
        return _valid;
    }

    uint16_t distanceMillimeters() const override
    {
        return _distanceMillimeters;
    }

    void setBeginResult(bool result)
    {
        _beginResult = result;
    }

    void setReading(
        uint16_t distanceMillimeters)
    {
        _distanceMillimeters = distanceMillimeters;
        _valid = true;
    }

    void invalidate()
    {
        _valid = false;
    }

private:
    bool _beginResult = true;
    bool _valid = false;
    uint16_t _distanceMillimeters = 0;
};
