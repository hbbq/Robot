#pragma once

#include <cstdint>

#include <DeviceNetworkConfig.h>

struct CommunicationConfig
{
    uint8_t wifiChannel;
    DeviceNetworkConfig network;
};