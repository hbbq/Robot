#include "LedController.h"

#include <Arduino.h>

#include <algorithm>
#include <cmath>

namespace
{
    constexpr float Pi = 3.14159265358979323846f;    
    
    constexpr uint32_t PwmFrequency = 5000;
    constexpr uint8_t PwmResolution = 8;

    constexpr uint32_t UpdateInterval = 15;

    constexpr uint32_t SlowBlink = 500;
    constexpr uint32_t FastBlink = 150;
    constexpr uint32_t MinRandom = 50;
    constexpr uint32_t MaxRandom = 500;

    constexpr uint32_t PulseDuration = 1500;

    constexpr float MaximumBrightness = 1.0f;
    constexpr float MinimumPulseBrightness = 0.05f;

    constexpr uint32_t TaskStackSize = 2048;
    constexpr uint8_t TaskPriority  = 8;
}

LedController::LedController(
    const LedControllerConfig& config
)
    : _config(config)
{
}

void LedController::begin()
{
    _maxDuty =
        (1UL << PwmResolution) - 1UL;

    ledcAttach(
        _config.pin,
        PwmFrequency,
        PwmResolution
    );

    writeBrightness(0.0f);

    xTaskCreate(
        taskEntry,
        "LedController", 
        TaskStackSize,
        this,
        TaskPriority,
        &_taskHandle
    );
}

void LedController::setMode(LedMode mode)
{
    _mode.store(mode);
}

LedMode LedController::getMode() const
{
    return _mode.load();
}

void LedController::setBrightness(float brightness)
{
    _brightness.store(
        std::clamp(brightness, 0.0f, 1.0f)
    );
}

void LedController::taskEntry(void* parameter)
{
    auto* controller =
        static_cast<LedController*>(parameter);

    controller->taskLoop();
}

void LedController::taskLoop()
{
    LedMode previousMode = _mode.load();
    uint32_t modeStartedAtMs = millis();

    while (true)
    {
        const LedMode currentMode = _mode.load();

        if (currentMode != previousMode)
        {
            previousMode = currentMode;
            modeStartedAtMs = millis();
            nextRandomChange = 0;
        }

        const uint32_t elapsedMs =
            millis() - modeStartedAtMs;

        updateOutput(currentMode, elapsedMs);

        vTaskDelay(
            pdMS_TO_TICKS(UpdateInterval)
        );
    }
}

void LedController::updateOutput(
    LedMode mode,
    uint32_t elapsedMs
)
{
    const float brightness =
        _brightness.load() *
        MaximumBrightness;

    switch (mode)
    {
        case LedMode::Off:
            writeBrightness(0.0f);
            break;

        case LedMode::On:
            writeBrightness(brightness);
            break;

        case LedMode::SlowBlink:
        {
            const bool on =
                (elapsedMs /
                    SlowBlink) % 2 == 0;

            writeBrightness(on ? brightness : 0.0f);
            break;
        }

        case LedMode::FastBlink:
        {
            const bool on =
                (elapsedMs /
                    FastBlink) % 2 == 0;

            writeBrightness(on ? brightness : 0.0f);
            break;
        }

        case LedMode::Pulse:
        {
            const uint32_t positionMs =
                elapsedMs % PulseDuration;

            const float phase =
                static_cast<float>(positionMs) /
                static_cast<float>(
                    PulseDuration
                );

            // 0 → 1 → 0 med mjuk sinuskurva.
            const float pulse =
                0.5f -
                0.5f * std::cos(
                    phase * 2.0f * Pi
                );

            const float minimum =
                MinimumPulseBrightness;

            const float level =
                minimum +
                pulse * (1.0f - minimum);

            writeBrightness(level * brightness);
            
            break;
        } 
    }
}

void LedController::writeBrightness(float brightness){
    brightness =
        std::clamp(brightness, 0.0f, 1.0f);

    uint32_t duty = static_cast<uint32_t>(
        brightness * static_cast<float>(_maxDuty)
    );

    if (!_config.activeHigh)
    {
        duty = _maxDuty - duty;
    }

    ledcWrite(_config.pin, duty);  
}