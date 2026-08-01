#include "EspNowManager.h"

#include "IEspNowReceiver.h"

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#include <cstring>

namespace
{
    constexpr uint8_t BroadcastAddress[6] =
    {
        0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF
    };

    bool isValidPayload(const void* data, size_t size)
    {
        return data != nullptr &&
               size > 0 &&
               size <= ESP_NOW_MAX_DATA_LEN;
    }
}

EspNowManager* EspNowManager::_instance = nullptr;

EspNowManager::EspNowManager(
    IEspNowReceiver& receiver,
    uint8_t wifiChannel)
    : _receiver(receiver),
      _wifiChannel(wifiChannel)
{
}

EspNowManager::~EspNowManager()
{
    if (!_started)
    {
        return;
    }

    esp_now_unregister_recv_cb();
    esp_now_deinit();

    if (_instance == this)
    {
        _instance = nullptr;
    }
}

bool EspNowManager::begin()
{
    if (_started)
    {
        return true;
    }

    if (_instance != nullptr && _instance != this)
    {
        return false;
    }

    if (_wifiChannel < 1 || _wifiChannel > 14)
    {
        return false;
    }

    WiFi.mode(WIFI_STA);

    const esp_err_t channelResult = esp_wifi_set_channel(
        _wifiChannel,
        WIFI_SECOND_CHAN_NONE);

    if (channelResult != ESP_OK)
    {
        return false;
    }

    const esp_err_t initResult = esp_now_init();

    if (initResult != ESP_OK)
    {
        return false;
    }

    _instance = this;

    const esp_err_t callbackResult =
        esp_now_register_recv_cb(receiveCallback);

    if (callbackResult != ESP_OK)
    {
        _instance = nullptr;
        esp_now_deinit();

        return false;
    }

    if (!addPeer(BroadcastAddress))
    {
        esp_now_unregister_recv_cb();
        esp_now_deinit();

        _instance = nullptr;

        return false;
    }

    _started = true;

    return true;
}

bool EspNowManager::broadcast(
    const void* data,
    size_t size)
{
    return send(BroadcastAddress, data, size);
}

bool EspNowManager::send(
    const uint8_t macAddress[6],
    const void* data,
    size_t size)
{
    if (!_started ||
        macAddress == nullptr ||
        !isValidPayload(data, size))
    {
        return false;
    }

    if (!addPeer(macAddress))
    {
        return false;
    }

    const esp_err_t result = esp_now_send(
        macAddress,
        static_cast<const uint8_t*>(data),
        size);

    return result == ESP_OK;
}

uint8_t EspNowManager::getWifiChannel() const
{
    return _wifiChannel;
}

bool EspNowManager::addPeer(
    const uint8_t macAddress[6])
{
    if (esp_now_is_peer_exist(macAddress))
    {
        return true;
    }

    esp_now_peer_info_t peer{};

    std::memcpy(
        peer.peer_addr,
        macAddress,
        sizeof(peer.peer_addr));

    peer.channel = _wifiChannel;
    peer.ifidx = WIFI_IF_STA;
    peer.encrypt = false;

    const esp_err_t result = esp_now_add_peer(&peer);

    return result == ESP_OK ||
           result == ESP_ERR_ESPNOW_EXIST;
}

void EspNowManager::receiveCallback(
    const esp_now_recv_info_t* info,
    const uint8_t* data,
    int size)
{
    if (_instance == nullptr ||
        info == nullptr ||
        data == nullptr ||
        size <= 0)
    {
        return;
    }
    const int8_t rssi =
        info->rx_ctrl != nullptr
            ? info->rx_ctrl->rssi
            : INT8_MIN;

    _instance->_receiver.onReceive(
        info->src_addr,
        data,
        static_cast<size_t>(size),
        rssi);
}