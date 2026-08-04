#pragma once

#include <cstdint>

#include <IBehavior.h>
#include <RandomDriveBehaviorConfig.h>

class IMotionController;
class IDistanceSensor;
class DistanceSensorScanner;
class IClock;
class IRandom;

class RandomDriveBehavior : public IBehavior
{
public:
    RandomDriveBehavior(
        IMotionController& motionController,
        IDistanceSensor& distanceSensor,
        DistanceSensorScanner& distanceSensorScanner,
        IClock& clock,
        IRandom& random,
        const RandomDriveBehaviorConfig& config);

    void begin() override;
    void update() override;
    RobotMode mode() const override;

private:
    enum class State
    {
        Waiting,
        MovingForward,
        Turning,
        BackingAway,
        ScanningLeft,
        ScanningRight,
        CenteringSensor,
        AvoidanceTurning,
        Blocked
    };

    IMotionController& _motionController;
    IDistanceSensor& _distanceSensor;
    DistanceSensorScanner& _distanceSensorScanner;
    IClock& _clock;
    IRandom& _random;
    const RandomDriveBehaviorConfig& _config;

    State _state = State::Waiting;

    uint32_t _waitStartedMs = 0;
    uint32_t _waitDurationMs = 0;

    bool _sensorReadingLost = false;
    uint32_t _sensorReadingLostAtMs = 0;

    bool _leftReadingValid = false;
    bool _rightReadingValid = false;
    uint16_t _leftDistanceMillimeters = 0;
    uint16_t _rightDistanceMillimeters = 0;
    bool _avoidanceTurnLeft = false;

    void startWaiting();
    void startForward();
    void startTurn();
    void startAvoidance();
    void startLeftScan();
    void finishLeftScan();
    void finishRightScan();
    void chooseAvoidanceDirection();
    void startAvoidanceTurn();
    void updateMovingForward();
};
