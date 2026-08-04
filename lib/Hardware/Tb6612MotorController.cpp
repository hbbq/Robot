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

    _maxDuty =
        (1UL << _config.pwmResolutionBits) - 1UL;

    const bool attached = ledcAttachChannel(
        _config.pwmPin,
        _config.pwmFrequency,
        _config.pwmResolutionBits,
        _config.pwmChannel);

    Serial.printf(
        "Motor begin: in1=%d in2=%d pwm=%d channel=%u freq=%lu res=%u attached=%s\n",
        _config.in1Pin,
        _config.in2Pin,
        _config.pwmPin,
        _config.pwmChannel,
        _config.pwmFrequency,
        _config.pwmResolutionBits,
        attached ? "YES" : "NO"
    );

    if (!attached)
    {
        Serial.printf(
            "[Motor] LEDC attach failed: pwm=%u channel=%u freq=%lu res=%u\n",
            _config.pwmPin,
            _config.pwmChannel,
            static_cast<unsigned long>(_config.pwmFrequency),
            _config.pwmResolutionBits);
    }

    stop();
}

void Tb6612MotorController::setSpeed(float speed)
{Serial.printf(
    "Motor pwm=%d speed=%.2f\n",
    _config.pwmPin,
    speed);
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
    Serial.printf(
    "DIR pwm=%d in1=%d in2=%d\n",
    _config.pwmPin,
    digitalRead(_config.in1Pin),
    digitalRead(_config.in2Pin));
}

void Tb6612MotorController::writePwm(float magnitude)
{
    magnitude = std::clamp(magnitude, 0.0f, 1.0f);

    if (
        magnitude > 0.0f &&
        magnitude < _config.minimumSpeed)
    {
        magnitude = _config.minimumSpeed;
    }

    const uint32_t duty = static_cast<uint32_t>(
        magnitude * static_cast<float>(_maxDuty));

    const bool result = ledcWrite(_config.pwmPin, duty);

    Serial.printf(
        "PWM pin=%d magnitude=%.3f duty=%lu write=%s read=%lu\n",
        _config.pwmPin,
        magnitude,
        duty,
        result ? "YES" : "NO",
        ledcRead(_config.pwmPin));
}
