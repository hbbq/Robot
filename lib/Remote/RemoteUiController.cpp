#include "RemoteUiController.h"

#include <DeviceNetworkService.h>
#include <IDisplayDriver.h>
#include <ITouchController.h>
#include <ReadinessController.h>
#include <RobotStateStore.h>
#include <JoystickModel.h>
#include <IClock.h>

namespace
{
    // Low-saturation RGB565 palette for a calmer control interface.
    constexpr uint16_t BackgroundColor = 0x10A3;
    constexpr uint16_t PanelColor = 0x1905;
    constexpr uint16_t PanelAltColor = 0x10C4;
    constexpr uint16_t BorderColor = 0x29C8;
    constexpr uint16_t ForegroundColor = 0xDF1D;
    constexpr uint16_t MutedColor = 0x7411;
    constexpr uint16_t AccentColor = 0x5CF5;
    constexpr uint16_t SuccessColor = 0x6CF0;
    constexpr uint16_t WarningColor = 0xBC2E;

    constexpr int16_t ButtonY = 244;
    constexpr int16_t ButtonH = 32;

    constexpr int16_t IdleButtonX = 7;
    constexpr int16_t IdleButtonW = 70;

    constexpr int16_t AutoButtonX = 86;
    constexpr int16_t AutoButtonW = 70;

    constexpr int16_t RemoteButtonX = 165;
    constexpr int16_t RemoteButtonW = 68;

    constexpr int16_t JoystickRadius = 48;
    constexpr int16_t JoystickCenterX = 120;
    constexpr int16_t JoystickCenterY = 162;
}

RemoteUiController::RemoteUiController(
    IDisplayDriver& display,
    ITouchController& touch,
    DeviceNetworkService& network,
    RobotStateStore& robotState,
    ReadinessController& readiness,
    IClock& clock)
    : _display(display),
      _touch(touch),
      _network(network),
      _robotState(robotState),
      _readiness(readiness),
      _clock(clock),
      _joystick(
          JoystickCenterX,
          JoystickCenterY,
          JoystickRadius)
{
}

void RemoteUiController::begin()
{
    _lastReady =
        _readiness.isReady();

    _lastMode =
        _robotState.mode();

    _lastMotion =
        _robotState.motion();

    _dirty = true;

    draw();

    _dirty = false;
}

void RemoteUiController::update()
{
    handleTouch();
    updateState();
    sendDriveCommand();

    if (_dirty)
    {
        draw();
        _dirty = false;
    }
}

void RemoteUiController::handleTouch()
{
    if (!_readiness.isReady())
    {
        if (_joystick.active())
        {
            _joystick.release();
            _dirty = true;
        }

        return;
    }

    int16_t x = 0;
    int16_t y = 0;

    const bool touched =
        _touch.newTouch(x, y);

    if (!touched)
    {
        if (_joystick.active())
        {
            _joystick.release();

            _network.sendDriveCommand(
                0.0f,
                0.0f);

            _dirty = true;
        }

        return;
    }

    if (isInsideIdleButton(x, y))
    {
        _joystick.release();

        _network.sendDriveCommand(
            0.0f,
            0.0f);

        _network.sendSetRobotMode(
            RobotMode::Idle);

        return;
    }

    if (isInsideAutoButton(x, y))
    {
        _joystick.release();

        _network.sendDriveCommand(
            0.0f,
            0.0f);

        _network.sendSetRobotMode(
            RobotMode::Autonomous);

        return;
    }

    if (isInsideRemoteButton(x, y))
    {
        _network.sendSetRobotMode(
            RobotMode::RemoteControl);

        return;
    }

    if (_robotState.mode() ==
        RobotMode::RemoteControl)
    {
        _joystick.setTouch(x, y);
        _dirty = true;
    }
}

void RemoteUiController::updateState()
{
    const bool ready =
        _readiness.isReady();

    const RobotMode mode =
        _robotState.mode();

    const RobotMotion motion =
        _robotState.motion();

    if (ready == _lastReady &&
        mode == _lastMode &&
        motion == _lastMotion)
    {
        return;
    }

    _lastReady = ready;
    _lastMode = mode;
    _lastMotion = motion;

    _dirty = true;
}

void RemoteUiController::sendDriveCommand()
{
    if (!_joystick.active())
    {
        return;
    }

    const uint32_t nowMs =
        _clock.millis();

    if (nowMs - _lastDriveSendMs <
        DriveSendIntervalMs)
    {
        return;
    }

    _lastDriveSendMs = nowMs;

    _network.sendDriveCommand(
        _joystick.linear(),
        _joystick.angular());
}

void RemoteUiController::requestMode(
    RobotMode mode)
{
    _network.sendSetRobotMode(mode);
}

