#include "FaceController.h"

#include <IDisplayDriver.h>
#include <IClock.h>
#include <IRandom.h>

FaceController::FaceController(
    IDisplayDriver& display,
    IClock& clock,
    IRandom& random)
    : _display(display),
      _clock(clock),
      _random(random)
{
}

void FaceController::begin()
{
    const uint32_t nowMs =
        _clock.millis();

    _lastBlinkMs = nowMs;
    _lastEyeMoveMs = nowMs;
    _sleepPhaseStartedMs = nowMs;

    scheduleNextBlink();
    scheduleNextEyeMove();

    _dirty = true;

    draw(
        false,
        RobotMode::Idle,
        RobotMotion::Stopped);

    _dirty = false;
}

void FaceController::update(
    bool ready,
    RobotMode mode,
    RobotMotion motion)
{
    const uint32_t nowMs =
        _clock.millis();

    const bool stateChanged =
        ready != _lastReady ||
        mode != _lastMode ||
        motion != _lastMotion;

    if (stateChanged)
    {
        const bool becameReady =
            ready && !_lastReady;

        const bool becameNotReady =
            !ready && _lastReady;

        const bool enteredIdle =
            motion == RobotMotion::Stopped &&
            _lastMotion != RobotMotion::Stopped;

        _lastReady = ready;
        _lastMode = mode;
        _lastMotion = motion;

        _dirty = true;

        if (becameNotReady)
        {
            _blinking = false;

            _idlePupilOffsetX = 0;
            _idlePupilOffsetY = 0;
        }

        if (becameReady)
        {
            _lastBlinkMs = nowMs;
            scheduleNextBlink();
        }

        if (enteredIdle)
        {
            _lastEyeMoveMs = nowMs;
            scheduleNextEyeMove();

            _idlePupilOffsetX = 0;
            _idlePupilOffsetY = 0;
        }
    }

    updateAnimations(
        ready,
        mode,
        motion,
        nowMs);

    if (!_dirty)
    {
        return;
    }

    draw(
        ready,
        mode,
        motion);

    _dirty = false;
}

void FaceController::updateAnimations(
    bool ready,
    RobotMode mode,
    RobotMotion motion,
    uint32_t nowMs)
{
    if (!ready)
    {
        if (_blinking)
        {
            _blinking = false;
            _dirty = true;
        }

        return;
    }

    if (mode == RobotMode::Idle)
    {
        constexpr uint32_t SleepBreathingIntervalMs = 1200;

        if (_blinking)
        {
            _blinking = false;
            _dirty = true;
        }

        if (nowMs - _sleepPhaseStartedMs >=
            SleepBreathingIntervalMs)
        {
            _sleepPhaseStartedMs = nowMs;
            _sleepBreathingUp = !_sleepBreathingUp;
            _dirty = true;
        }

        return;
    }

    constexpr uint32_t BlinkDurationMs = 90;

    // Blinkning oberoende av RobotActivity
    if (!_blinking &&
        nowMs - _lastBlinkMs >=
            _nextBlinkDelayMs)
    {
        _blinking = true;
        _blinkStartedMs = nowMs;

        _dirty = true;
    }

    if (_blinking &&
        nowMs - _blinkStartedMs >=
            BlinkDurationMs)
    {
        _blinking = false;
        _lastBlinkMs = nowMs;

        scheduleNextBlink();

        _dirty = true;
    }

    // Slumpmässiga ögonrörelser bara när roboten är Idle
    if (motion != RobotMotion::Stopped)
    {
        return;
    }

    if (nowMs - _lastEyeMoveMs >=
        _nextEyeMoveDelayMs)
    {
        _lastEyeMoveMs = nowMs;

        if (_random.next(0, 100) < 40)
        {
            _idlePupilOffsetX = 0;
            _idlePupilOffsetY = 0;
        }
        else
        {
            _idlePupilOffsetX =
                static_cast<int16_t>(
                    _random.next(-7, 8));

            _idlePupilOffsetY =
                static_cast<int16_t>(
                    _random.next(-4, 5));
        }

        scheduleNextEyeMove();

        _dirty = true;
    }
}

void FaceController::scheduleNextBlink()
{
    _nextBlinkDelayMs =
        static_cast<uint32_t>(
            _random.next(
                2500,
                6500));
}

void FaceController::scheduleNextEyeMove()
{
    _nextEyeMoveDelayMs =
        static_cast<uint32_t>(
            _random.next(
                800,
                2500));
}

