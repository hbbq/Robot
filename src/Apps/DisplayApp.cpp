#ifdef DEVICE_DISPLAY

#include "DisplayApp.h"

#include <format>

#include "../AppConfig.h"

DisplayApp::DisplayApp()
    : _deviceRegistry(
          AppConfig::SystemRobotId),

      _display(1),

      _brightnessController(
          _display,
          _clock,
          AppConfig::DisplayBrightness),

      _faceController(
          _display,
          _clock,
          _random),

    _messageDispatcher(
        _deviceRegistry,
        _robotStateStore,
        _remoteDriveState,
        _robotModeRequestStore,
        _autonomousBehaviorRequestStore,
        _frontScanMeasurementStore,
        _clock),

      _espNow(
          _messageDispatcher,
          AppConfig::Communication.wifiChannel),

      _deviceNetwork(
          _espNow,
          _deviceRegistry,
          _clock,
          AppConfig::Communication.network),

      _statusLed(
          AppConfig::StatusLedHardware,
          AppConfig::StatusLedAnimation),

      _readiness(
          _deviceRegistry,
          _statusLed,
          AppConfig::Readiness)
{
}

void DisplayApp::begin()
{
    Serial.begin(115200);
    delay(1000);

    _statusLed.begin();
    _readiness.begin();
    
    _display.begin();
    _brightnessController.begin();
    _faceController.begin();

    if (!_deviceNetwork.begin())
    {
        Serial.println("Network initialization failed");
    }
}

void DisplayApp::update()
{
    _deviceNetwork.update();
    _readiness.update();

    const bool displayActive =
        _readiness.isReady() &&
        _robotStateStore.mode() != RobotMode::Idle;

    _brightnessController.setActive(displayActive);
    _brightnessController.update();

    _faceController.update(
        _readiness.isReady(),
        _robotStateStore.mode(),
        _robotStateStore.motion(),
        _robotStateStore.autonomousBehavior());

    if (_readiness.isReady())
    {
        // Visa normalt robotansikte.
    }
    else
    {
        // Visa väntar på robot/frånkopplad.
    }
}

#endif
