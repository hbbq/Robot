#pragma once

#include <stddef.h>
#include <stdint.h>

class IEspNowReceiver
{
public:
    virtual ~IEspNowReceiver() = default;

    virtual void onReceive(
        const uint8_t senderMac[6],
        const uint8_t* data,
        size_t size,
        int8_t rssi) = 0;
};