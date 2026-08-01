#pragma once

#include "IDriveController.h"
#include "../Hardware/IMotorController.h"
#include <RobotMotion.h>

class DriveController : public IDriveController
{
public:
    DriveController(
        IMotorController& leftMotor,
        IMotorController& rightMotor
    );

    void begin() override;

    void forward(float speed) override;
    void backward(float speed) override;

    void rotateLeft(float speed) override;
    void rotateRight(float speed) override;

    void setDrive(float linear, float angular) override;

    void stop() override;

    DriveState getState() const override;
    float getLinearSpeed() const override;
    float getAngularSpeed() const override;
    
    RobotMotion getMotion() const;

private:
    IMotorController& _leftMotor;
    IMotorController& _rightMotor;

    DriveState _state = DriveState::Stopped;

    float _linearSpeed = 0.0f;
    float _angularSpeed = 0.0f;

    void apply(float leftSpeed, float rightSpeed);
    void updateState();
};