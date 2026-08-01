#ifdef DEVICE_DISPLAY

#include "DisplayApp.h"

#include <format>

#include "../AppConfig.h"

DisplayApp::DisplayApp()
    : _display(1),

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
          AppConfig::StatusLed),

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
    _display.setBrightness(50);
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
