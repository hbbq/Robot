#ifdef HAS_DISPLAY_169

#include "Cst816TouchDriver.h"

#include <Arduino.h>
#include <Wire.h>

#include <TouchDrvCSTXXX.hpp>

namespace
{
    constexpr uint8_t I2cSdaPin = 8;
    constexpr uint8_t I2cSclPin = 7;
    constexpr uint8_t TouchIrqPin = 11;
}

volatile bool
Cst816TouchDriver::_interruptTriggered = false;

Cst816TouchDriver::Cst816TouchDriver()
{
    _touch = new TouchDrvCSTXXX();
}

Cst816TouchDriver::~Cst816TouchDriver()
{
    delete _touch;
}

bool Cst816TouchDriver::begin()
{
    Wire.begin(
        I2cSdaPin,
        I2cSclPin);

    const bool initialized =
        _touch->begin(
            Wire,
            CST816_SLAVE_ADDRESS,
            I2cSdaPin,
            I2cSclPin);

    if (!initialized)
    {
        return false;
    }

    pinMode(
        TouchIrqPin,
        INPUT);

    attachInterrupt(
        TouchIrqPin,
        interruptHandler,
        FALLING);

    return true;
}

bool Cst816TouchDriver::newTouch(
    int16_t& x,
    int16_t& y)
{
    if (!_interruptTriggered)
    {
        _pressed = false;
        return false;
    }

    _interruptTriggered = false;

    int16_t xPoints[5]{};
    int16_t yPoints[5]{};

    const uint8_t touched =
        _touch->getPoint(
            xPoints,
            yPoints,
            _touch->getSupportTouchPoint());

    if (touched == 0)
    {
        _pressed = false;
        return false;
    }

    x = xPoints[0];
    y = yPoints[0];

    _pressed = true;

    return true;
}

bool Cst816TouchDriver::isPressed() const
{
    return _pressed;
}

void IRAM_ATTR
Cst816TouchDriver::interruptHandler()
{
    _interruptTriggered = true;
}

#endif