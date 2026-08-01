#pragma once

class IBehavior;

class BehaviorController
{
public:
    void begin();

    void update();

    void setBehavior(
        IBehavior& behavior);

    IBehavior* currentBehavior() const;

private:
    IBehavior* _currentBehavior = nullptr;
};