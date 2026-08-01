#pragma once

#include <cstdint>

#include <Capability.h>
#include <DeviceType.h>

struct DeviceNetworkConfig
{
    DeviceType deviceType;
    Capability capabilities;

    uint32_t announcementIntervalMs = 10000;
    uint32_t heartbeatIntervalMs = 1000;
    uint32_t deviceTimeoutMs = 5000;
};