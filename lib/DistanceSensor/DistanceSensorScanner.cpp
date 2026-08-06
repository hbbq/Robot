#include "DistanceSensorScanner.h"

#include <Arduino.h>
#include <IClock.h>
#include <IDistanceSensor.h>
#include <IServoController.h>

FrontScanAssessment FrontScanState::assess(
    uint32_t nowMs,
    uint32_t freshnessMs,
    uint16_t obstacleThresholdMillimeters) const
{
    const DistanceSensorSample* samples[] =
    {
        &left,
        &center,
        &right
    };

    for (const DistanceSensorSample* sample : samples)
    {
        if (sample->isFresh(nowMs, freshnessMs) &&
            sample->valid &&
            sample->distanceMillimeters <
            obstacleThresholdMillimeters)
        {
            return FrontScanAssessment::Obstacle;
        }
    }

    for (const DistanceSensorSample* sample : samples)
    {
        if (!sample->isFresh(nowMs, freshnessMs) ||
            !sample->valid)
        {
            return FrontScanAssessment::Incomplete;
        }
    }

    return FrontScanAssessment::Clear;
}

DistanceSensorScanner::DistanceSensorScanner(
    IServoController& servoController,
    IDistanceSensor& distanceSensor,
    IClock& clock,
    const DistanceSensorPanConfig& config)
    : _servoController(servoController),
      _distanceSensor(distanceSensor),
      _clock(clock),
      _config(config)
{
}

void DistanceSensorScanner::begin()
{
    lookCenter();
}

void DistanceSensorScanner::update()
{
    const uint32_t nowMs = _clock.millis();

    switch (_state)
    {
        case State::Settling:
            if (nowMs - _stateStartedMs >=
                _config.settleTimeMs)
            {
                _readingSequenceAtSettle =
                    _distanceSensor.readingSequence();
                Serial.printf(
                    "[FrontScan] settled dir=%s baselineSeq=%lu\n",
                    _targetDirection == DistanceSensorDirection::Left
                        ? "LEFT"
                        : _targetDirection == DistanceSensorDirection::Right
                            ? "RIGHT"
                            : "CENTER",
                    static_cast<unsigned long>(
                        _readingSequenceAtSettle));
                _stateStartedMs = nowMs;
                _state = State::WaitingForReading;
            }
            break;

        case State::WaitingForReading:
            if (_distanceSensor.hasValidReading() &&
                _distanceSensor.readingSequence() !=
                    _readingSequenceAtSettle)
            {
                completeReading(true, nowMs);
            }
            else if (nowMs - _stateStartedMs >=
                     _config.readingTimeoutMs)
            {
                completeReading(false, nowMs);
            }
            break;

        case State::Idle:
        case State::Complete:
            break;
    }
}

void DistanceSensorScanner::lookCenter()
{
    lookAt(
        _config.centerAngle,
        DistanceSensorDirection::Center);
}

void DistanceSensorScanner::lookLeft()
{
    lookAt(
        _config.leftAngle,
        DistanceSensorDirection::Left);
}

void DistanceSensorScanner::lookRight()
{
    lookAt(
        _config.rightAngle,
        DistanceSensorDirection::Right);
}

void DistanceSensorScanner::startContinuousSweep()
{
    if (_continuousSweep)
    {
        return;
    }

    _continuousSweep = true;
    _sweepIndex = 0;
    lookAt(
        _config.centerAngle,
        DistanceSensorDirection::Center,
        true);
}

void DistanceSensorScanner::stopContinuousSweep()
{
    _continuousSweep = false;
}

bool DistanceSensorScanner::isContinuousSweepActive() const
{
    return _continuousSweep;
}

const FrontScanState& DistanceSensorScanner::frontScanState() const
{
    return _frontScanState;
}

FrontScanAssessment DistanceSensorScanner::assessFront(
    uint16_t obstacleThresholdMillimeters) const
{
    const uint32_t nowMs = _clock.millis();
    const FrontScanAssessment assessment =
        _frontScanState.assess(
        nowMs,
        _config.sampleFreshnessMs,
        obstacleThresholdMillimeters);

    logAssessment(assessment, nowMs);
    return assessment;
}

