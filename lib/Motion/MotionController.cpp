#include "MotionController.h"

#include <Arduino.h>
#include <cmath>

MotionController::MotionController(
    IDriveController& drive,
    IClock& clock,
    const MotionControllerConfig& config
)
    : _drive(drive),
      _clock(clock),
      _config(config)
{
}

void MotionController::update()
{
    if (_state == MotionState::Idle)
    {
        return;
    }

    const uint32_t elapsedMs =
        _clock.millis() - _startedAtMs;

    if (elapsedMs >= _durationMs)
    {
        stop();
    }
}

void MotionController::goForward(float meters)
{
    Serial.printf(
        "[Motion] Forward %.2f m\n",
        meters);

    const float distance = std::abs(meters);

    if (distance == 0.0f)
    {
        stop();
        return;
    }

    start(
        MotionState::MovingForward,
        static_cast<uint32_t>(
            distance * _config.millisecondsPerMeter
        )
    );

    _drive.forward(_config.moveSpeed);
}

void MotionController::goBackward(float meters)
{
    Serial.printf(
        "[Motion] Backward %.2f m\n",
        meters);

    const float distance = std::abs(meters);

    if (distance == 0.0f)
    {
        stop();
        return;
    }

    start(
        MotionState::MovingBackward,
        static_cast<uint32_t>(
            distance * _config.millisecondsPerMeter
        )
    );

    _drive.backward(_config.moveSpeed);
}

void MotionController::turnLeft(float degrees)
{
    Serial.printf(
        "[Motion] Turn left %.1f deg\n",
        degrees);

    const float angle = std::abs(degrees);

    if (angle == 0.0f)
    {
        stop();
        return;
    }

    start(
        MotionState::TurningLeft,
        static_cast<uint32_t>(
            angle * _config.millisecondsPerDegree
        )
    );

    _drive.rotateLeft(_config.turnSpeed);
}

void MotionController::turnRight(float degrees)
{
    Serial.printf(
        "[Motion] Turn right %.1f deg\n",
        degrees);

    const float angle = std::abs(degrees);

    if (angle == 0.0f)
    {
        stop();
        return;
    }

    start(
        MotionState::TurningRight,
        static_cast<uint32_t>(
            angle * _config.millisecondsPerDegree
        )
    );

    _drive.rotateRight(_config.turnSpeed);
}

void MotionController::turn(float degrees)
{
    if (degrees < 0.0f)
    {
        turnLeft(-degrees);
    }
    else if (degrees > 0.0f)
    {
        turnRight(degrees);
    }
}

void MotionController::stop()
{
    if (_drive.getState() != DriveState::Stopped)
    {
        Serial.println("[Motion] Stop");
    }

    _drive.stop();

    _state = MotionState::Idle;
    _startedAtMs = 0;
    _durationMs = 0;
}

bool MotionController::isBusy() const
{
    return _state != MotionState::Idle;
}

MotionState MotionController::getState() const
{
    return _state;
}

uint32_t MotionController::getDurationMs() const
{
    return _durationMs;
}

void MotionController::start(
    MotionState state,
    uint32_t durationMs
)
{
    if (_state != MotionState::Idle)
    {
        _drive.stop();
    }

    _state = state;
    _startedAtMs = _clock.millis();
    _durationMs = durationMs;
}