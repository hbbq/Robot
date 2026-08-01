#include "DanceBehavior.h"

#include <IMotionController.h>

DanceBehavior::DanceBehavior(
    IMotionController& motionController)
    : _motionController(motionController)
{
}

void DanceBehavior::begin()
{
    _motionController.stop();
    _step = 0;
    startCurrentStep();
}

void DanceBehavior::update()
{
    if (_motionController.isBusy())
    {
        return;
    }

    _step = static_cast<uint8_t>(
        (_step + 1) % 6);

    startCurrentStep();
}

RobotMode DanceBehavior::mode() const
{
    return RobotMode::Autonomous;
}

void DanceBehavior::startCurrentStep()
{
    switch (_step)
    {
        case 0:
            _motionController.turnLeft(60.0f);
            break;

        case 1:
            _motionController.turnRight(120.0f);
            break;

        case 2:
            _motionController.turnLeft(60.0f);
            break;

        case 3:
            _motionController.goForward(0.2f);
            break;

        case 4:
            _motionController.goBackward(0.2f);
            break;

        case 5:
            _motionController.turnRight(180.0f);
            break;
    }
}
