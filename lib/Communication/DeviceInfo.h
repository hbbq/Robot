#pragma once

#include <array>
#include <cstdint>

#include <Capability.h>
#include <DeviceType.h>

class DeviceInfo
{
public:
    DeviceInfo() = default;

    DeviceInfo(
        const uint8_t macAddress[6],
        DeviceType deviceType,
        Capability capabilities,
        int8_t rssi,
        uint32_t nowMs);

    void update(
        DeviceType deviceType,
        Capability capabilities,
        int8_t rssi,
        uint32_t nowMs);
        
    void markSeen(
        int8_t rssi,
        uint32_t nowMs);
    
    void updatePresence(
        uint32_t nowMs,
        uint32_t timeoutMs);

    const std::array<uint8_t, 6>& macAddress() const;

    DeviceType deviceType() const;
    Capability capabilities() const;

    bool hasCapability(Capability capability) const;

    int8_t lastRssi() const;
    float filteredRssi() const;

    uint32_t lastSeenMs() const;
    bool isOnline() const;
    bool isExpired(
        uint32_t nowMs,
        uint32_t timeoutMs) const;

private:
    static constexpr float RssiFilterFactor = 0.2f;

    std::array<uint8_t, 6> _macAddress{};

    DeviceType _deviceType = DeviceType::Unknown;
    Capability _capabilities = Capability::None;

    int8_t _lastRssi = 0;
    float _filteredRssi = 0.0f;
    bool _hasRssi = false;
    bool _online = true;

    uint32_t _lastSeenMs = 0;
};