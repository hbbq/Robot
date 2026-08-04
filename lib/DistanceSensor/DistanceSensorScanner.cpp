#include "DistanceSensorScanner.h"

#include <IClock.h>
#include <IDistanceSensor.h>
#include <IServoController.h>

DistanceSensorScanner::DistanceSensorScanner(
    IServoController& servoController,
    IDistanceSensor& distanceSensor,
    IClock& clock,
    const DistanceSensorPanConfig& config)
    : _servoController(servoController),
      _distanceSensor(distanceSensor),
      _clock(clock),
      _config(config)
{
}

void DistanceSensorScanner::begin()
{
    lookCenter();
}

void DistanceSensorScanner::update()
{
    const uint32_t nowMs = _clock.millis();

    switch (_state)
    {
        case State::Settling:
            if (nowMs - _stateStartedMs >=
                _config.settleTimeMs)
            {
                _readingSequenceAtSettle =
                    _distanceSensor.readingSequence();
                _stateStartedMs = nowMs;
                _state = State::WaitingForReading;
            }
            break;

        case State::WaitingForReading:
            if (_distanceSensor.hasValidReading() &&
                _distanceSensor.readingSequence() !=
                    _readingSequenceAtSettle)
            {
                _distanceMillimeters =
                    _distanceSensor.distanceMillimeters();
                _hasValidReading = true;
                _state = State::Complete;
            }
            else if (nowMs - _stateStartedMs >=
                     _config.readingTimeoutMs)
            {
                _hasValidReading = false;
                _state = State::Complete;
            }
            break;

        case State::Idle:
        case State::Complete:
            break;
    }
}

void DistanceSensorScanner::lookCenter()
{
    lookAt(_config.centerAngle);
}

void DistanceSensorScanner::lookLeft()
{
    lookAt(_config.leftAngle);
}

void DistanceSensorScanner::lookRight()
{
    lookAt(_config.rightAngle);
}

bool DistanceSensorScanner::isBusy() const
{
    return _state == State::Settling ||
        _state == State::WaitingForReading;
}

bool DistanceSensorScanner::isComplete() const
{
    return _state == State::Complete;
}

bool DistanceSensorScanner::hasValidReading() const
{
    return isComplete() && _hasValidReading;
}

uint16_t DistanceSensorScanner::distanceMillimeters() const
{
    return _distanceMillimeters;
}

void DistanceSensorScanner::lookAt(float angle)
{
    _servoController.setAngle(angle);
    _hasValidReading = false;
    _stateStartedMs = _clock.millis();
    _state = State::Settling;
}
