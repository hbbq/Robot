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
    constexpr uint16_t BackgroundColor = 0x0000;
    constexpr uint16_t ForegroundColor = 0xFFFF;

    constexpr int16_t ButtonY = 245;
    constexpr int16_t ButtonH = 30;

    constexpr int16_t IdleButtonX = 5;
    constexpr int16_t IdleButtonW = 70;

    constexpr int16_t AutoButtonX = 85;
    constexpr int16_t AutoButtonW = 70;

    constexpr int16_t RemoteButtonX = 165;
    constexpr int16_t RemoteButtonW = 70;
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
          120,
          145,
          85)
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
    _display.clear(
        BackgroundColor);

    _display.setTextColor(
        ForegroundColor);

    _display.setTextSize(1);

    // -------------------------
    // Connection
    // -------------------------

    _display.setCursor(8, 8);

    if (_readiness.isReady())
    {
        _display.print(
            "ROBOT: CONNECTED");
    }
    else
    {
        _display.print(
            "ROBOT: DISCONNECTED");
    }

    // -------------------------
    // Robot state
    // -------------------------

    _display.setCursor(8, 24);

    _display.print("Mode: ");
    _display.print(
        modeText(
            _robotState.mode()));

    _display.setCursor(8, 38);

    _display.print("Motion: ");
    _display.print(
        motionText(
            _robotState.motion()));

    // -------------------------
    // Joystick
    // -------------------------

    drawJoystick();

    // -------------------------
    // Mode buttons
    // -------------------------

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
    _display.drawRect(
        x,
        y,
        width,
        height,
        ForegroundColor);

    if (selected)
    {
        _display.fillRect(
            x + 3,
            y + height - 5,
            width - 6,
            3,
            ForegroundColor);
    }

    _display.setCursor(
        x + 10,
        y + 10);

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

    _display.drawCircle(
        centerX,
        centerY,
        radius,
        ForegroundColor);

    _display.drawLine(
        centerX,
        centerY - radius,
        centerX,
        centerY + radius,
        ForegroundColor);

    _display.drawLine(
        centerX - radius,
        centerY,
        centerX + radius,
        centerY,
        ForegroundColor);

    // Bara aktiv joystick när roboten
    // faktiskt rapporterar REMOTE.
    if (_robotState.mode() ==
        RobotMode::RemoteControl)
    {
        _display.fillCircle(
            _joystick.knobX(),
            _joystick.knobY(),
            16,
            ForegroundColor);
    }
    else
    {
        // liten neutral mittpunkt
        _display.fillCircle(
            centerX,
            centerY,
            5,
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