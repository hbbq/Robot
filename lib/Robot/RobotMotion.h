#pragma once

#include <cstdint>

enum class RobotMotion : uint8_t
{
    Stopped = 0,
    Forward,
    Backward,
    TurningLeft,
    TurningRight,
    Curve
};