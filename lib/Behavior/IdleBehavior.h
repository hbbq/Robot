#pragma once

#include <IBehavior.h>

class MotionController;

class IdleBehavior : public IBehavior
{
public:
    explicit IdleBehavior(
        MotionController& motionController);

    void begin() override;
    void update() override;

private:
    MotionController& _motionController;
};