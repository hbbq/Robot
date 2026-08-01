#pragma once

#include <cstdint>

#include <IBehavior.h>

class IMotionController;

class DanceBehavior : public IBehavior
{
public:
    explicit DanceBehavior(
        IMotionController& motionController);

    void begin() override;
    void update() override;
    RobotMode mode() const override;

private:
    IMotionController& _motionController;
    uint8_t _step = 0;

    void startCurrentStep();
};
