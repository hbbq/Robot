#pragma once

#include <stdint.h>

enum class MessageType : uint8_t
{
    Unknown = 0,
    Announcement,
    Heartbeat,
    RobotState,
    DriveCommand,
    SetRobotMode
};