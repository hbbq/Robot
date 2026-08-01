#pragma once

#include <RobotMode.h>

class IBehavior;

class BehaviorController
{
public:
    void begin();

    void update();

    void setBehavior(
        IBehavior& behavior);

    IBehavior* currentBehavior() const;
    RobotMode currentMode() const;

private:
    IBehavior* _currentBehavior = nullptr;
};
