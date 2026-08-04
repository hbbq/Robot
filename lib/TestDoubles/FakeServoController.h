#pragma once

#include <IServoController.h>

class FakeServoController : public IServoController
{
public:
    void begin() override
    {
        _beginCalled = true;
    }

    void setAngle(float degrees) override
    {
        _angle = degrees;
    }

    float getAngle() const override
    {
        return _angle;
    }

    bool beginCalled() const
    {
        return _beginCalled;
    }

private:
    bool _beginCalled = false;
    float _angle = 0.0f;
};
