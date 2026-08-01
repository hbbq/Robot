#pragma once

#include <Capability.h>
#include <ILedController.h>

struct ReadinessConfig
{
    Capability requiredCapabilities =
        Capability::None;

    LedMode startingLedMode =
        LedMode::SlowBlink;

    LedMode readyLedMode =
        LedMode::On;

    LedMode notReadyLedMode =
        LedMode::FastBlink;
};