#include "RandomDriveBehavior.h"

#include <MotionController.h>
#include <IClock.h>
#include <IRandom.h>

RandomDriveBehavior::RandomDriveBehavior(
    MotionController& motionController,
    IClock& clock,
    IRandom& random)
    : _motionController(motionController),
      _clock(clock),
      _random(random)
{
}

void RandomDriveBehavior::begin()
{
    _motionController.stop();
    startWaiting();
}

void RandomDriveBehavior::update()
{
    switch (_state)
    {
        case State::Waiting:
        {
            if (_clock.millis() >= _waitUntilMs)
            {
                // Exempelvis 70 % chans att köra framåt
                // och 30 % att svänga.
                if (_random.next(0, 100) < 70)
                {
                    startForward();
                }
                else
                {
                    startTurn();
                }
            }

            break;
        }

        case State::MovingForward:
        {
            if (!_motionController.isBusy())
            {
                startWaiting();
            }

            break;
        }

        case State::Turning:
        {
            if (!_motionController.isBusy())
            {
                startWaiting();
            }

            break;
        }
    }
}

void RandomDriveBehavior::startWaiting()
{
    _state = State::Waiting;

    const uint32_t waitMs =
        _random.next(500, 2000);

    _waitUntilMs =
        _clock.millis() + waitMs;
}

void RandomDriveBehavior::startForward()
{
    _state = State::MovingForward;

    const float meters =
        _random.nextFloat(
            0.2f,
            1.0f);

    _motionController.goForward(
        meters);
}

void RandomDriveBehavior::startTurn()
{
    _state = State::Turning;

    float degrees =
        _random.nextFloat(
            30.0f,
            150.0f);

    if (_random.next(0, 2) == 0)
    {
        degrees = -degrees;
    }

    _motionController.turn(
        degrees);
}