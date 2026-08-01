#pragma once

#include <Capability.h>
#include <DeviceType.h>
#include <ILedController.h>

struct ReadinessConfig
{
    DeviceType requiredDeviceType =
        DeviceType::Unknown;

    Capability requiredCapabilities =
        Capability::None;

    LedMode startingLedMode =
        LedMode::SlowBlink;

    LedMode readyLedMode =
        LedMode::On;

    LedMode notReadyLedMode =
        LedMode::FastBlink;
};