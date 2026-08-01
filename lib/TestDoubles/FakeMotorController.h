#pragma once

#include "IMotorController.h"

class FakeMotorController : public IMotorController
{
public:
   void begin() override {}
    void stop() override { _speed = 0; }

    void setSpeed(float speed) override
    {
        _speed = speed;
    }

    float getSpeed() const override
    {
        return _speed;
    }

private:
    float _speed = 0;
};