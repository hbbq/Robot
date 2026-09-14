#pragma once

#include <cstdint>

#include <FrontScanMeasurement.h>

class FrontScanMeasurementStore
{
public:
    void setMeasurement(
        const FrontScanMeasurement& measurement,
        uint32_t receivedAtMs)
    {
        _measurement = measurement;
        _receivedAtMs = receivedAtMs;
        _hasMeasurement = true;
        ++_revision;
    }

    bool hasMeasurement() const
    {
        return _hasMeasurement;
    }

    const FrontScanMeasurement& measurement() const
    {
        return _measurement;
    }

    uint32_t receivedAtMs() const
    {
        return _receivedAtMs;
    }

    uint32_t revision() const
    {
        return _revision;
    }

    bool isFresh(
        const FrontScanSectorMeasurement& sector,
        uint32_t nowMs) const
    {
        if (!_hasMeasurement || !sector.hasSample || !sector.valid)
        {
            return false;
        }

        if (sector.ageMs > _measurement.sampleFreshnessMs)
        {
            return false;
        }

        const uint32_t remainingFreshnessMs =
            _measurement.sampleFreshnessMs - sector.ageMs;

        return nowMs - _receivedAtMs <= remainingFreshnessMs;
    }

    uint8_t freshnessMask(uint32_t nowMs) const
    {
        uint8_t mask = 0;

        if (isFresh(_measurement.left, nowMs))
        {
            mask |= 1U << 0;
        }

        if (isFresh(_measurement.center, nowMs))
        {
            mask |= 1U << 1;
        }

        if (isFresh(_measurement.right, nowMs))
        {
            mask |= 1U << 2;
        }

        return mask;
    }

private:
    FrontScanMeasurement _measurement;
    uint32_t _receivedAtMs = 0;
    uint32_t _revision = 0;
    bool _hasMeasurement = false;
};
