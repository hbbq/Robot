#include "ExploreBehavior.h"

#include <Arduino.h>
#include <IClock.h>
#include <DistanceSensorScanner.h>
#include <IMotionController.h>
#include <IRandom.h>

ExploreBehavior::ExploreBehavior(
    IMotionController& motionController,
    DistanceSensorScanner& distanceSensorScanner,
    IClock& clock,
    IRandom& random,
    const ExploreBehaviorConfig& config)
    : _motionController(motionController),
      _distanceSensorScanner(distanceSensorScanner),
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
                _config.pauseBetweenMovesMs)
            {
                const FrontScanAssessment assessment =
                    _distanceSensorScanner.assessFront(
                        _config.obstacleThresholdMillimeters);

                if (assessment == FrontScanAssessment::Clear)
                {
                    _forwardRefusalLogged = false;
                    Serial.printf(
                        "[Explore] state=Waiting scan=CLEAR sweep=%u "
                        "forward=STARTED\n",
                        _distanceSensorScanner.isContinuousSweepActive()
                            ? 1u
                            : 0u);
                    startForward();
                }
                else if (assessment == FrontScanAssessment::Obstacle)
                {
                    _forwardRefusalLogged = false;
                    Serial.printf(
                        "[Explore] state=Waiting scan=OBSTACLE sweep=%u "
                        "transition=BackingAway\n",
                        _distanceSensorScanner.isContinuousSweepActive()
                            ? 1u
                            : 0u);
                    startAvoidance();
                }
                else
                {
                    const uint8_t assessmentValue =
                        static_cast<uint8_t>(assessment);

                    if (!_forwardRefusalLogged ||
                        assessmentValue !=
                            _lastForwardRefusalAssessment)
                    {
                        Serial.printf(
                            "[Explore] state=Waiting scan=INCOMPLETE sweep=%u "
                            "forward=REFUSED reason=waiting-for-valid-front\n",
                            _distanceSensorScanner.isContinuousSweepActive()
                                ? 1u
                                : 0u);
                        _forwardRefusalLogged = true;
                        _lastForwardRefusalAssessment =
                            assessmentValue;
                    }
                }
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
    _distanceSensorScanner.startContinuousSweep();
    _forwardRefusalLogged = false;
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
    _distanceSensorScanner.lookCenter();
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
    _distanceSensorScanner.lookCenter();
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

    const FrontScanAssessment assessment =
        _distanceSensorScanner.assessFront(
            _config.obstacleThresholdMillimeters);

    if (assessment == FrontScanAssessment::Obstacle)
    {
        Serial.printf(
            "[Explore] state=MovingForward scan=OBSTACLE sweep=%u "
            "transition=BackingAway\n",
            _distanceSensorScanner.isContinuousSweepActive()
                ? 1u
                : 0u);
        startAvoidance();
        return;
    }
    else if (assessment == FrontScanAssessment::Clear)
    {
        _sensorReadingLost = false;
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
