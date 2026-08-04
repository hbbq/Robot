#include "CalibrationBehavior.h"

#include <DistanceSensorScanner.h>
#include <IClock.h>
#include <IDistanceSensor.h>
#include <IMotionController.h>

#include <cmath>

CalibrationBehavior::CalibrationBehavior(
    IMotionController& motionController,
    IDistanceSensor& distanceSensor,
    DistanceSensorScanner& distanceSensorScanner,
    IClock& clock,
    const CalibrationBehaviorConfig& config)
    : _motionController(motionController),
      _distanceSensor(distanceSensor),
      _distanceSensorScanner(distanceSensorScanner),
      _clock(clock),
      _config(config)
{
}

void CalibrationBehavior::begin()
{
    _motionController.stop();
    _distanceSensorScanner.lookCenter();
    _state = State::Idle;
    refreshLease();
}

void CalibrationBehavior::update()
{
    if (leaseExpired())
    {
        stop();
        return;
    }

    switch (_state)
    {
        case State::CenteringForForward:
            updateCenteringForForward();
            break;

        case State::MovingForward:
            updateMovingForward();
            break;

        case State::MovingOther:
            if (!_motionController.isBusy())
            {
                _state = State::Idle;
            }
            break;

        case State::Idle:
            break;
    }
}

RobotMode CalibrationBehavior::mode() const
{
    return RobotMode::Calibration;
}

bool CalibrationBehavior::submitMotion(
    CalibrationMotionCommand command,
    float value)
{
    if (isBusy() ||
        !std::isfinite(value) ||
        value <= 0.0f)
    {
        return false;
    }

    switch (command)
    {
        case CalibrationMotionCommand::Forward:
            if (value > _config.maximumDistanceMeters)
            {
                return false;
            }

            _pendingForwardMeters = value;
            _sensorReadingLost = false;
            _distanceSensorScanner.lookCenter();
            _state = State::CenteringForForward;
            return true;

        case CalibrationMotionCommand::Backward:
            if (value > _config.maximumDistanceMeters)
            {
                return false;
            }

            _motionController.goBackward(value);
            _state = State::MovingOther;
            return true;

        case CalibrationMotionCommand::TurnLeft:
            if (value > _config.maximumTurnDegrees)
            {
                return false;
            }

            _motionController.turnLeft(value);
            _state = State::MovingOther;
            return true;

        case CalibrationMotionCommand::TurnRight:
            if (value > _config.maximumTurnDegrees)
            {
                return false;
            }

            _motionController.turnRight(value);
            _state = State::MovingOther;
            return true;
    }

    return false;
}

void CalibrationBehavior::stop()
{
    _motionController.stop();
    _state = State::Idle;
    _sensorReadingLost = false;
}

void CalibrationBehavior::refreshLease()
{
    _lastLeaseRefreshMs = _clock.millis();
}

bool CalibrationBehavior::leaseExpired() const
{
    return _clock.millis() - _lastLeaseRefreshMs >=
        _config.sessionLeaseMs;
}

bool CalibrationBehavior::isBusy() const
{
    return _state != State::Idle ||
        _motionController.isBusy();
}

void CalibrationBehavior::updateCenteringForForward()
{
    if (!_distanceSensorScanner.isComplete())
    {
        return;
    }

    if (!_distanceSensorScanner.hasValidReading() ||
        _distanceSensorScanner.distanceMillimeters() <
            _config.obstacleThresholdMillimeters)
    {
        stop();
        return;
    }

    _sensorReadingLost = false;
    _motionController.goForward(_pendingForwardMeters);
    _state = State::MovingForward;
}

void CalibrationBehavior::updateMovingForward()
{
    const uint32_t nowMs = _clock.millis();

    if (_distanceSensor.hasValidReading())
    {
        _sensorReadingLost = false;

        if (_distanceSensor.distanceMillimeters() <
            _config.obstacleThresholdMillimeters)
        {
            stop();
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
        stop();
        return;
    }

    if (!_motionController.isBusy())
    {
        _state = State::Idle;
    }
}
