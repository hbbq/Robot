#include "RemoteUiController.h"

#include <DeviceNetworkService.h>
#include <IDisplayDriver.h>
#include <ITouchController.h>
#include <ReadinessController.h>
#include <RobotStateStore.h>
#include <FrontScanMeasurementStore.h>
#include <JoystickModel.h>
#include <IClock.h>

#include <cstring>
#include <cstdio>

namespace
{
    // Low-saturation RGB565 palette for a calmer control interface.
    constexpr uint16_t BackgroundColor = 0x10A3;
    constexpr uint16_t PanelColor = 0x1905;
    constexpr uint16_t BorderColor = 0x29C8;
    constexpr uint16_t ForegroundColor = 0xDF1D;
    constexpr uint16_t MutedColor = 0x7411;
    constexpr uint16_t AccentColor = 0x5CF5;
    constexpr uint16_t SuccessColor = 0x6CF0;
    constexpr uint16_t WarningColor = 0xBC2E;
    constexpr uint16_t IdlePulseDimColor = 0x3AD0;

    constexpr int16_t StatusBarHeight = 44;
    constexpr int16_t ContentCenterY = 139;

    constexpr int16_t ButtonY = 244;
    constexpr int16_t ButtonH = 36;

    constexpr int16_t IdleButtonX = 7;
    constexpr int16_t IdleButtonW = 70;

    constexpr int16_t AutoButtonX = 86;
    constexpr int16_t AutoButtonW = 70;

    constexpr int16_t RemoteButtonX = 165;
    constexpr int16_t RemoteButtonW = 68;

    constexpr int16_t JoystickRadius = 48;
    constexpr int16_t JoystickCenterX = 120;
    constexpr int16_t JoystickCenterY = 162;

    constexpr uint32_t IdlePulseIntervalMs = 700;
    constexpr int16_t BehaviorControlY = 70;
    constexpr int16_t BehaviorControlH = 55;
    constexpr int16_t PreviousBehaviorControlX = 12;
    constexpr int16_t NextBehaviorControlX = 180;
    constexpr int16_t BehaviorControlW = 48;
}

RemoteUiController::RemoteUiController(
    IDisplayDriver& display,
    ITouchController& touch,
    DeviceNetworkService& network,
    RobotStateStore& robotState,
    FrontScanMeasurementStore& frontScanMeasurement,
    ReadinessController& readiness,
    IClock& clock,
    const JoystickConfig& joystickConfig,
    const DriveCommandTransmissionConfig& transmissionConfig,
    const RemoteUiConfig& uiConfig)
    : _display(display),
      _touch(touch),
      _network(network),
      _robotState(robotState),
      _frontScanMeasurement(frontScanMeasurement),
      _readiness(readiness),
      _clock(clock),
      _transmissionConfig(transmissionConfig),
      _uiConfig(uiConfig),
      _joystick(
          JoystickCenterX,
          JoystickCenterY,
          JoystickRadius,
          joystickConfig)
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

    _lastAutonomousBehavior =
        _robotState.autonomousBehavior();

    _lastMeasurementRevision =
        _frontScanMeasurement.revision();

    _lastMeasurementFreshnessMask =
        _frontScanMeasurement.freshnessMask(_clock.millis());

    _idlePulseStartedMs =
        _clock.millis();

    _dirty = true;

    draw();

    _dirty = false;
}

