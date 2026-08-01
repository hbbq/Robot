#include "BehaviorController.h"

#include <IBehavior.h>

void BehaviorController::begin()
{
    if (_currentBehavior != nullptr)
    {
        _currentBehavior->begin();
    }
}

void BehaviorController::update()
{
    if (_currentBehavior != nullptr)
    {
        _currentBehavior->update();
    }
}

void BehaviorController::setBehavior(
    IBehavior& behavior)
{
    if (_currentBehavior == &behavior)
    {
        return;
    }

    _currentBehavior = &behavior;
    _currentBehavior->begin();
}

IBehavior* BehaviorController::currentBehavior() const
{
    return _currentBehavior;
}

RobotMode BehaviorController::currentMode() const
{
    return _currentBehavior != nullptr
        ? _currentBehavior->mode()
        : RobotMode::Idle;
}
