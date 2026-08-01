#pragma once

class IMotorController
{
public:
    virtual ~IMotorController() = default;

    virtual void begin() = 0;

    virtual void setSpeed(float speed) = 0;

    virtual float getSpeed() const = 0;

    virtual void stop() = 0;
};