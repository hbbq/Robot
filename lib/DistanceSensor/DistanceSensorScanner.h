#pragma once

#include <cstdint>

#include <DistanceSensorPanConfig.h>

class IServoController;
class IDistanceSensor;
class IClock;

enum class DistanceSensorDirection : uint8_t
{
    Left,
    Center,
    Right
};

struct DistanceSensorSample
{
    bool hasSample = false;
    bool valid = false;
    uint16_t distanceMillimeters = 0;
    uint32_t capturedAtMs = 0;

    uint32_t ageMs(uint32_t nowMs) const
    {
        return nowMs - capturedAtMs;
    }

    bool isFresh(uint32_t nowMs, uint32_t freshnessMs) const
    {
        return hasSample &&
            ageMs(nowMs) <= freshnessMs;
    }
};

enum class FrontScanAssessment : uint8_t
{
    Clear,
    Obstacle,
    Incomplete
};

struct FrontScanState
{
    DistanceSensorSample left;
    DistanceSensorSample center;
    DistanceSensorSample right;

    FrontScanAssessment assess(
        uint32_t nowMs,
        uint32_t freshnessMs,
        uint16_t obstacleThresholdMillimeters) const;
};

class DistanceSensorScanner
{
public:
    DistanceSensorScanner(
        IServoController& servoController,
        IDistanceSensor& distanceSensor,
        IClock& clock,
        const DistanceSensorPanConfig& config);

    void begin();
    void update();

    void lookCenter();
    void lookLeft();
    void lookRight();

    void startContinuousSweep();
    void stopContinuousSweep();
    bool isContinuousSweepActive() const;

    const FrontScanState& frontScanState() const;
    uint32_t sampleRevision() const;
    uint32_t sampleFreshnessMs() const;
    FrontScanAssessment assessFront(
        uint16_t obstacleThresholdMillimeters) const;

    bool isBusy() const;
    bool isComplete() const;
    bool hasValidReading() const;
    uint16_t distanceMillimeters() const;

private:
    enum class DiagnosticIncompleteReason : uint8_t
    {
        None,
        LeftMissing,
        LeftInvalid,
        LeftStale,
        CenterMissing,
        CenterInvalid,
        CenterStale,
        RightMissing,
        RightInvalid,
        RightStale
    };

    enum class State
    {
        Idle,
        Settling,
        WaitingForReading,
        Complete
    };

    IServoController& _servoController;
    IDistanceSensor& _distanceSensor;
    IClock& _clock;
    const DistanceSensorPanConfig& _config;

    State _state = State::Idle;
    uint32_t _stateStartedMs = 0;
    uint32_t _readingSequenceAtSettle = 0;
    bool _hasValidReading = false;
    uint16_t _distanceMillimeters = 0;
    DistanceSensorDirection _targetDirection =
        DistanceSensorDirection::Center;
    FrontScanState _frontScanState;
    uint32_t _sampleRevision = 0;
    bool _continuousSweep = false;
    uint8_t _sweepIndex = 0;
    mutable bool _hasLoggedAssessment = false;
    mutable FrontScanAssessment _lastLoggedAssessment =
        FrontScanAssessment::Incomplete;
    mutable DiagnosticIncompleteReason _lastLoggedIncompleteReason =
        DiagnosticIncompleteReason::None;

    void lookAt(
        float angle,
        DistanceSensorDirection direction,
        bool preserveContinuousSweep = false);
    void completeReading(bool valid, uint32_t nowMs);
    void advanceContinuousSweep();
    DistanceSensorSample& sampleFor(
        DistanceSensorDirection direction);
    void logSamples(uint32_t nowMs) const;
    void logAssessment(
        FrontScanAssessment assessment,
        uint32_t nowMs) const;
    DiagnosticIncompleteReason incompleteReason(
        uint32_t nowMs) const;
};
