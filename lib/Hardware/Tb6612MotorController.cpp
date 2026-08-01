#include "Tb6612MotorController.h"

#include <Arduino.h>

#include <algorithm>
#include <cmath>

Tb6612MotorController::Tb6612MotorController(
    const Tb6612MotorControllerConfig& config
)
    : _config(config)
{
}

void Tb6612MotorController::begin()
{
    pinMode(_config.in1Pin, OUTPUT);
    pinMode(_config.in2Pin, OUTPUT);
    pinMode(_config.pwmPin, OUTPUT);

    _maxDuty =
        (1UL << _config.pwmResolutionBits) - 1UL;

    ledcAttach(
        _config.pwmPin,
        _config.pwmFrequency,
        _config.pwmResolutionBits
    );

    stop();
}

void Tb6612MotorController::setSpeed(float speed)
{
    speed = std::clamp(speed, -1.0f, 1.0f);

    if (std::abs(speed) < 0.001f)
    {
        stop();
        return;
    }

    _speed = speed;

    bool forward = speed > 0.0f;

    if (_config.inverted)
    {
        forward = !forward;
    }

    setDirection(forward);
    writePwm(std::abs(speed));
}

float Tb6612MotorController::getSpeed() const
{
    return _speed;
}

void Tb6612MotorController::stop()
{
    _speed = 0.0f;

    ledcWrite(_config.pwmPin, 0);

    digitalWrite(_config.in1Pin, LOW);
    digitalWrite(_config.in2Pin, LOW);
}

void Tb6612MotorController::setDirection(bool forward)
{
    if (forward)
    {
        digitalWrite(_config.in1Pin, HIGH);
        digitalWrite(_config.in2Pin, LOW);
    }
    else
    {
        digitalWrite(_config.in1Pin, LOW);
        digitalWrite(_config.in2Pin, HIGH);
    }
}

void Tb6612MotorController::writePwm(float magnitude)
{
    magnitude = std::clamp(magnitude, 0.0f, 1.0f);

    if (
        magnitude > 0.0f &&
        magnitude < _config.minimumSpeed
    )
    {
        magnitude = _config.minimumSpeed;
    }

    const uint32_t duty = static_cast<uint32_t>(
        magnitude * static_cast<float>(_maxDuty)
    );

    ledcWrite(_config.pwmPin, duty);
}