bool DistanceSensorScanner::isBusy() const
{
    return _state == State::Settling ||
        _state == State::WaitingForReading;
}

bool DistanceSensorScanner::isComplete() const
{
    return _state == State::Complete;
}

bool DistanceSensorScanner::hasValidReading() const
{
    return isComplete() && _hasValidReading;
}

uint16_t DistanceSensorScanner::distanceMillimeters() const
{
    return _distanceMillimeters;
}

void DistanceSensorScanner::lookAt(
    float angle,
    DistanceSensorDirection direction,
    bool preserveContinuousSweep)
{
    if (!preserveContinuousSweep)
    {
        _continuousSweep = false;
    }

    _servoController.setAngle(angle);
    _targetDirection = direction;
    Serial.printf(
        "[FrontScan] target=%s angle=%.1f sweep=%u\n",
        direction == DistanceSensorDirection::Left
            ? "LEFT"
            : direction == DistanceSensorDirection::Right
                ? "RIGHT"
                : "CENTER",
        angle,
        _continuousSweep ? 1u : 0u);
    _hasValidReading = false;
    _stateStartedMs = _clock.millis();
    _state = State::Settling;
}

void DistanceSensorScanner::completeReading(
    bool valid,
    uint32_t nowMs)
{
    DistanceSensorSample& sample =
        sampleFor(_targetDirection);

    sample.hasSample = true;
    sample.valid = valid;
    sample.capturedAtMs = nowMs;

    if (valid)
    {
        _distanceMillimeters =
            _distanceSensor.distanceMillimeters();
        _hasValidReading = true;
        sample.distanceMillimeters =
            _distanceMillimeters;

        Serial.printf(
            "[FrontScan] accepted dir=%s seq=%lu mm=%u\n",
            _targetDirection == DistanceSensorDirection::Left
                ? "LEFT"
                : _targetDirection == DistanceSensorDirection::Right
                    ? "RIGHT"
                    : "CENTER",
            static_cast<unsigned long>(
                _distanceSensor.readingSequence()),
            _distanceMillimeters);
    }
    else
    {
        _hasValidReading = false;
        Serial.printf(
            "[FrontScan] timeout dir=%s no fresh reading after seq=%lu\n",
            _targetDirection == DistanceSensorDirection::Left
                ? "LEFT"
                : _targetDirection == DistanceSensorDirection::Right
                    ? "RIGHT"
                    : "CENTER",
            static_cast<unsigned long>(
                _readingSequenceAtSettle));
    }

    logSamples(nowMs);

    _state = State::Complete;

    if (_continuousSweep)
    {
        advanceContinuousSweep();
    }
}

void DistanceSensorScanner::advanceContinuousSweep()
{
    // Center between side samples keeps the sensor facing forward
    // frequently while still covering both sides of the front sector.
    static constexpr DistanceSensorDirection Sequence[] =
    {
        DistanceSensorDirection::Center,
        DistanceSensorDirection::Left,
        DistanceSensorDirection::Center,
        DistanceSensorDirection::Right
    };

    _sweepIndex = static_cast<uint8_t>(
        (_sweepIndex + 1) %
        (sizeof(Sequence) / sizeof(Sequence[0])));

    const DistanceSensorDirection direction =
        Sequence[_sweepIndex];

    switch (direction)
    {
        case DistanceSensorDirection::Left:
            lookAt(_config.leftAngle, direction, true);
            break;

        case DistanceSensorDirection::Right:
            lookAt(_config.rightAngle, direction, true);
            break;

        case DistanceSensorDirection::Center:
            lookAt(_config.centerAngle, direction, true);
            break;
    }
}

DistanceSensorSample& DistanceSensorScanner::sampleFor(
    DistanceSensorDirection direction)
{
    switch (direction)
    {
        case DistanceSensorDirection::Left:
            return _frontScanState.left;

        case DistanceSensorDirection::Right:
            return _frontScanState.right;

        case DistanceSensorDirection::Center:
        default:
            return _frontScanState.center;
    }
}