void RemoteUiController::draw()
{
    const bool ready =
        _readiness.isReady();

    _display.clear(
        BackgroundColor);

    const uint16_t statusDotColor =
        ready ? SuccessColor : WarningColor;

    _display.fillRect(
        8,
        8,
        224,
        30,
        PanelColor);

    _display.drawRect(
        8,
        8,
        224,
        30,
        BorderColor);

    _display.fillCircle(
        20,
        23,
        4,
        statusDotColor);

    _display.setTextColor(
        MutedColor);
    _display.setTextSize(1);
    _display.setCursor(31, 18);
    _display.print("ROBOT");

    _display.setTextColor(
        ForegroundColor);
    _display.setCursor(
        ready ? 181 : 175,
        18);
    _display.print(
        ready ? "ONLINE" : "OFFLINE");

    _display.fillRect(
        8,
        46,
        224,
        52,
        PanelColor);

    _display.setTextColor(
        ForegroundColor);
    _display.setCursor(18, 56);
    _display.print("MODE: ");
    _display.print(
        modeText(
            _robotState.mode()));

    _display.setCursor(18, 74);
    _display.print("MOTION: ");
    _display.print(
        motionText(
            _robotState.motion()));

    _display.fillRect(
        8,
        108,
        224,
        122,
        PanelAltColor);

    drawJoystick();

    drawModeButton(
        IdleButtonX,
        ButtonY,
        IdleButtonW,
        ButtonH,
        "IDLE",
        _robotState.mode() ==
            RobotMode::Idle);

    drawModeButton(
        AutoButtonX,
        ButtonY,
        AutoButtonW,
        ButtonH,
        "AUTO",
        _robotState.mode() ==
            RobotMode::Autonomous);

    drawModeButton(
        RemoteButtonX,
        ButtonY,
        RemoteButtonW,
        ButtonH,
        "REMOTE",
        _robotState.mode() ==
            RobotMode::RemoteControl);

    _display.flush();
}

void RemoteUiController::drawModeButton(
    int16_t x,
    int16_t y,
    int16_t width,
    int16_t height,
    const char* text,
    bool selected)
{
    if (selected)
    {
        _display.fillRect(
            x,
            y,
            width,
            height,
            AccentColor);

        _display.setTextColor(
            BackgroundColor);
    }
    else
    {
        _display.fillRect(
            x,
            y,
            width,
            height,
            PanelColor);

        _display.drawRect(
            x,
            y,
            width,
            height,
            BorderColor);

        _display.setTextColor(
            ForegroundColor);
    }

    _display.setCursor(
        x + 11,
        y + 11);
    _display.print(text);
}

void RemoteUiController::drawJoystick()
{
    const int16_t centerX =
        _joystick.centerX();

    const int16_t centerY =
        _joystick.centerY();

    const int16_t radius =
        _joystick.radius();

    _display.drawRect(
        centerX - radius,
        centerY - radius,
        radius * 2,
        radius * 2,
        BorderColor);

    _display.fillRect(
        centerX - radius + 4,
        centerY - radius + 4,
        (radius * 2) - 8,
        (radius * 2) - 8,
        PanelColor);

    _display.drawLine(
        centerX,
        centerY - radius,
        centerX,
        centerY + radius,
        MutedColor);

    _display.drawLine(
        centerX - radius,
        centerY,
        centerX + radius,
        centerY,
        MutedColor);

    if (_robotState.mode() ==
        RobotMode::RemoteControl)
    {
        _display.fillCircle(
            _joystick.knobX(),
            _joystick.knobY(),
            15,
            AccentColor);
    }
    else
    {
        _display.fillCircle(
            centerX,
            centerY,
            9,
            ForegroundColor);
    }
}

bool RemoteUiController::isInside(
    int16_t x,
    int16_t y,
    int16_t rectX,
    int16_t rectY,
    int16_t width,
    int16_t height) const
{
    return
        x >= rectX &&
        x < rectX + width &&
        y >= rectY &&
        y < rectY + height;
}

bool RemoteUiController::isInsideAutoButton(
    int16_t x,
    int16_t y) const
{
    return isInside(
        x, y,
        AutoButtonX,
        ButtonY,
        AutoButtonW,
        ButtonH);
}

bool RemoteUiController::isInsideRemoteButton(
    int16_t x,
    int16_t y) const
{
    return isInside(
        x, y,
        RemoteButtonX,
        ButtonY,
        RemoteButtonW,
        ButtonH);
}

bool RemoteUiController::isInsideIdleButton(
    int16_t x,
    int16_t y) const
{
    return isInside(
        x, y,
        IdleButtonX,
        ButtonY,
        IdleButtonW,
        ButtonH);
}

const char* RemoteUiController::modeText(
    RobotMode mode) const
{
    switch (mode)
    {
        case RobotMode::Idle:
            return "IDLE";

        case RobotMode::Autonomous:
            return "AUTO";

        case RobotMode::RemoteControl:
            return "REMOTE";
    }

    return "?";
}

const char* RemoteUiController::motionText(
    RobotMotion motion) const
{
    switch (motion)
    {
        case RobotMotion::Stopped:
            return "Stopped";

        case RobotMotion::Forward:
            return "Forward";

        case RobotMotion::Backward:
            return "Backward";

        case RobotMotion::TurningLeft:
            return "Turning left";

        case RobotMotion::TurningRight:
            return "Turning right";

        case RobotMotion::Curve:
            return "Curving";
    }

    return "?";
}
