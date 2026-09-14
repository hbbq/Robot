#pragma once

#include <cstdint>

#include <FrontScanTelemetryConfig.h>

class DistanceSensorScanner;
class IFrontScanMeasurementSender;
class IClock;

class FrontScanTelemetryPublisher
{
public:
    FrontScanTelemetryPublisher(
        const DistanceSensorScanner& scanner,
        IFrontScanMeasurementSender& sender,
        IClock& clock,
        const FrontScanTelemetryConfig& config);

    void update();

private:
    const DistanceSensorScanner& _scanner;
    IFrontScanMeasurementSender& _sender;
    IClock& _clock;
    const FrontScanTelemetryConfig& _config;
    uint32_t _observedSampleRevision = 0;
    uint32_t _lastSentSampleRevision = 0;
    uint32_t _lastSentMs = 0;
    uint32_t _lastAttemptMs = 0;
    bool _hasSent = false;
    bool _hasAttempted = false;

    bool send(uint32_t nowMs);
};
