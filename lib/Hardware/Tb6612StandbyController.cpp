#include "Tb6612StandbyController.h"

#include <Arduino.h>

Tb6612StandbyController::Tb6612StandbyController(
    const Tb6612StandbyConfig& config)
    : _config(config)
{
}

void Tb6612StandbyController::begin()
{
    pinMode(_config.pin, OUTPUT);
    disable();
}

void Tb6612StandbyController::enable()
{
    writeEnabled(true);
}

void Tb6612StandbyController::disable()
{
    writeEnabled(false);
}

void Tb6612StandbyController::writeEnabled(bool enabled)
{
    const bool high = enabled == _config.activeHigh;
    digitalWrite(_config.pin, high ? HIGH : LOW);
}
