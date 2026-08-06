#include "Esp32ServoController.h"

#include <Arduino.h>
#include <ServoPulseMapper.h>

#include <algorithm>
#include <cmath>

namespace
{
    constexpr uint32_t ServoPwmFrequencyHz = 50;
    constexpr uint8_t ServoPwmResolutionBits = 14;
    constexpr float MicrosecondsPerSecond = 1000000.0f;
}

Esp32ServoController::Esp32ServoController(
    const ServoControllerConfig& config)
    : _config(config),
      _angleDegrees(std::clamp(
          config.centerAngleDegrees,
          config.minimumAngleDegrees,
          config.maximumAngleDegrees))
{
}

void Esp32ServoController::begin()
{
    _maximumDuty =
        (1UL << ServoPwmResolutionBits) - 1UL;

    const bool attached = ledcAttachChannel(
        _config.pin,
        ServoPwmFrequencyHz,
        ServoPwmResolutionBits,
        _config.pwmChannel);

    if (!attached)
    {
        Serial.printf(
            "[Servo] LEDC attach failed: pin=%u channel=%u freq=%lu res=%u\n",
            _config.pin,
            _config.pwmChannel,
            static_cast<unsigned long>(ServoPwmFrequencyHz),
            ServoPwmResolutionBits);
    }

    setAngle(_config.centerAngleDegrees);
}

void Esp32ServoController::setAngle(float degrees)
{
    const float safeDegrees = std::isfinite(degrees)
        ? degrees
        : _config.centerAngleDegrees;

    _angleDegrees = std::clamp(
        safeDegrees,
        _config.minimumAngleDegrees,
        _config.maximumAngleDegrees);

    const uint32_t pulseMicroseconds =
        servoPulseMicrosecondsForAngle(
            _angleDegrees,
            _config);

    writePulse(pulseMicroseconds);
}

float Esp32ServoController::getAngle() const
{
    return _angleDegrees;
}

void Esp32ServoController::writePulse(
    uint32_t pulseMicroseconds)
{
    const float dutyFraction =
        static_cast<float>(pulseMicroseconds) *
        static_cast<float>(ServoPwmFrequencyHz) /
        MicrosecondsPerSecond;

    const uint32_t duty = static_cast<uint32_t>(
        std::lround(
            dutyFraction *
            static_cast<float>(_maximumDuty)));

    ledcWrite(_config.pin, duty);
}
