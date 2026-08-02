#include "WifiConnectionService.h"

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <IClock.h>
#include <cstring>

WifiConnectionService::WifiConnectionService(
    IClock& clock,
    const WifiConnectionConfig& config)
    : _clock(clock),
      _config(config)
{
}

void WifiConnectionService::begin()
{
    _enabled = _config.ssid != nullptr && std::strlen(_config.ssid) > 0;

    if (!_enabled)
    {
        Serial.println("[WiFi] Disabled: configure src/LocalSecrets.h");
        return;
    }

    WiFi.setAutoReconnect(false);
    startConnection();
}

void WifiConnectionService::update()
{
    if (!_enabled)
    {
        return;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        if (!_connected)
        {
            handleConnected();
        }
        return;
    }

    if (_connected)
    {
        _connected = false;
        Serial.println("[WiFi] Disconnected");
    }

    const uint32_t nowMs = _clock.millis();
    if (nowMs - _lastConnectAttemptMs >= _config.reconnectIntervalMs)
    {
        startConnection();
    }
}

bool WifiConnectionService::isConnected() const
{
    return _connected;
}

void WifiConnectionService::startConnection()
{
    _lastConnectAttemptMs = _clock.millis();
    Serial.printf("[WiFi] Connecting to %s\n", _config.ssid);
    WiFi.begin(
        _config.ssid,
        _config.password,
        _config.expectedChannel);
}

void WifiConnectionService::handleConnected()
{
    const uint8_t actualChannel = WiFi.channel();
    if (actualChannel != _config.expectedChannel)
    {
        rejectUnexpectedChannel(actualChannel);
        return;
    }

    _connected = true;
    Serial.printf(
        "[WiFi] Connected to %s, channel=%u\n",
        _config.ssid,
        static_cast<unsigned>(actualChannel));
}

void WifiConnectionService::rejectUnexpectedChannel(uint8_t actualChannel)
{
    Serial.printf(
        "[WiFi] Unexpected channel: %u, expected %u; disconnecting to protect ESP-NOW\n",
        static_cast<unsigned>(actualChannel),
        static_cast<unsigned>(_config.expectedChannel));

    WiFi.disconnect(false, false);
    const esp_err_t result = esp_wifi_set_channel(
        _config.expectedChannel,
        WIFI_SECOND_CHAN_NONE);

    if (result != ESP_OK)
    {
        Serial.printf(
            "[WiFi] Failed to restore ESP-NOW channel, error=%d\n",
            static_cast<int>(result));
    }
}
