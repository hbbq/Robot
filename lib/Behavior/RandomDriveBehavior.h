#pragma once

#include <cstdint>

#include <IBehavior.h>
#include <RandomDriveBehaviorConfig.h>

class IMotionController;
class IDistanceSensor;
class IClock;
class IRandom;

class RandomDriveBehavior : public IBehavior
{
public:
    RandomDriveBehavior(
        IMotionController& motionController,
        IDistanceSensor& distanceSensor,
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
        AvoidanceTurning
    };

    IMotionController& _motionController;
    IDistanceSensor& _distanceSensor;
    IClock& _clock;
    IRandom& _random;
    const RandomDriveBehaviorConfig& _config;

    State _state = State::Waiting;

    uint32_t _waitStartedMs = 0;
    uint32_t _waitDurationMs = 0;

    bool _sensorReadingLost = false;
    uint32_t _sensorReadingLostAtMs = 0;

    void startWaiting();
    void startForward();
    void startTurn();
    void startAvoidance();
    void startAvoidanceTurn();
    void updateMovingForward();
};
