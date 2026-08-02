#include "NtpTimeService.h"

#include <Arduino.h>
#include <IClock.h>
#include <ctime>

namespace
{
    constexpr std::time_t MinimumValidTimestamp = 1704067200;

    bool getLocalTimeParts(std::tm& result)
    {
        const std::time_t now = std::time(nullptr);
        return localtime_r(&now, &result) != nullptr;
    }
}

NtpTimeService::NtpTimeService(
    IClock& clock,
    const NtpTimeServiceConfig& config)
    : _clock(clock),
      _config(config)
{
}

void NtpTimeService::begin()
{
    _lastCheckMs = _clock.millis();
}

void NtpTimeService::update(bool wifiConnected)
{
    const uint32_t nowMs = _clock.millis();

    if (wifiConnected &&
        (!_synchronizationRequested ||
         nowMs - _lastSynchronizationRequestMs >= _config.resyncIntervalMs))
    {
        requestSynchronization();
    }

    if (nowMs - _lastCheckMs >= _config.synchronizationCheckIntervalMs)
    {
        _lastCheckMs = nowMs;
        checkSynchronization();
    }
}

bool NtpTimeService::hasValidTime() const
{
    return _valid;
}

int64_t NtpTimeService::unixTimestamp() const
{
    return _valid ? static_cast<int64_t>(std::time(nullptr)) : 0;
}

uint8_t NtpTimeService::localHour() const
{
    std::tm local{};
    return _valid && getLocalTimeParts(local)
        ? static_cast<uint8_t>(local.tm_hour)
        : 0;
}

uint8_t NtpTimeService::localMinute() const
{
    std::tm local{};
    return _valid && getLocalTimeParts(local)
        ? static_cast<uint8_t>(local.tm_min)
        : 0;
}

void NtpTimeService::requestSynchronization()
{
    configTzTime(
        _config.timezone,
        _config.primaryServer,
        _config.secondaryServer);
    _synchronizationRequested = true;
    _lastSynchronizationRequestMs = _clock.millis();
    Serial.println("[Time] NTP synchronization requested");
}

void NtpTimeService::checkSynchronization()
{
    const std::time_t now = std::time(nullptr);
    if (now < MinimumValidTimestamp || _valid)
    {
        return;
    }

    _valid = true;
    std::tm local{};
    if (!getLocalTimeParts(local))
    {
        Serial.println("[Time] NTP synchronized");
        return;
    }

    char formatted[24]{};
    std::strftime(formatted, sizeof(formatted), "%Y-%m-%d %H:%M:%S", &local);
    Serial.println("[Time] NTP synchronized");
    Serial.printf("[Time] %s\n", formatted);
}
