#pragma once

#include <cstdint>

#include <RobotActivity.h>
#include <RobotMode.h>
#include <RobotMotion.h>

class IDisplayDriver;
class IClock;
class IRandom;

class FaceController
{
public:
    FaceController(
        IDisplayDriver& display,
        IClock& clock,
        IRandom& random);

    void begin();

    void update(
        bool ready,
        RobotMode mode,
        RobotMotion motion);

private:
    IDisplayDriver& _display;
    IClock& _clock;
    IRandom& _random;

    bool _lastReady = false;

    RobotMode _lastMode =
        RobotMode::Idle;

    RobotMotion _lastMotion =
        RobotMotion::Stopped;

    bool _dirty = true;

    bool _blinking = false;

    uint32_t _blinkStartedMs = 0;
    uint32_t _lastBlinkMs = 0;
    uint32_t _nextBlinkDelayMs = 0;

    uint32_t _lastEyeMoveMs = 0;
    uint32_t _nextEyeMoveDelayMs = 0;

    uint32_t _sleepPhaseStartedMs = 0;
    bool _sleepBreathingUp = false;

    int16_t _idlePupilOffsetX = 0;
    int16_t _idlePupilOffsetY = 0;

    void updateAnimations(
        bool ready,
        RobotMode mode,
        RobotMotion motion,
        uint32_t nowMs);

    void scheduleNextBlink();
    void scheduleNextEyeMove();

    void draw(
        bool ready,
        RobotMode mode,
        RobotMotion motion);

    void drawRemoteControl(
        RobotMotion motion);

    void drawNotReady();
    void drawSleeping();
    void drawIdle();
    void drawMovingForward();
    void drawMovingBackward();
    void drawTurningLeft();
    void drawTurningRight();
    void drawBlink();

    void drawEye(
        int16_t centerX,
        int16_t centerY,
        int16_t radius,
        int16_t pupilOffsetX = 0,
        int16_t pupilOffsetY = 0);

    static constexpr uint16_t BackgroundColor =
        0x0000;

    static constexpr uint16_t SleepBackgroundColor =
        0x0863;

    static constexpr uint16_t AutonomousBackgroundColor =
        0x08C3;

    static constexpr uint16_t RemoteBackgroundColor =
        0x20C3;

    static constexpr uint16_t EyeColor =
        0xFFFF;

    static constexpr uint16_t PupilColor =
        0x0000;
};
