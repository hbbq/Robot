#pragma once
#include <cstdint>
#include <ITimeProvider.h>
#include <NtpTimeServiceConfig.h>

class IClock;

class NtpTimeService : public ITimeProvider
{
public:
    NtpTimeService(IClock& clock, const NtpTimeServiceConfig& config);
    void begin();
    void update(bool wifiConnected);
    bool hasValidTime() const override;
    int64_t unixTimestamp() const override;
    uint8_t localHour() const override;
    uint8_t localMinute() const override;

private:
    IClock& _clock;
    const NtpTimeServiceConfig& _config;
    bool _valid = false;
    bool _synchronizationRequested = false;
    uint32_t _lastCheckMs = 0;
    uint32_t _lastSynchronizationRequestMs = 0;

    void requestSynchronization();
    void checkSynchronization();
};
