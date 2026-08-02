#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include <freertos/FreeRTOS.h>

#include <Capability.h>
#include <DeviceInfo.h>
#include <DeviceType.h>
#include <RobotId.h>

class DeviceRegistry
{
public:
    static constexpr size_t MaxDevices = 10;

    explicit DeviceRegistry(RobotId robotId);

    bool updateDevice(
        const uint8_t macAddress[6],
        RobotId robotId,
        DeviceType deviceType,
        Capability capabilities,
        int8_t rssi,
        uint32_t nowMs);
        
    bool markSeen(
        const uint8_t macAddress[6],
        int8_t rssi,
        uint32_t nowMs);

    void updatePresence(
        uint32_t nowMs,
        uint32_t timeoutMs);

    bool hasDevice(
        DeviceType deviceType) const;

    bool hasCapability(
        Capability capability) const;
        
    bool hasAllCapabilities(
        Capability requiredCapabilities) const;

    bool hasOnlineDeviceWithAllCapabilities(
        DeviceType deviceType,
        Capability requiredCapabilities) const;

    std::optional<DeviceInfo> find(
        DeviceType deviceType) const;

    std::optional<DeviceInfo> findFirst(
        Capability capability) const;

    std::optional<DeviceInfo> findByMac(
        const uint8_t macAddress[6]) const;

    std::vector<DeviceInfo> getDevices() const;

    size_t count() const;

    RobotId robotId() const;

private:
    int findDeviceIndexUnsafe(
        const uint8_t macAddress[6]) const;

    std::array<DeviceInfo, MaxDevices> _devices{};
    size_t _deviceCount = 0;
    RobotId _robotId;

    mutable portMUX_TYPE _mutex =
        portMUX_INITIALIZER_UNLOCKED;
};
