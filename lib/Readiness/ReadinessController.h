#pragma once

#include <ReadinessConfig.h>

class DeviceRegistry;
class ILedController;

enum class ReadinessState
{
    Starting,
    Ready,
    MissingRequirements
};

class ReadinessController
{
public:
    ReadinessController(
        DeviceRegistry& deviceRegistry,
        const ReadinessConfig& config);

    ReadinessController(
        DeviceRegistry& deviceRegistry,
        ILedController& statusLed,
        const ReadinessConfig& config);

    void begin();
    void update();

    bool isReady() const;
    ReadinessState state() const;

private:
    DeviceRegistry& _deviceRegistry;
    ILedController* _statusLed = nullptr;
    const ReadinessConfig& _config;

    ReadinessState _state =
        ReadinessState::Starting;

    void setState(ReadinessState state);
    void updateLed();
};