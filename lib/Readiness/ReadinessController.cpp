#include "ReadinessController.h"

#include <DeviceRegistry.h>
#include <ILedController.h>

ReadinessController::ReadinessController(
    DeviceRegistry& deviceRegistry,
    const ReadinessConfig& config)
    : _deviceRegistry(deviceRegistry),
      _config(config)
{
}

ReadinessController::ReadinessController(
    DeviceRegistry& deviceRegistry,
    ILedController& statusLed,
    const ReadinessConfig& config)
    : _deviceRegistry(deviceRegistry),
      _statusLed(&statusLed),
      _config(config)
{
}

void ReadinessController::begin()
{
    _state = ReadinessState::Starting;
    updateLed();
}

void ReadinessController::update()
{
    const bool requirementsMet =
        _deviceRegistry.hasOnlineDeviceWithAllCapabilities(
            _config.requiredDeviceType,
            _config.requiredCapabilities);

    setState(
        requirementsMet
            ? ReadinessState::Ready
            : ReadinessState::MissingRequirements);
}

bool ReadinessController::isReady() const
{
    return _state == ReadinessState::Ready;
}

ReadinessState ReadinessController::state() const
{
    return _state;
}

void ReadinessController::setState(
    ReadinessState state)
{
    if (_state == state)
    {
        return;
    }

    _state = state;

    updateLed();
}

void ReadinessController::updateLed()
{
    if (_statusLed == nullptr)
    {
        return;
    }

    switch (_state)
    {
        case ReadinessState::Starting:
            _statusLed->setMode(
                _config.startingLedMode);
            break;

        case ReadinessState::Ready:
            _statusLed->setMode(
                _config.readyLedMode);
            break;

        case ReadinessState::MissingRequirements:
            _statusLed->setMode(
                _config.notReadyLedMode);
            break;
    }
}