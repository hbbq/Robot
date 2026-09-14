#pragma once

struct FrontScanMeasurement;

class IFrontScanMeasurementSender
{
public:
    virtual ~IFrontScanMeasurementSender() = default;

    virtual bool sendFrontScanMeasurement(
        const FrontScanMeasurement& measurement) = 0;
};
