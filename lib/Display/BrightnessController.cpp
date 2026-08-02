#include "BrightnessController.h"

#include <IClock.h>
#include <IDisplayDriver.h>

BrightnessController::BrightnessController(
    IDisplayDriver& display,
    IClock& clock,
    const BrightnessControllerConfig& config)
    : _display(display),
      _clock(clock),
      _config(config)
{
}

void BrightnessController::begin()
{
    _currentPercent = _config.minimumPercent;
    _startPercent = _currentPercent;
    _targetPercent = _currentPercent;
    _fadeStartedMs = _clock.millis();

    _display.setBrightness(_currentPercent);
}

void BrightnessController::setActive(bool active)
{
    const uint8_t requestedPercent =
        active
            ? _config.maximumPercent
            : _config.minimumPercent;

    if (requestedPercent == _targetPercent)
    {
        return;
    }

    _startPercent = _currentPercent;
    _targetPercent = requestedPercent;
    _fadeStartedMs = _clock.millis();
}

void BrightnessController::update()
{
    if (_currentPercent == _targetPercent)
    {
        return;
    }

    const uint32_t elapsedMs =
        _clock.millis() - _fadeStartedMs;

    uint8_t nextPercent = _targetPercent;

    if (_config.fadeDurationMs > 0 &&
        elapsedMs < _config.fadeDurationMs)
    {
        const int32_t difference =
            static_cast<int32_t>(_targetPercent) -
            static_cast<int32_t>(_startPercent);

        nextPercent = static_cast<uint8_t>(
            static_cast<int32_t>(_startPercent) +
            difference * static_cast<int32_t>(elapsedMs) /
                static_cast<int32_t>(_config.fadeDurationMs));
    }

    if (nextPercent == _currentPercent)
    {
        return;
    }

    _currentPercent = nextPercent;
    _display.setBrightness(_currentPercent);
}
