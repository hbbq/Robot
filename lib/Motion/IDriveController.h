#pragma once

enum class DriveState
{
    Stopped,
    Forward,
    Backward,
    RotateLeft,
    RotateRight,
    Curve
};

class IDriveController
{
public:
    virtual ~IDriveController() = default;

    virtual void begin() = 0;

    virtual void forward(float speed) = 0;
    virtual void backward(float speed) = 0;

    virtual void rotateLeft(float speed) = 0;
    virtual void rotateRight(float speed) = 0;

    virtual void setDrive(float linear, float angular) = 0;

    virtual void stop() = 0;

    virtual DriveState getState() const = 0;
    virtual float getLinearSpeed() const = 0;
    virtual float getAngularSpeed() const = 0;
};