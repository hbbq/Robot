#include "FrontScanTelemetryPublisher.h"

#include <algorithm>
#include <limits>

#include <DistanceSensorScanner.h>
#include <FrontScanMeasurement.h>
#include <IClock.h>
#include <IFrontScanMeasurementSender.h>

namespace
{
    FrontScanSectorMeasurement makeSector(
        const DistanceSensorSample& sample,
        uint32_t nowMs)
    {
        const uint32_t ageMs = sample.hasSample
            ? sample.ageMs(nowMs)
            : 0;

        return FrontScanSectorMeasurement
        {
            .hasSample = sample.hasSample,
            .valid = sample.hasSample && sample.valid,
            .distanceMillimeters = sample.hasSample
                ? sample.distanceMillimeters
                : static_cast<uint16_t>(0),
            .ageMs = static_cast<uint16_t>(std::min<uint32_t>(
                ageMs,
                std::numeric_limits<uint16_t>::max()))
        };
    }
}

FrontScanTelemetryPublisher::FrontScanTelemetryPublisher(
    const DistanceSensorScanner& scanner,
    IFrontScanMeasurementSender& sender,
    IClock& clock,
    const FrontScanTelemetryConfig& config)
    : _scanner(scanner),
      _sender(sender),
      _clock(clock),
      _config(config)
{
}

void FrontScanTelemetryPublisher::update()
{
    const uint32_t nowMs = _clock.millis();
    _observedSampleRevision = _scanner.sampleRevision();

    const bool hasNewSample =
        _observedSampleRevision != _lastSentSampleRevision;
    const bool minimumIntervalElapsed =
        !_hasAttempted ||
        nowMs - _lastAttemptMs >= _config.minimumSendIntervalMs;
    const bool periodicUpdate =
        (!_hasSent && nowMs >= _config.snapshotIntervalMs) ||
        (_hasSent &&
         nowMs - _lastSentMs >= _config.snapshotIntervalMs);

    if ((hasNewSample || periodicUpdate) && minimumIntervalElapsed)
    {
        send(nowMs);
    }
}

bool FrontScanTelemetryPublisher::send(uint32_t nowMs)
{
    _hasAttempted = true;
    _lastAttemptMs = nowMs;

    const FrontScanState& scan = _scanner.frontScanState();
    const FrontScanMeasurement measurement
    {
        .left = makeSector(scan.left, nowMs),
        .center = makeSector(scan.center, nowMs),
        .right = makeSector(scan.right, nowMs),
        .sampleFreshnessMs = static_cast<uint16_t>(std::min<uint32_t>(
            _scanner.sampleFreshnessMs(),
            std::numeric_limits<uint16_t>::max()))
    };

    if (!_sender.sendFrontScanMeasurement(measurement))
    {
        return false;
    }

    _hasSent = true;
    _lastSentMs = nowMs;
    _lastSentSampleRevision = _observedSampleRevision;
    return true;
}
