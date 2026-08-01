#include "ExploreBehavior.h"

#include <IClock.h>
#include <IDistanceSensor.h>
#include <IMotionController.h>
#include <IRandom.h>

ExploreBehavior::ExploreBehavior(
    IMotionController& motionController,
    IDistanceSensor& distanceSensor,
    IClock& clock,
    IRandom& random,
    const ExploreBehaviorConfig& config)
    : _motionController(motionController),
      _distanceSensor(distanceSensor),
      _clock(clock),
      _random(random),
      _config(config)
{
}

void ExploreBehavior::begin()
{
    _motionController.stop();
    startWaiting();
}

void ExploreBehavior::update()
{
    switch (_state)
    {
        case State::Waiting:
            if (_clock.millis() - _waitStartedMs >=
                    _config.pauseBetweenMovesMs &&
                _distanceSensor.hasValidReading())
            {
                startForward();
            }
            break;

        case State::MovingForward:
            updateMovingForward();
            break;

        case State::CourseCorrecting:
        case State::AvoidanceTurning:
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
    }
}

RobotMode ExploreBehavior::mode() const
{
    return RobotMode::Autonomous;
}

void ExploreBehavior::startWaiting()
{
    _state = State::Waiting;
    _waitStartedMs = _clock.millis();
}

void ExploreBehavior::startForward()
{
    _state = State::MovingForward;
    _sensorReadingLost = false;

    _motionController.goForward(
        _random.nextFloat(
            _config.minimumForwardDistanceMeters,
            _config.maximumForwardDistanceMeters));
}

void ExploreBehavior::finishForwardMovement()
{
    if (_random.next(0, 100) <
        _config.courseCorrectionChancePercent)
    {
        startCourseCorrection();
        return;
    }

    startWaiting();
}

void ExploreBehavior::startCourseCorrection()
{
    _state = State::CourseCorrecting;

    float degrees =
        _random.nextFloat(
            _config.minimumCourseCorrectionDegrees,
            _config.maximumCourseCorrectionDegrees);

    if (_random.next(0, 2) == 0)
    {
        degrees = -degrees;
    }

    _motionController.turn(degrees);
}

void ExploreBehavior::startAvoidance()
{
    _motionController.stop();
    _state = State::BackingAway;
    _motionController.goBackward(
        _config.backupDistanceMeters);
}

void ExploreBehavior::startAvoidanceTurn()
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

void ExploreBehavior::updateMovingForward()
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
        finishForwardMovement();
    }
}
