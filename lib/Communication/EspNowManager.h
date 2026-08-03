#pragma once

#include <stddef.h>
#include <stdint.h>

class IEspNowReceiver;

class EspNowManager
{
public:
    explicit EspNowManager(
        IEspNowReceiver& receiver,
        uint8_t wifiChannel);

    ~EspNowManager();

    EspNowManager(const EspNowManager&) = delete;
    EspNowManager& operator=(const EspNowManager&) = delete;

    bool begin();

    bool broadcast(
        const void* data,
        size_t size);

    bool send(
        const uint8_t macAddress[6],
        const void* data,
        size_t size);

    uint8_t getWifiChannel() const;

private:
    static void receiveCallback(
        const struct esp_now_recv_info* info,
        const uint8_t* data,
        int size);

    bool addPeer(const uint8_t macAddress[6]);

    IEspNowReceiver& _receiver;
    uint8_t _wifiChannel;
    bool _started = false;

    static EspNowManager* _instance;
};
