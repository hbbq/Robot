#pragma once

#include <cstdint>
#include <RobotMode.h>
#include <RobotMotion.h>
#include <JoystickModel.h>
#include <AutonomousBehaviorType.h>

class IDisplayDriver;
class ITouchController;
class DeviceNetworkService;
class RobotStateStore;
class ReadinessController;
class IClock;

class RemoteUiController
{
public:
    RemoteUiController(
        IDisplayDriver& display,
        ITouchController& touch,
        DeviceNetworkService& network,
        RobotStateStore& robotState,
        ReadinessController& readiness,
        IClock& clock);

    void begin();
    void update();

private:
    IDisplayDriver& _display;
    ITouchController& _touch;
    DeviceNetworkService& _network;
    RobotStateStore& _robotState;
    ReadinessController& _readiness;
    JoystickModel _joystick;
    IClock& _clock;

    bool _dirty = true;

    RobotMode _lastMode =
        RobotMode::Idle;

    bool _lastReady = false;
    
    RobotMotion _lastMotion =
        RobotMotion::Stopped;

    AutonomousBehaviorType _lastAutonomousBehavior =
        AutonomousBehaviorType::RandomDrive;

    static constexpr uint32_t DriveSendIntervalMs = 100;

    uint32_t _lastDriveSendMs = 0;

    uint32_t _idlePulseStartedMs = 0;
    bool _idlePulseBright = false;
    bool _autonomousSelectionTouchActive = false;
    uint32_t _lastAutonomousSelectionTouchMs = 0;

    void handleTouch();
    void updateState();
    void updateIdleAnimation();
    void sendDriveCommand();

    void draw();
    void drawStatusBar();
    void drawModeControls();
    void drawIdleContent();
    void drawRemoteContent();
    void drawAutonomousContent();
    void drawDisconnectedContent();
    void drawAutonomousSelection(
        int16_t y);
    void drawJoystick();
    void drawModeButton(
        int16_t x,
        int16_t y,
        int16_t width,
        int16_t height,
        const char* text,
        bool selected);

    void drawCenteredText(
        const char* text,
        int16_t y,
        uint8_t textSize,
        uint16_t color);

    void requestMode(
        RobotMode mode);

    bool isInside(
        int16_t x,
        int16_t y,
        int16_t rectX,
        int16_t rectY,
        int16_t width,
        int16_t height) const;

    bool isInsideAutoButton(
        int16_t x,
        int16_t y) const;

    bool isInsideRemoteButton(
        int16_t x,
        int16_t y) const;

    bool isInsideIdleButton(
        int16_t x,
        int16_t y) const;

    bool isInsidePreviousBehaviorControl(
        int16_t x,
        int16_t y) const;

    bool isInsideNextBehaviorControl(
        int16_t x,
        int16_t y) const;

    AutonomousBehaviorType previousAutonomousBehavior() const;
    AutonomousBehaviorType nextAutonomousBehavior() const;

    const char* modeText(
        RobotMode mode) const;

    const char* motionText(
        RobotMotion motion) const;

    const char* autonomousBehaviorText(
        AutonomousBehaviorType behavior) const;
};
