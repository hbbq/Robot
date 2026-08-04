#include "LedController.h"

#include <Arduino.h>

#include <algorithm>
#include <cmath>

namespace
{
    constexpr float Pi = 3.14159265358979323846f;    
    
    constexpr uint32_t TaskStackSize = 2048;
    constexpr uint8_t TaskPriority  = 8;
}

LedController::LedController(
    const LedHardwareConfig& hardwareConfig,
    const LedAnimationConfig& animationConfig
)
    : _hardwareConfig(hardwareConfig),
      _animationConfig(animationConfig)
{
}

void LedController::begin()
{
    _maxDuty =
        (1UL << _hardwareConfig.pwmResolutionBits) - 1UL;

    const bool attached = ledcAttachChannel(
        _hardwareConfig.pin,
        _hardwareConfig.pwmFrequencyHz,
        _hardwareConfig.pwmResolutionBits,
        _hardwareConfig.pwmChannel);

    if (!attached)
    {
        Serial.printf(
            "[LED] LEDC attach failed: pin=%u channel=%u freq=%lu res=%u\n",
            _hardwareConfig.pin,
            _hardwareConfig.pwmChannel,
            static_cast<unsigned long>(
                _hardwareConfig.pwmFrequencyHz),
            _hardwareConfig.pwmResolutionBits);
    }

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
            pdMS_TO_TICKS(
                _animationConfig.updateIntervalMs)
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
        _animationConfig.maximumBrightness;

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
                    _animationConfig.slowBlinkIntervalMs) % 2 == 0;

            writeBrightness(on ? brightness : 0.0f);
            break;
        }

        case LedMode::FastBlink:
        {
            const bool on =
                (elapsedMs /
                    _animationConfig.fastBlinkIntervalMs) % 2 == 0;

            writeBrightness(on ? brightness : 0.0f);
            break;
        }

        case LedMode::Pulse:
        {
            const uint32_t positionMs =
                elapsedMs % _animationConfig.pulseDurationMs;

            const float phase =
                static_cast<float>(positionMs) /
                static_cast<float>(
                    _animationConfig.pulseDurationMs
                );

            // 0 → 1 → 0 med mjuk sinuskurva.
            const float pulse =
                0.5f -
                0.5f * std::cos(
                    phase * 2.0f * Pi
                );

            const float minimum =
                _animationConfig.minimumPulseBrightness;

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

    if (!_hardwareConfig.activeHigh)
    {
        duty = _maxDuty - duty;
    }

    ledcWrite(_hardwareConfig.pin, duty);
}