void FaceController::draw(
    bool ready,
    RobotMode mode,
    RobotMotion motion)
{
    if (!ready)
    {
        _display.clear(BackgroundColor);
        drawNotReady();
        _display.flush();
        return;
    }

    if (mode == RobotMode::Idle)
    {
        _display.clear(SleepBackgroundColor);
        drawSleeping();
        _display.flush();
        return;
    }

    _display.clear(
        mode == RobotMode::RemoteControl
            ? RemoteBackgroundColor
            : AutonomousBackgroundColor);

    if (_blinking)
    {
        drawBlink();
        _display.flush();
        return;
    }

    if (mode == RobotMode::RemoteControl)
    {
        drawRemoteControl(
            motion);

        _display.flush();
        return;
    }

    switch (motion)
    {
        case RobotMotion::Stopped:
            drawIdle();
            break;

        case RobotMotion::Forward:
            drawMovingForward();
            break;

        case RobotMotion::Backward:
            drawMovingBackward();
            break;

        case RobotMotion::TurningLeft:
            drawTurningLeft();
            break;

        case RobotMotion::TurningRight:
            drawTurningRight();
            break;

        case RobotMotion::Curve:
            drawMovingForward();
            break;
    }

    _display.flush();
}

void FaceController::drawRemoteControl(
    RobotMotion motion)
{
    const int16_t centerY =
        _display.height() / 2;

    const int16_t leftX =
        _display.width() / 3;

    const int16_t rightX =
        _display.width() * 2 / 3;

    int16_t pupilOffsetX = 0;
    int16_t pupilOffsetY = 0;

    switch (motion)
    {
        case RobotMotion::TurningLeft:
            pupilOffsetX = -10;
            break;

        case RobotMotion::TurningRight:
            pupilOffsetX = 10;
            break;

        case RobotMotion::Backward:
            pupilOffsetY = 7;
            break;

        default:
            break;
    }

    drawEye(
        leftX,
        centerY,
        25,
        pupilOffsetX,
        pupilOffsetY);

    drawEye(
        rightX,
        centerY,
        25,
        pupilOffsetX,
        pupilOffsetY);

    // Angled brows give remote control a focused expression.
    _display.drawLine(
        leftX - 28,
        centerY - 39,
        leftX + 23,
        centerY - 31,
        EyeColor);

    _display.drawLine(
        rightX - 23,
        centerY - 31,
        rightX + 28,
        centerY - 39,
        EyeColor);

    drawFlatMouth(
        _display.width() / 2,
        centerY + 55,
        44);
}

void FaceController::drawSleeping()
{
    const int16_t centerY =
        _display.height() / 2 +
        (_sleepBreathingUp ? 2 : 0);

    const int16_t leftX =
        _display.width() / 3;

    const int16_t rightX =
        _display.width() * 2 / 3;

    constexpr int16_t HalfEyeWidth = 27;

    // Soft downward curves suggest closed, sleeping eyes.
    _display.drawLine(
        leftX - HalfEyeWidth,
        centerY - 4,
        leftX,
        centerY + 5,
        EyeColor);
    _display.drawLine(
        leftX,
        centerY + 5,
        leftX + HalfEyeWidth,
        centerY - 4,
        EyeColor);

    _display.drawLine(
        rightX - HalfEyeWidth,
        centerY - 4,
        rightX,
        centerY + 5,
        EyeColor);
    _display.drawLine(
        rightX,
        centerY + 5,
        rightX + HalfEyeWidth,
        centerY - 4,
        EyeColor);

    const int16_t zX =
        _display.width() - 48;

    const int16_t zY =
        centerY - 70;

    _display.drawLine(zX, zY, zX + 18, zY, EyeColor);
    _display.drawLine(zX + 18, zY, zX, zY + 18, EyeColor);
    _display.drawLine(zX, zY + 18, zX + 18, zY + 18, EyeColor);

    drawSmile(
        _display.width() / 2,
        centerY + 55,
        36,
        5);
}

void FaceController::drawNotReady()
{
    const int16_t centerY =
        _display.height() / 2;

    const int16_t leftX =
        _display.width() / 3;

    const int16_t rightX =
        _display.width() * 2 / 3;

    constexpr int16_t EyeWidth = 50;
    constexpr int16_t EyeHeight = 6;

    _display.fillRect(
        leftX - EyeWidth / 2,
        centerY - EyeHeight / 2,
        EyeWidth,
        EyeHeight,
        EyeColor);

    _display.fillRect(
        rightX - EyeWidth / 2,
        centerY - EyeHeight / 2,
        EyeWidth,
        EyeHeight,
        EyeColor);

    drawFlatMouth(
        _display.width() / 2,
        centerY + 52,
        42);
}

