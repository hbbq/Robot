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
    _display.clear(BackgroundColor);

    if (!ready)
    {
        drawNotReady();
        _display.flush();
        return;
    }

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
    _display.clear(0x6666);

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