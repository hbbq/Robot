#pragma once
#include <cstdint>

struct WifiConnectionConfig
{
    const char* ssid;
    const char* password;
    uint8_t expectedChannel;
    uint32_t reconnectIntervalMs;
};
