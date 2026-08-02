#pragma once
#include <cstdint>

struct NtpTimeServiceConfig
{
    const char* timezone;
    const char* primaryServer;
    const char* secondaryServer;
    uint32_t synchronizationCheckIntervalMs;
    uint32_t resyncIntervalMs;
};
