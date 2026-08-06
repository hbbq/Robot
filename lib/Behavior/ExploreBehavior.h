#pragma once

#include <cstdint>

#include <ExploreBehaviorConfig.h>
#include <IBehavior.h>

class IMotionController;
class DistanceSensorScanner;
class IClock;
class IRandom;

class ExploreBehavior : public IBehavior
{
public:
    ExploreBehavior(
        IMotionController& motionController,
        DistanceSensorScanner& distanceSensorScanner,
        IClock& clock,
        IRandom& random,
        const ExploreBehaviorConfig& config);

    void begin() override;
    void update() override;
    RobotMode mode() const override;

private:
    enum class State
    {
        Waiting,
        MovingForward,
        CourseCorrecting,
        BackingAway,
        AvoidanceTurning
    };

    IMotionController& _motionController;
    DistanceSensorScanner& _distanceSensorScanner;
    IClock& _clock;
    IRandom& _random;
    const ExploreBehaviorConfig& _config;

    State _state = State::Waiting;
    uint32_t _waitStartedMs = 0;
    bool _sensorReadingLost = false;
    uint32_t _sensorReadingLostAtMs = 0;
    bool _forwardRefusalLogged = false;
    uint8_t _lastForwardRefusalAssessment = 0xFF;

    void startWaiting();
    void startForward();
    void finishForwardMovement();
    void startCourseCorrection();
    void startAvoidance();
    void startAvoidanceTurn();
    void updateMovingForward();
};
