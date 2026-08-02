#pragma once
#include <cstdint>
#include <WifiConnectionConfig.h>

class IClock;

class WifiConnectionService
{
public:
    WifiConnectionService(IClock& clock, const WifiConnectionConfig& config);
    void begin();
    void update();
    bool isConnected() const;

private:
    IClock& _clock;
    const WifiConnectionConfig& _config;
    bool _enabled = false;
    bool _connected = false;
    uint32_t _lastConnectAttemptMs = 0;

    void startConnection();
    void handleConnected();
    void rejectUnexpectedChannel(uint8_t actualChannel);
};