void DistanceSensorScanner::logSamples(uint32_t nowMs) const
{
    const DistanceSensorSample& left = _frontScanState.left;
    const DistanceSensorSample& center = _frontScanState.center;
    const DistanceSensorSample& right = _frontScanState.right;

    Serial.printf(
        "[FrontScan] L[h=%u v=%u mm=%u age=%lu] "
        "C[h=%u v=%u mm=%u age=%lu] "
        "R[h=%u v=%u mm=%u age=%lu]\n",
        left.hasSample ? 1u : 0u,
        left.valid ? 1u : 0u,
        left.distanceMillimeters,
        static_cast<unsigned long>(left.ageMs(nowMs)),
        center.hasSample ? 1u : 0u,
        center.valid ? 1u : 0u,
        center.distanceMillimeters,
        static_cast<unsigned long>(center.ageMs(nowMs)),
        right.hasSample ? 1u : 0u,
        right.valid ? 1u : 0u,
        right.distanceMillimeters,
        static_cast<unsigned long>(right.ageMs(nowMs)));
}

void DistanceSensorScanner::logAssessment(
    FrontScanAssessment assessment,
    uint32_t nowMs) const
{
    const DiagnosticIncompleteReason reason =
        assessment == FrontScanAssessment::Incomplete
            ? incompleteReason(nowMs)
            : DiagnosticIncompleteReason::None;

    if (_hasLoggedAssessment &&
        assessment == _lastLoggedAssessment &&
        reason == _lastLoggedIncompleteReason)
    {
        return;
    }

    _hasLoggedAssessment = true;
    _lastLoggedAssessment = assessment;
    _lastLoggedIncompleteReason = reason;

    const char* assessmentText =
        assessment == FrontScanAssessment::Clear
            ? "CLEAR"
            : assessment == FrontScanAssessment::Obstacle
                ? "OBSTACLE"
                : "INCOMPLETE";

    const char* reasonText = "none";
    switch (reason)
    {
        case DiagnosticIncompleteReason::LeftMissing: reasonText = "left missing"; break;
        case DiagnosticIncompleteReason::LeftInvalid: reasonText = "left invalid"; break;
        case DiagnosticIncompleteReason::LeftStale: reasonText = "left stale"; break;
        case DiagnosticIncompleteReason::CenterMissing: reasonText = "center missing"; break;
        case DiagnosticIncompleteReason::CenterInvalid: reasonText = "center invalid"; break;
        case DiagnosticIncompleteReason::CenterStale: reasonText = "center stale"; break;
        case DiagnosticIncompleteReason::RightMissing: reasonText = "right missing"; break;
        case DiagnosticIncompleteReason::RightInvalid: reasonText = "right invalid"; break;
        case DiagnosticIncompleteReason::RightStale: reasonText = "right stale"; break;
        case DiagnosticIncompleteReason::None: break;
    }

    Serial.printf(
        "[FrontScan] assessment=%s%s%s\n",
        assessmentText,
        assessment == FrontScanAssessment::Incomplete ? " reason=" : "",
        assessment == FrontScanAssessment::Incomplete ? reasonText : "");
}

DistanceSensorScanner::DiagnosticIncompleteReason
DistanceSensorScanner::incompleteReason(uint32_t nowMs) const
{
    const struct
    {
        const DistanceSensorSample& sample;
        DiagnosticIncompleteReason missing;
        DiagnosticIncompleteReason invalid;
        DiagnosticIncompleteReason stale;
    } sectors[] =
    {
        {_frontScanState.left, DiagnosticIncompleteReason::LeftMissing,
            DiagnosticIncompleteReason::LeftInvalid, DiagnosticIncompleteReason::LeftStale},
        {_frontScanState.center, DiagnosticIncompleteReason::CenterMissing,
            DiagnosticIncompleteReason::CenterInvalid, DiagnosticIncompleteReason::CenterStale},
        {_frontScanState.right, DiagnosticIncompleteReason::RightMissing,
            DiagnosticIncompleteReason::RightInvalid, DiagnosticIncompleteReason::RightStale}
    };

    for (const auto& sector : sectors)
    {
        if (!sector.sample.hasSample)
        {
            return sector.missing;
        }
        if (!sector.sample.valid)
        {
            return sector.invalid;
        }
        if (!sector.sample.isFresh(nowMs, _config.sampleFreshnessMs))
        {
            return sector.stale;
        }
    }

    return DiagnosticIncompleteReason::None;
}
