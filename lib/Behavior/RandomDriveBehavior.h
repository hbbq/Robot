#pragma once

#include <cstdint>

#include <IBehavior.h>

class MotionController;
class IClock;
class IRandom;

class RandomDriveBehavior : public IBehavior
{
public:
    RandomDriveBehavior(
        MotionController& motionController,
        IClock& clock,
        IRandom& random);

    void begin() override;
    void update() override;

private:
    enum class State
    {
        Waiting,
        MovingForward,
        Turning
    };

    MotionController& _motionController;
    IClock& _clock;
    IRandom& _random;

    State _state = State::Waiting;

    uint32_t _waitUntilMs = 0;

    void startWaiting();
    void startForward();
    void startTurn();
};