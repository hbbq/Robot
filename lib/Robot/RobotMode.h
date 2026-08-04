#pragma once

#include <cstdint>

enum class RobotMode : uint8_t
{
    Idle = 0,
    Autonomous,
    RemoteControl,
    Calibration
};
