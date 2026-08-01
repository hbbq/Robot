#pragma once

#include <atomic>
#include <cstdint>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <ILedController.h>
#include <LedControllerConfig.h>

#include <ArduinoRandom.h>

class LedController : public ILedController
{
public:
    explicit LedController(
        const LedControllerConfig& config
    );

    void begin() override;

    void setMode(LedMode mode) override;
    LedMode getMode() const override;

    void setBrightness(float brightness) override;

private:
    const LedControllerConfig& _config;

    std::atomic<LedMode> _mode { LedMode::Off };
    std::atomic<float> _brightness { 1.0f };

    TaskHandle_t _taskHandle = nullptr;

    uint32_t _maxDuty = 0;

    static void taskEntry(void* parameter);
    void taskLoop();

    void updateOutput(
        LedMode mode,
        uint32_t elapsedMs
    );

    void writeBrightness(float brightness);

    uint32_t nextRandomChange = 0;
    bool nextRandomOn = false;

    ArduinoRandom random;
};