void RemoteUiController::update()
{
    handleTouch();
    updateState();
    updateIdleAnimation();
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

    const uint32_t nowMs =
        _clock.millis();

    if (!touched)
    {
        if (_autonomousSelectionTouchActive &&
            nowMs - _lastAutonomousSelectionTouchMs >=
                _uiConfig.selectionReleaseMs)
        {
            _autonomousSelectionTouchActive = false;
        }

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

    if (_autonomousSelectionTouchActive)
    {
        _lastAutonomousSelectionTouchMs = nowMs;
    }

    if (_robotState.mode() == RobotMode::Autonomous &&
        isInsidePreviousBehaviorControl(x, y))
    {
        if (!_autonomousSelectionTouchActive)
        {
            _autonomousSelectionTouchActive = true;
            _lastAutonomousSelectionTouchMs = nowMs;
            _network.sendSetAutonomousBehavior(
                previousAutonomousBehavior());
        }

        return;
    }

    if (_robotState.mode() == RobotMode::Autonomous &&
        isInsideNextBehaviorControl(x, y))
    {
        if (!_autonomousSelectionTouchActive)
        {
            _autonomousSelectionTouchActive = true;
            _lastAutonomousSelectionTouchMs = nowMs;
            _network.sendSetAutonomousBehavior(
                nextAutonomousBehavior());
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

    const AutonomousBehaviorType autonomousBehavior =
        _robotState.autonomousBehavior();

    const uint32_t measurementRevision =
        _frontScanMeasurement.revision();

    const uint8_t measurementFreshnessMask =
        _frontScanMeasurement.freshnessMask(_clock.millis());

    if (ready == _lastReady &&
        mode == _lastMode &&
        motion == _lastMotion &&
        autonomousBehavior == _lastAutonomousBehavior &&
        measurementRevision == _lastMeasurementRevision &&
        measurementFreshnessMask == _lastMeasurementFreshnessMask)
    {
        return;
    }

    _lastReady = ready;
    _lastMode = mode;
    _lastMotion = motion;
    _lastAutonomousBehavior = autonomousBehavior;
    _lastMeasurementRevision = measurementRevision;
    _lastMeasurementFreshnessMask = measurementFreshnessMask;

    if (mode != RobotMode::RemoteControl &&
        _joystick.active())
    {
        _joystick.release();
    }

    if (mode == RobotMode::Idle)
    {
        _idlePulseStartedMs = _clock.millis();
        _idlePulseBright = false;
    }

    _dirty = true;
}

void RemoteUiController::updateIdleAnimation()
{
    if (!_readiness.isReady() ||
        _robotState.mode() != RobotMode::Idle)
    {
        return;
    }

    const uint32_t nowMs = _clock.millis();

    if (nowMs - _idlePulseStartedMs <
        IdlePulseIntervalMs)
    {
        return;
    }

    _idlePulseStartedMs = nowMs;
    _idlePulseBright = !_idlePulseBright;
    _dirty = true;
}

void RemoteUiController::sendDriveCommand()
{
    if (!_joystick.active() ||
        _robotState.mode() !=
            RobotMode::RemoteControl)
    {
        return;
    }

    const uint32_t nowMs =
        _clock.millis();

    if (nowMs - _lastDriveSendMs <
        _transmissionConfig.sendIntervalMs)
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
    _display.clear(BackgroundColor);

    drawStatusBar();

    if (!_readiness.isReady())
    {
        drawDisconnectedContent();
    }
    else
    {
        drawFrontScanDiagnostic();

        switch (_robotState.mode())
        {
            case RobotMode::Idle:
                drawIdleContent();
                break;

            case RobotMode::Autonomous:
                drawAutonomousContent();
                break;

            case RobotMode::RemoteControl:
                drawRemoteContent();
                break;

            case RobotMode::Calibration:
                drawCalibrationContent();
                break;
        }
    }

    drawModeControls();

    _display.flush();
}

void RemoteUiController::drawFrontScanDiagnostic()
{
    if (!_frontScanMeasurement.hasMeasurement())
    {
        drawCenteredText("L:M--- C:M--- R:M---", 49, 1, MutedColor);
        return;
    }

    const FrontScanMeasurement& measurement =
        _frontScanMeasurement.measurement();
    const uint32_t nowMs = _clock.millis();

    const auto formatSector = [this, nowMs](
        char label,
        const FrontScanSectorMeasurement& sector,
        char* output,
        size_t outputSize)
    {
        const char quality = !sector.hasSample
            ? 'M'
            : !sector.valid
                ? 'I'
                : _frontScanMeasurement.isFresh(sector, nowMs)
                    ? 'V'
                    : 'S';

        if (!sector.hasSample)
        {
            std::snprintf(output, outputSize, "%c:%c---", label, quality);
        }
        else
        {
            std::snprintf(
                output,
                outputSize,
                "%c:%c%u",
                label,
                quality,
                sector.distanceMillimeters);
        }
    };

    char left[10]{};
    char center[10]{};
    char right[10]{};
    char line[32]{};
    formatSector('L', measurement.left, left, sizeof(left));
    formatSector('C', measurement.center, center, sizeof(center));
    formatSector('R', measurement.right, right, sizeof(right));
    std::snprintf(line, sizeof(line), "%s %s %s", left, center, right);

    drawCenteredText(line, 49, 1, ForegroundColor);
}

void RemoteUiController::drawStatusBar()
{
    const bool ready = _readiness.isReady();

    _display.fillRect(
        0,
        0,
        _display.width(),
        StatusBarHeight,
        PanelColor);

    _display.fillRect(
        0,
        StatusBarHeight - 1,
        _display.width(),
        1,
        BorderColor);

    if (!ready)
    {
        drawCenteredText(
            "DISCONNECTED",
            15,
            2,
            WarningColor);
        return;
    }

    const char* mode =
        modeText(_robotState.mode());

    const char* motion =
        motionText(_robotState.motion());

    constexpr int16_t SeparatorWidth = 14;

    const int16_t modeWidth =
        static_cast<int16_t>(std::strlen(mode) * 6);

    const int16_t motionWidth =
        static_cast<int16_t>(std::strlen(motion) * 6);

    const int16_t startX =
        (_display.width() -
         modeWidth - SeparatorWidth - motionWidth) / 2;

    _display.setTextSize(1);
    _display.setTextColor(ForegroundColor);
    _display.setCursor(startX, 18);
    _display.print(mode);

    const int16_t separatorX =
        startX + modeWidth + SeparatorWidth / 2;

    _display.fillCircle(
        separatorX,
        21,
        2,
        AccentColor);

    _display.setCursor(
        startX + modeWidth + SeparatorWidth,
        18);
    _display.print(motion);
}

void RemoteUiController::drawModeControls()
{
    _display.fillRect(
        0,
        ButtonY,
        _display.width(),
        1,
        BorderColor);

    drawModeButton(
        IdleButtonX,
        ButtonY,
        IdleButtonW,
        ButtonH,
        "IDLE",
        _robotState.mode() == RobotMode::Idle);

    drawModeButton(
        AutoButtonX,
        ButtonY,
        AutoButtonW,
        ButtonH,
        "AUTO",
        _robotState.mode() == RobotMode::Autonomous);

    drawModeButton(
        RemoteButtonX,
        ButtonY,
        RemoteButtonW,
        ButtonH,
        "REMOTE",
        _robotState.mode() == RobotMode::RemoteControl);
}

void RemoteUiController::drawIdleContent()
{
    drawCenteredText(
        "Zzz",
        ContentCenterY - 24,
        4,
        _idlePulseBright
            ? AccentColor
            : IdlePulseDimColor);

    drawCenteredText(
        "sleeping",
        ContentCenterY + 34,
        1,
        MutedColor);

    drawAutonomousSelection(
        ContentCenterY + 67);
}

void RemoteUiController::drawRemoteContent()
{
    drawAutonomousSelection(65);
    drawJoystick();
}

void RemoteUiController::drawAutonomousContent()
{
    drawCenteredText(
        "AUTONOMOUS",
        ContentCenterY - 66,
        1,
        MutedColor);

    drawCenteredText(
        autonomousBehaviorText(
            _robotState.autonomousBehavior()),
        ContentCenterY - 50,
        1,
        AccentColor);

    _display.drawCircle(
        PreviousBehaviorControlX + BehaviorControlW / 2,
        BehaviorControlY + BehaviorControlH / 2,
        15,
        BorderColor);

    _display.drawCircle(
        NextBehaviorControlX + BehaviorControlW / 2,
        BehaviorControlY + BehaviorControlH / 2,
        15,
        BorderColor);

    _display.setTextSize(1);
    _display.setTextColor(ForegroundColor);
    _display.setCursor(
        PreviousBehaviorControlX + 21,
        BehaviorControlY + 22);
    _display.print("<");

    _display.setCursor(
        NextBehaviorControlX + 21,
        BehaviorControlY + 22);
    _display.print(">");

    const char* motion =
        motionText(_robotState.motion());

    const uint8_t textSize =
        std::strlen(motion) > 10 ? 2 : 3;

    drawCenteredText(
        motion,
        ContentCenterY,
        textSize,
        ForegroundColor);
}

void RemoteUiController::drawCalibrationContent()
{
    drawCenteredText(
        "CALIBRATION",
        ContentCenterY - 28,
        2,
        WarningColor);

    drawCenteredText(
        motionText(_robotState.motion()),
        ContentCenterY + 14,
        2,
        ForegroundColor);
}

void RemoteUiController::drawDisconnectedContent()
{
    drawCenteredText(
        "Waiting for robot",
        ContentCenterY,
        1,
        MutedColor);
}

void RemoteUiController::drawAutonomousSelection(
    int16_t y)
{
    drawCenteredText(
        "SELECTED AUTO",
        y,
        1,
        MutedColor);

    drawCenteredText(
        autonomousBehaviorText(
            _robotState.autonomousBehavior()),
        y + 14,
        1,
        AccentColor);
}

void RemoteUiController::drawModeButton(
    int16_t x,
    int16_t y,
    int16_t width,
    int16_t height,
    const char* text,
    bool selected)
{
    _display.setTextSize(1);
    _display.setTextColor(
        selected ? ForegroundColor : MutedColor);

    const int16_t textWidth =
        static_cast<int16_t>(
            std::strlen(text) * 6);

    _display.setCursor(
        x + (width - textWidth) / 2,
        y + 12);
    _display.print(text);

    if (selected)
    {
        _display.fillRect(
            x + 12,
            y + height - 5,
            width - 24,
            2,
            AccentColor);
    }
}

void RemoteUiController::drawCenteredText(
    const char* text,
    int16_t y,
    uint8_t textSize,
    uint16_t color)
{
    const int16_t textWidth =
        static_cast<int16_t>(
            std::strlen(text) *
            6 * textSize);

    _display.setTextSize(textSize);
    _display.setTextColor(color);
    _display.setCursor(
        (_display.width() - textWidth) / 2,
        y);
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
        BorderColor);

    _display.drawCircle(
        centerX,
        centerY,
        radius - 1,
        BorderColor);

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

    _display.fillCircle(
        _joystick.knobX(),
        _joystick.knobY(),
        15,
        AccentColor);

    _display.fillCircle(
        _joystick.knobX(),
        _joystick.knobY(),
        4,
        ForegroundColor);
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

bool RemoteUiController::isInsidePreviousBehaviorControl(
    int16_t x,
    int16_t y) const
{
    return isInside(
        x,
        y,
        PreviousBehaviorControlX,
        BehaviorControlY,
        BehaviorControlW,
        BehaviorControlH);
}

bool RemoteUiController::isInsideNextBehaviorControl(
    int16_t x,
    int16_t y) const
{
    return isInside(
        x,
        y,
        NextBehaviorControlX,
        BehaviorControlY,
        BehaviorControlW,
        BehaviorControlH);
}

AutonomousBehaviorType
RemoteUiController::previousAutonomousBehavior() const
{
    switch (_robotState.autonomousBehavior())
    {
        case AutonomousBehaviorType::RandomDrive:
            return AutonomousBehaviorType::Dance;

        case AutonomousBehaviorType::Explore:
            return AutonomousBehaviorType::RandomDrive;

        case AutonomousBehaviorType::Dance:
            return AutonomousBehaviorType::Explore;
    }

    return AutonomousBehaviorType::RandomDrive;
}

AutonomousBehaviorType
RemoteUiController::nextAutonomousBehavior() const
{
    switch (_robotState.autonomousBehavior())
    {
        case AutonomousBehaviorType::RandomDrive:
            return AutonomousBehaviorType::Explore;

        case AutonomousBehaviorType::Explore:
            return AutonomousBehaviorType::Dance;

        case AutonomousBehaviorType::Dance:
            return AutonomousBehaviorType::RandomDrive;
    }

    return AutonomousBehaviorType::RandomDrive;
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

        case RobotMode::Calibration:
            return "CAL";
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

const char* RemoteUiController::autonomousBehaviorText(
    AutonomousBehaviorType behavior) const
{
    switch (behavior)
    {
        case AutonomousBehaviorType::RandomDrive:
            return "RANDOM DRIVE";

        case AutonomousBehaviorType::Explore:
            return "EXPLORE";

        case AutonomousBehaviorType::Dance:
            return "DANCE";
    }

    return "?";
}
