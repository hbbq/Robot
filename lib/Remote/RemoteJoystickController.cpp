#include "RemoteJoystickController.h"

#include <cmath>
#include <algorithm>

#include <IDisplayDriver.h>
#include <ITouchController.h>
#include <DeviceNetworkService.h>
#include <IClock.h>

namespace
{
    constexpr uint16_t BackgroundColor = 0x0000;
    constexpr uint16_t OutlineColor = 0xFFFF;
    constexpr uint16_t KnobColor = 0xFFFF;
}

RemoteJoystickController::RemoteJoystickController(
    IDisplayDriver& display,
    ITouchController& touch,
    DeviceNetworkService& network,
    IClock& clock)
    : _display(display),
      _touch(touch),
      _network(network),
      _clock(clock)
{
}

void RemoteJoystickController::begin()
{
    _active = false;

    _linear = 0.0f;
    _angular = 0.0f;

    _knobX = CenterX;
    _knobY = CenterY;

    draw();
}

void RemoteJoystickController::update()
{
    updateTouch();

    if (_active)
    {
        sendCommand();
    }
}

void RemoteJoystickController::updateTouch()
{
    int16_t x;
    int16_t y;

    if (_touch.newTouch(x, y))
    {
        setPosition(x, y);
        return;
    }

    if (_active)
    {
        release();
    }
}

void RemoteJoystickController::setPosition(
    int16_t x,
    int16_t y)
{
    float dx =
        static_cast<float>(x - CenterX);

    float dy =
        static_cast<float>(y - CenterY);

    const float distance =
        std::sqrt(dx * dx + dy * dy);

    // Begränsa knoppen till joystickens cirkel
    if (distance > Radius)
    {
        const float scale =
            static_cast<float>(Radius) /
            distance;

        dx *= scale;
        dy *= scale;
    }

    _knobX =
        CenterX +
        static_cast<int16_t>(dx);

    _knobY =
        CenterY +
        static_cast<int16_t>(dy);

    // Display-Y ökar nedåt.
    // Framåt ska vara positiv linear.
    _linear =
        -dy /
        static_cast<float>(Radius);

    // Höger ska vara positiv angular.
    _angular =
        dx /
        static_cast<float>(Radius);

    if (std::abs(_linear) < DeadZone)
    {
        _linear = 0.0f;
    }

    if (std::abs(_angular) < DeadZone)
    {
        _angular = 0.0f;
    }

    _linear =
        std::clamp(
            _linear,
            -1.0f,
            1.0f);

    _angular =
        std::clamp(
            _angular,
            -1.0f,
            1.0f);

    _active = true;

    draw();
}

void RemoteJoystickController::release()
{
    _active = false;

    _linear = 0.0f;
    _angular = 0.0f;

    _knobX = CenterX;
    _knobY = CenterY;

    // Viktigt: stoppa direkt.
    sendCommand(true);

    draw();
}

void RemoteJoystickController::sendCommand(
    bool force)
{
    const uint32_t nowMs =
        _clock.millis();

    if (!force &&
        nowMs - _lastSendMs < SendIntervalMs)
    {
        return;
    }

    _lastSendMs = nowMs;

    _network.sendDriveCommand(
        _linear,
        _angular);
}

void RemoteJoystickController::draw()
{
    _display.clear(
        BackgroundColor);

    _display.drawCircle(
        CenterX,
        CenterY,
        Radius,
        OutlineColor);

    // Mittlinjer
    _display.drawLine(
        CenterX,
        CenterY - Radius,
        CenterX,
        CenterY + Radius,
        OutlineColor);

    _display.drawLine(
        CenterX - Radius,
        CenterY,
        CenterX + Radius,
        CenterY,
        OutlineColor);

    _display.fillCircle(
        _knobX,
        _knobY,
        KnobRadius,
        KnobColor);

    _display.flush();
}