#pragma once

class IServoController
{
public:
    virtual ~IServoController() = default;

    virtual void begin() = 0;
    virtual void setAngle(float degrees) = 0;
    virtual float getAngle() const = 0;
};
