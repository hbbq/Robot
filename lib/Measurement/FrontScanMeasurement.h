#pragma once

#include <cstdint>

struct FrontScanSectorMeasurement
{
    bool hasSample = false;
    bool valid = false;
    uint16_t distanceMillimeters = 0;
    uint16_t ageMs = 0;
};

struct FrontScanMeasurement
{
    FrontScanSectorMeasurement left;
    FrontScanSectorMeasurement center;
    FrontScanSectorMeasurement right;
    uint16_t sampleFreshnessMs = 0;
};
