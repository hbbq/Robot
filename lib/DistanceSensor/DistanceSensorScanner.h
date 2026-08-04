#pragma once

#include <cstdint>

#include <DistanceSensorPanConfig.h>

class IServoController;
class IDistanceSensor;
class IClock;

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

    bool isBusy() const;
    bool isComplete() const;
    bool hasValidReading() const;
    uint16_t distanceMillimeters() const;

private:
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

    void lookAt(float angle);
};
