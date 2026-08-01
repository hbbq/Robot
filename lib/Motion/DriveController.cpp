#include "DriveController.h"

#include <algorithm>
#include <cmath>

DriveController::DriveController(
    IMotorController& leftMotor,
    IMotorController& rightMotor
)
    : _leftMotor(leftMotor),
      _rightMotor(rightMotor)
{
}

void DriveController::begin()
{
    _leftMotor.begin();
    _rightMotor.begin();

    stop();
}

void DriveController::forward(float speed)
{
    setDrive(std::abs(speed), 0.0f);
}

void DriveController::backward(float speed)
{
    setDrive(-std::abs(speed), 0.0f);
}

void DriveController::rotateLeft(float speed)
{
    setDrive(0.0f, -std::abs(speed));
}

void DriveController::rotateRight(float speed)
{
    setDrive(0.0f, std::abs(speed));
}

void DriveController::setDrive(float linear, float angular)
{
    _linearSpeed = std::clamp(linear, -1.0f, 1.0f);
    _angularSpeed = std::clamp(angular, -1.0f, 1.0f);

    float leftSpeed = _linearSpeed + _angularSpeed;
    float rightSpeed = _linearSpeed - _angularSpeed;

    const float largestMagnitude = std::max(
        std::abs(leftSpeed),
        std::abs(rightSpeed)
    );

    if (largestMagnitude > 1.0f)
    {
        leftSpeed /= largestMagnitude;
        rightSpeed /= largestMagnitude;
    }

    apply(leftSpeed, rightSpeed);
    updateState();
}

void DriveController::stop()
{
    _linearSpeed = 0.0f;
    _angularSpeed = 0.0f;
    _state = DriveState::Stopped;

    _leftMotor.stop();
    _rightMotor.stop();
}

DriveState DriveController::getState() const
{
    return _state;
}

RobotMotion DriveController::getMotion() const
{
    constexpr float Epsilon = 0.001f;

    const float linear =
        getLinearSpeed();

    const float angular =
        getAngularSpeed();

    if (std::abs(linear) < Epsilon &&
        std::abs(angular) < Epsilon)
    {
        return RobotMotion::Stopped;
    }

    if (std::abs(linear) < Epsilon)
    {
        return angular < 0.0f
            ? RobotMotion::TurningLeft
            : RobotMotion::TurningRight;
    }

    if (std::abs(angular) >= Epsilon)
    {
        return RobotMotion::Curve;
    }

    return linear > 0.0f
        ? RobotMotion::Forward
        : RobotMotion::Backward;
}

float DriveController::getLinearSpeed() const
{
    return _linearSpeed;
}

float DriveController::getAngularSpeed() const
{
    return _angularSpeed;
}

void DriveController::apply(float leftSpeed, float rightSpeed)
{
    _leftMotor.setSpeed(leftSpeed);
    _rightMotor.setSpeed(rightSpeed);
}

void DriveController::updateState()
{
    constexpr float epsilon = 0.001f;

    const bool hasLinear =
        std::abs(_linearSpeed) > epsilon;

    const bool hasAngular =
        std::abs(_angularSpeed) > epsilon;

    if (!hasLinear && !hasAngular)
    {
        _state = DriveState::Stopped;
    }
    else if (hasLinear && hasAngular)
    {
        _state = DriveState::Curve;
    }
    else if (_linearSpeed > 0.0f)
    {
        _state = DriveState::Forward;
    }
    else if (_linearSpeed < 0.0f)
    {
        _state = DriveState::Backward;
    }
    else if (_angularSpeed < 0.0f)
    {
        _state = DriveState::RotateLeft;
    }
    else
    {
        _state = DriveState::RotateRight;
    }
}