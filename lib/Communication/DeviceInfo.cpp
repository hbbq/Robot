#include "DeviceInfo.h"

#include <algorithm>

DeviceInfo::DeviceInfo(
    const uint8_t macAddress[6],
    RobotId robotId,
    DeviceType deviceType,
    Capability capabilities,
    int8_t rssi,
    uint32_t nowMs)
{
    _robotId = robotId;

    if (macAddress != nullptr)
    {
        std::copy_n(
            macAddress,
            _macAddress.size(),
            _macAddress.begin());
    }

    update(
        deviceType,
        capabilities,
        rssi,
        nowMs);
}

void DeviceInfo::update(
    DeviceType deviceType,
    Capability capabilities,
    int8_t rssi,
    uint32_t nowMs)
{
    _deviceType = deviceType;
    _capabilities = capabilities;

    markSeen(
        rssi,
        nowMs);
}

void DeviceInfo::markSeen(
    int8_t rssi,
    uint32_t nowMs)
{
    _lastSeenMs = nowMs;
    _lastRssi = rssi;
    _online = true;

    if (!_hasRssi)
    {
        _filteredRssi =
            static_cast<float>(rssi);

        _hasRssi = true;

        return;
    }

    _filteredRssi =
        _filteredRssi *
            (1.0f - RssiFilterFactor) +
        static_cast<float>(rssi) *
            RssiFilterFactor;
}

void DeviceInfo::updatePresence(
    uint32_t nowMs,
    uint32_t timeoutMs)
{
    _online =
        nowMs - _lastSeenMs <= timeoutMs;
}

const std::array<uint8_t, 6>&
DeviceInfo::macAddress() const
{
    return _macAddress;
}

DeviceType DeviceInfo::deviceType() const
{
    return _deviceType;
}

Capability DeviceInfo::capabilities() const
{
    return _capabilities;
}

bool DeviceInfo::hasCapability(
    Capability capability) const
{
    return ::hasCapability(
        _capabilities,
        capability);
}

int8_t DeviceInfo::lastRssi() const
{
    return _lastRssi;
}

float DeviceInfo::filteredRssi() const
{
    return _filteredRssi;
}

uint32_t DeviceInfo::lastSeenMs() const
{
    return _lastSeenMs;
}

bool DeviceInfo::isOnline() const
{
    return _online;
}

bool DeviceInfo::isExpired(
    uint32_t nowMs,
    uint32_t timeoutMs) const
{
    return nowMs - _lastSeenMs > timeoutMs;
}

RobotId DeviceInfo::robotId() const
{
    return _robotId;
}
