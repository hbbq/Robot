#pragma once

#include <IDriveController.h>

class FakeDriveController : public IDriveController
{
public:
    void begin() override
    {
        _beginCalled = true;
    }

    void forward(float speed) override
    {
        _state = DriveState::Forward;
        _linearSpeed = speed;
        _angularSpeed = 0.0f;
    }

    void backward(float speed) override
    {
        _state = DriveState::Backward;
        _linearSpeed = -speed;
        _angularSpeed = 0.0f;
    }

    void rotateLeft(float speed) override
    {
        _state = DriveState::RotateLeft;
        _linearSpeed = 0.0f;
        _angularSpeed = -speed;
    }

    void rotateRight(float speed) override
    {
        _state = DriveState::RotateRight;
        _linearSpeed = 0.0f;
        _angularSpeed = speed;
    }

    void setDrive(float linear, float angular) override
    {
        _state = DriveState::Curve;
        _linearSpeed = linear;
        _angularSpeed = angular;
    }

    void stop() override
    {
        _state = DriveState::Stopped;
        _linearSpeed = 0.0f;
        _angularSpeed = 0.0f;
        ++_stopCallCount;
    }

    DriveState getState() const override
    {
        return _state;
    }

    float getLinearSpeed() const override
    {
        return _linearSpeed;
    }

    float getAngularSpeed() const override
    {
        return _angularSpeed;
    }

    int getStopCallCount() const
    {
        return _stopCallCount;
    }

private:
    bool _beginCalled = false;

    DriveState _state = DriveState::Stopped;

    float _linearSpeed = 0.0f;
    float _angularSpeed = 0.0f;

    int _stopCallCount = 0;
};