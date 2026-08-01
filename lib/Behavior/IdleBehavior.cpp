#include "IdleBehavior.h"

#include <MotionController.h>

IdleBehavior::IdleBehavior(
    MotionController& motionController)
    : _motionController(motionController)
{
}

void IdleBehavior::begin()
{
    _motionController.stop();
}

void IdleBehavior::update()
{
}

RobotMode IdleBehavior::mode() const
{
    return RobotMode::Idle;
}