void FaceController::drawIdle()
{
    const int16_t centerY =
        _display.height() / 2;

    const int16_t leftX =
        _display.width() / 3;

    const int16_t rightX =
        _display.width() * 2 / 3;

    drawEye(
        leftX,
        centerY,
        28,
        _idlePupilOffsetX,
        _idlePupilOffsetY);

    drawEye(
        rightX,
        centerY,
        28,
        _idlePupilOffsetX,
        _idlePupilOffsetY);

    drawSmile(
        _display.width() / 2,
        centerY + 58,
        50);
}

void FaceController::drawMovingForward()
{
    const int16_t centerY =
        _display.height() / 2;

    const int16_t leftX =
        _display.width() / 3;

    const int16_t rightX =
        _display.width() * 2 / 3;

    drawEye(
        leftX,
        centerY,
        32);

    drawEye(
        rightX,
        centerY,
        32);

    drawSmile(
        _display.width() / 2,
        centerY + 62,
        58,
        10);
}

void FaceController::drawMovingBackward()
{
    const int16_t centerY =
        _display.height() / 2;

    const int16_t leftX =
        _display.width() / 3;

    const int16_t rightX =
        _display.width() * 2 / 3;

    drawEye(
        leftX,
        centerY,
        26,
        0,
        7);

    drawEye(
        rightX,
        centerY,
        26,
        0,
        7);

    drawConcernedMouth(
        _display.width() / 2,
        centerY + 58,
        46);
}

void FaceController::drawTurningLeft()
{
    const int16_t centerY =
        _display.height() / 2;

    const int16_t leftX =
        _display.width() / 3;

    const int16_t rightX =
        _display.width() * 2 / 3;

    drawEye(
        leftX,
        centerY,
        28,
        -10,
        0);

    drawEye(
        rightX,
        centerY,
        28,
        -10,
        0);

    drawSmile(
        _display.width() / 2 - 6,
        centerY + 58,
        44,
        6);
}

void FaceController::drawTurningRight()
{
    const int16_t centerY =
        _display.height() / 2;

    const int16_t leftX =
        _display.width() / 3;

    const int16_t rightX =
        _display.width() * 2 / 3;

    drawEye(
        leftX,
        centerY,
        28,
        10,
        0);

    drawEye(
        rightX,
        centerY,
        28,
        10,
        0);

    drawSmile(
        _display.width() / 2 + 6,
        centerY + 58,
        44,
        6);
}

void FaceController::drawBlink()
{
    const int16_t centerY =
        _display.height() / 2;

    const int16_t leftX =
        _display.width() / 3;

    const int16_t rightX =
        _display.width() * 2 / 3;

    constexpr int16_t EyeWidth = 50;
    constexpr int16_t EyeHeight = 7;

    _display.fillRect(
        leftX - EyeWidth / 2,
        centerY - EyeHeight / 2,
        EyeWidth,
        EyeHeight,
        EyeColor);

    _display.fillRect(
        rightX - EyeWidth / 2,
        centerY - EyeHeight / 2,
        EyeWidth,
        EyeHeight,
        EyeColor);

    drawSmile(
        _display.width() / 2,
        centerY + 58,
        48,
        7);
}

void FaceController::drawSmile(
    int16_t centerX,
    int16_t centerY,
    int16_t width,
    int16_t depth)
{
    const int16_t halfWidth = width / 2;

    _display.drawLine(
        centerX - halfWidth,
        centerY,
        centerX,
        centerY + depth,
        EyeColor);

    _display.drawLine(
        centerX,
        centerY + depth,
        centerX + halfWidth,
        centerY,
        EyeColor);
}

void FaceController::drawFlatMouth(
    int16_t centerX,
    int16_t centerY,
    int16_t width)
{
    _display.drawLine(
        centerX - width / 2,
        centerY,
        centerX + width / 2,
        centerY,
        EyeColor);
}

void FaceController::drawConcernedMouth(
    int16_t centerX,
    int16_t centerY,
    int16_t width,
    int16_t depth)
{
    const int16_t halfWidth = width / 2;

    _display.drawLine(
        centerX - halfWidth,
        centerY + depth,
        centerX,
        centerY,
        EyeColor);

    _display.drawLine(
        centerX,
        centerY,
        centerX + halfWidth,
        centerY + depth,
        EyeColor);
}

void FaceController::drawEye(
    int16_t centerX,
    int16_t centerY,
    int16_t radius,
    int16_t pupilOffsetX,
    int16_t pupilOffsetY)
{
    _display.fillCircle(
        centerX,
        centerY,
        radius,
        EyeColor);

    _display.fillCircle(
        centerX + pupilOffsetX,
        centerY + pupilOffsetY,
        radius / 3,
        PupilColor);
}
