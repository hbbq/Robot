#pragma once

#include <cstdint>

enum class RobotActivity : uint8_t
{
    NotReady = 0,
    Idle,
    MovingForward,
    MovingBackward,
    TurningLeft,
    TurningRight
};