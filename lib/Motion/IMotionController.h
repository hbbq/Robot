#pragma once

class IMotionController
{
public:
    virtual ~IMotionController() = default;

    virtual void update() = 0;

    virtual void goForward(float meters) = 0;
    virtual void goBackward(float meters) = 0;

    virtual void turnLeft(float degrees) = 0;
    virtual void turnRight(float degrees) = 0;
    virtual void turn(float degrees) = 0;

    virtual void stop() = 0;

    virtual bool isBusy() const = 0;
};