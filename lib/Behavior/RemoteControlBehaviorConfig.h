#pragma once

#include <cstdint>

struct RemoteControlBehaviorConfig
{
    uint32_t commandTimeoutMs;
    float maxLinearSpeed;
    float maxAngularSpeed;
};
