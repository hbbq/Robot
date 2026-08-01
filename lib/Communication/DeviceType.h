#pragma once

#include <stdint.h>

enum class DeviceType : uint8_t
{
    Unknown,
    Robot,
    Remote,
    Display
};