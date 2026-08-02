#include "DeviceRegistry.h"

#include <algorithm>
#include <cstring>

DeviceRegistry::DeviceRegistry(RobotId robotId)
    : _robotId(robotId)
{
}

bool DeviceRegistry::updateDevice(
    const uint8_t macAddress[6],
    RobotId robotId,
    DeviceType deviceType,
    Capability capabilities,
    int8_t rssi,
    uint32_t nowMs)
{
    if (macAddress == nullptr || robotId != _robotId)
    {
        return false;
    }

    portENTER_CRITICAL(&_mutex);

    int index =
        findDeviceIndexUnsafe(macAddress);

    if (index < 0)
    {
        if (_deviceCount >= MaxDevices)
        {
            portEXIT_CRITICAL(&_mutex);
            return false;
        }

        index = static_cast<int>(_deviceCount);
        ++_deviceCount;

        _devices[index] = DeviceInfo(
            macAddress,
            robotId,
            deviceType,
            capabilities,
            rssi,
            nowMs);
    }
    else
    {
        _devices[index].update(
            deviceType,
            capabilities,
            rssi,
            nowMs);
    }

    portEXIT_CRITICAL(&_mutex);

    return true;
}

bool DeviceRegistry::markSeen(
    const uint8_t macAddress[6],
    int8_t rssi,
    uint32_t nowMs)
{
    if (macAddress == nullptr)
    {
        return false;
    }

    portENTER_CRITICAL(&_mutex);

    const int index =
        findDeviceIndexUnsafe(macAddress);

    if (index < 0)
    {
        portEXIT_CRITICAL(&_mutex);
        return false;
    }

    _devices[index].markSeen(
        rssi,
        nowMs);

    portEXIT_CRITICAL(&_mutex);

    return true;
}

void DeviceRegistry::updatePresence(
    uint32_t nowMs,
    uint32_t timeoutMs)
{
    portENTER_CRITICAL(&_mutex);

    for (size_t i = 0;
         i < _deviceCount;
         ++i)
    {
        _devices[i].updatePresence(
            nowMs,
            timeoutMs);
    }

    portEXIT_CRITICAL(&_mutex);
}

bool DeviceRegistry::hasDevice(
    DeviceType deviceType) const
{
    return find(deviceType).has_value();
}

bool DeviceRegistry::hasCapability(
    Capability capability) const
{
    return findFirst(capability).has_value();
}

bool DeviceRegistry::hasAllCapabilities(
    Capability requiredCapabilities) const
{
    portENTER_CRITICAL(&_mutex);

    Capability available =
        Capability::None;

    for (size_t index = 0;
         index < _deviceCount;
         ++index)
    {
        const auto& device =
            _devices[index];

        if (!device.isOnline())
        {
            continue;
        }

        available |=
            device.capabilities();
    }

    const bool result =
        ::hasCapability(
            available,
            requiredCapabilities);

    portEXIT_CRITICAL(&_mutex);

    return result;
}

bool DeviceRegistry::hasOnlineDeviceWithAllCapabilities(
    DeviceType deviceType,
    Capability requiredCapabilities) const
{
    if (deviceType == DeviceType::Unknown ||
        requiredCapabilities == Capability::None)
    {
        return false;
    }

    portENTER_CRITICAL(&_mutex);

    for (size_t index = 0;
         index < _deviceCount;
         ++index)
    {
        const auto& device =
            _devices[index];

        if (!device.isOnline() ||
            device.deviceType() != deviceType)
        {
            continue;
        }

        const Capability available =
            device.capabilities();

        if ((available & requiredCapabilities) ==
            requiredCapabilities)
        {
            portEXIT_CRITICAL(&_mutex);
            return true;
        }
    }

    portEXIT_CRITICAL(&_mutex);
    return false;
}

std::optional<DeviceInfo>
DeviceRegistry::find(
    DeviceType deviceType) const
{
    portENTER_CRITICAL(&_mutex);

    for (size_t index = 0;
         index < _deviceCount;
         ++index)
    {
        if (_devices[index].deviceType() ==
            deviceType)
        {
            const DeviceInfo result =
                _devices[index];

            portEXIT_CRITICAL(&_mutex);
            return result;
        }
    }

    portEXIT_CRITICAL(&_mutex);

    return std::nullopt;
}

std::optional<DeviceInfo>
DeviceRegistry::findFirst(
    Capability capability) const
{
    portENTER_CRITICAL(&_mutex);

    for (size_t index = 0;
         index < _deviceCount;
         ++index)
    {
        if (_devices[index].hasCapability(
                capability))
        {
            const DeviceInfo result =
                _devices[index];

            portEXIT_CRITICAL(&_mutex);
            return result;
        }
    }

    portEXIT_CRITICAL(&_mutex);

    return std::nullopt;
}

std::optional<DeviceInfo>
DeviceRegistry::findByMac(
    const uint8_t macAddress[6]) const
{
    if (macAddress == nullptr)
    {
        return std::nullopt;
    }

    portENTER_CRITICAL(&_mutex);

    const int index =
        findDeviceIndexUnsafe(macAddress);

    if (index < 0)
    {
        portEXIT_CRITICAL(&_mutex);
        return std::nullopt;
    }

    const DeviceInfo result =
        _devices[index];

    portEXIT_CRITICAL(&_mutex);

    return result;
}

std::vector<DeviceInfo>
DeviceRegistry::getDevices() const
{
    std::array<DeviceInfo, MaxDevices> snapshot{};
    size_t snapshotCount = 0;

    portENTER_CRITICAL(&_mutex);

    snapshotCount = _deviceCount;

    std::copy_n(
        _devices.begin(),
        snapshotCount,
        snapshot.begin());

    portEXIT_CRITICAL(&_mutex);

    return std::vector<DeviceInfo>(
        snapshot.begin(),
        snapshot.begin() + snapshotCount);
}

size_t DeviceRegistry::count() const
{
    portENTER_CRITICAL(&_mutex);

    const size_t result = _deviceCount;

    portEXIT_CRITICAL(&_mutex);

    return result;
}

int DeviceRegistry::findDeviceIndexUnsafe(
    const uint8_t macAddress[6]) const
{
    for (size_t index = 0;
         index < _deviceCount;
         ++index)
    {
        const auto& storedMac =
            _devices[index].macAddress();

        if (std::memcmp(
                storedMac.data(),
                macAddress,
                storedMac.size()) == 0)
        {
            return static_cast<int>(index);
        }
    }

    return -1;
}

RobotId DeviceRegistry::robotId() const
{
    return _robotId;
}
