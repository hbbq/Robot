#pragma once

#include <cstdint>

#include <BrightnessControllerConfig.h>

class IClock;
class IDisplayDriver;

class BrightnessController
{
public:
    BrightnessController(
        IDisplayDriver& display,
        IClock& clock,
        const BrightnessControllerConfig& config);

    void begin();
    void setActive(bool active);
    void update();

private:
    IDisplayDriver& _display;
    IClock& _clock;
    const BrightnessControllerConfig& _config;

    uint8_t _currentPercent = 0;
    uint8_t _startPercent = 0;
    uint8_t _targetPercent = 0;
    uint32_t _fadeStartedMs = 0;
};
