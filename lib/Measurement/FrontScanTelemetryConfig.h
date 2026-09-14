#pragma once

#include <cstdint>

struct FrontScanTelemetryConfig
{
    uint32_t minimumSendIntervalMs;
    uint32_t snapshotIntervalMs;
};
