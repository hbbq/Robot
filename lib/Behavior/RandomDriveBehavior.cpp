#include "RandomDriveBehavior.h"

#include <Arduino.h>
#include <DistanceSensorScanner.h>
#include <IMotionController.h>
#include <IClock.h>
#include <IRandom.h>

RandomDriveBehavior::RandomDriveBehavior(
    IMotionController& motionController,
    DistanceSensorScanner& distanceSensorScanner,
    IClock& clock,
    IRandom& random,
    const RandomDriveBehaviorConfig& config)
    : _motionController(motionController),
      _distanceSensorScanner(distanceSensorScanner),
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
                if (_random.next(0, 100) <
                    _config.forwardChancePercent)
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
                startLeftScan();
            }
            break;

        case State::ScanningLeft:
            if (_distanceSensorScanner.isComplete())
            {
                finishLeftScan();
            }
            break;

        case State::ScanningRight:
            if (_distanceSensorScanner.isComplete())
            {
                finishRightScan();
            }
            break;

        case State::CenteringSensor:
            if (_distanceSensorScanner.isComplete())
            {
                chooseAvoidanceDirection();
            }
            break;

        case State::AvoidanceTurning:
            if (!_motionController.isBusy())
            {
                startWaiting();
            }
            break;

        case State::Blocked:
            break;
    }
}

RobotMode RandomDriveBehavior::mode() const
{
    return RobotMode::Autonomous;
}

void RandomDriveBehavior::startWaiting()
{
    _distanceSensorScanner.startContinuousSweep();
    _forwardRefusalLogged = false;
    _state = State::Waiting;
    _waitDurationMs = _random.next(
        _config.minimumWaitMs,
        _config.maximumWaitMs);
    _waitStartedMs = _clock.millis();
}

void RandomDriveBehavior::startForward()
{
    const FrontScanAssessment assessment =
        _distanceSensorScanner.assessFront(
            _config.obstacleThresholdMillimeters);

    if (assessment != FrontScanAssessment::Clear)
    {
        const uint8_t assessmentValue =
            static_cast<uint8_t>(assessment);

        if (!_forwardRefusalLogged ||
            assessmentValue != _lastForwardRefusalAssessment)
        {
            Serial.printf(
                "[RandomDrive] state=Waiting scan=%s sweep=%u "
                "forward=REFUSED\n",
                assessment == FrontScanAssessment::Obstacle
                    ? "OBSTACLE"
                    : "INCOMPLETE",
                _distanceSensorScanner.isContinuousSweepActive()
                    ? 1u
                    : 0u);
            _forwardRefusalLogged = true;
            _lastForwardRefusalAssessment = assessmentValue;
        }
        return;
    }

    _forwardRefusalLogged = false;

    Serial.printf(
        "[RandomDrive] state=Waiting scan=CLEAR sweep=%u "
        "forward=STARTED\n",
        _distanceSensorScanner.isContinuousSweepActive()
            ? 1u
            : 0u);

    _state = State::MovingForward;
    _sensorReadingLost = false;

    const float meters =
        _random.nextFloat(
            _config.minimumForwardDistanceMeters,
            _config.maximumForwardDistanceMeters);

    _motionController.goForward(meters);
}

void RandomDriveBehavior::startTurn()
{
    _distanceSensorScanner.lookCenter();
    _state = State::Turning;

    float degrees =
        _random.nextFloat(
            _config.minimumTurnDegrees,
            _config.maximumTurnDegrees);

    if (_random.next(0, 2) == 0)
    {
        degrees = -degrees;
    }

    _motionController.turn(degrees);
}

void RandomDriveBehavior::startAvoidance()
{
    _motionController.stop();
    _distanceSensorScanner.lookCenter();
    _state = State::BackingAway;

    _leftReadingValid = false;
    _rightReadingValid = false;

    _motionController.goBackward(
        _config.backupDistanceMeters);
}

void RandomDriveBehavior::startLeftScan()
{
    _state = State::ScanningLeft;
    _distanceSensorScanner.lookLeft();
}

void RandomDriveBehavior::finishLeftScan()
{
    _leftReadingValid =
        _distanceSensorScanner.hasValidReading();

    if (_leftReadingValid)
    {
        _leftDistanceMillimeters =
            _distanceSensorScanner.distanceMillimeters();
    }

    _state = State::ScanningRight;
    _distanceSensorScanner.lookRight();
}

void RandomDriveBehavior::finishRightScan()
{
    _rightReadingValid =
        _distanceSensorScanner.hasValidReading();

    if (_rightReadingValid)
    {
        _rightDistanceMillimeters =
            _distanceSensorScanner.distanceMillimeters();
    }

    _state = State::CenteringSensor;
    _distanceSensorScanner.lookCenter();
}

void RandomDriveBehavior::chooseAvoidanceDirection()
{
    if (!_leftReadingValid && !_rightReadingValid)
    {
        _motionController.stop();
        _state = State::Blocked;
        return;
    }

    if (_leftReadingValid && !_rightReadingValid)
    {
        _avoidanceTurnLeft = true;
    }
    else if (!_leftReadingValid && _rightReadingValid)
    {
        _avoidanceTurnLeft = false;
    }
    else if (_leftDistanceMillimeters !=
             _rightDistanceMillimeters)
    {
        _avoidanceTurnLeft =
            _leftDistanceMillimeters >
            _rightDistanceMillimeters;
    }
    else
    {
        _avoidanceTurnLeft =
            _random.next(0, 2) == 0;
    }

    startAvoidanceTurn();
}

void RandomDriveBehavior::startAvoidanceTurn()
{
    _state = State::AvoidanceTurning;

    float degrees =
        _random.nextFloat(
            _config.minimumAvoidanceTurnDegrees,
            _config.maximumAvoidanceTurnDegrees);

    if (_avoidanceTurnLeft)
    {
        degrees = -degrees;
    }

    _motionController.turn(degrees);
}

void RandomDriveBehavior::updateMovingForward()
{
    const uint32_t nowMs = _clock.millis();

    const FrontScanAssessment assessment =
        _distanceSensorScanner.assessFront(
            _config.obstacleThresholdMillimeters);

    if (assessment == FrontScanAssessment::Obstacle)
    {
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
        startWaiting();
    }
}
