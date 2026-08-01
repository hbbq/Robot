#include "RandomDriveBehavior.h"

#include <IDistanceSensor.h>
#include <IMotionController.h>
#include <IClock.h>
#include <IRandom.h>

RandomDriveBehavior::RandomDriveBehavior(
    IMotionController& motionController,
    IDistanceSensor& distanceSensor,
    IClock& clock,
    IRandom& random,
    const RandomDriveBehaviorConfig& config)
    : _motionController(motionController),
      _distanceSensor(distanceSensor),
      _clock(clock),
      _random(random),
      _config(config)
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
            if (_clock.millis() - _waitStartedMs >=
                _waitDurationMs)
            {
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

        case State::MovingForward:
            updateMovingForward();
            break;

        case State::Turning:
            if (!_motionController.isBusy())
            {
                startWaiting();
            }
            break;

        case State::BackingAway:
            if (!_motionController.isBusy())
            {
                startAvoidanceTurn();
            }
            break;

        case State::AvoidanceTurning:
            if (!_motionController.isBusy())
            {
                startWaiting();
            }
            break;
    }
}

RobotMode RandomDriveBehavior::mode() const
{
    return RobotMode::Autonomous;
}

void RandomDriveBehavior::startWaiting()
{
    _state = State::Waiting;
    _waitDurationMs = _random.next(500, 2000);
    _waitStartedMs = _clock.millis();
}

void RandomDriveBehavior::startForward()
{
    if (!_distanceSensor.hasValidReading())
    {
        return;
    }

    _state = State::MovingForward;
    _sensorReadingLost = false;

    const float meters =
        _random.nextFloat(0.2f, 1.0f);

    _motionController.goForward(meters);
}

void RandomDriveBehavior::startTurn()
{
    _state = State::Turning;

    float degrees =
        _random.nextFloat(30.0f, 150.0f);

    if (_random.next(0, 2) == 0)
    {
        degrees = -degrees;
    }

    _motionController.turn(degrees);
}

void RandomDriveBehavior::startAvoidance()
{
    _motionController.stop();
    _state = State::BackingAway;

    _motionController.goBackward(
        _config.backupDistanceMeters);
}

void RandomDriveBehavior::startAvoidanceTurn()
{
    _state = State::AvoidanceTurning;

    float degrees =
        _random.nextFloat(
            _config.minimumAvoidanceTurnDegrees,
            _config.maximumAvoidanceTurnDegrees);

    if (_random.next(0, 2) == 0)
    {
        degrees = -degrees;
    }

    _motionController.turn(degrees);
}

void RandomDriveBehavior::updateMovingForward()
{
    const uint32_t nowMs = _clock.millis();

    if (_distanceSensor.hasValidReading())
    {
        _sensorReadingLost = false;

        if (_distanceSensor.distanceMillimeters() <
            _config.obstacleThresholdMillimeters)
        {
            startAvoidance();
            return;
        }
    }
    else if (!_sensorReadingLost)
    {
        _sensorReadingLost = true;
        _sensorReadingLostAtMs = nowMs;
    }
    else if (nowMs - _sensorReadingLostAtMs >=
             _config.sensorLossTimeoutMs)
    {
        _motionController.stop();
        startWaiting();
        return;
    }

    if (!_motionController.isBusy())
    {
        startWaiting();
    }
}
