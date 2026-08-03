#ifdef DEVICE_REMOTE

#include "RemoteApp.h"

#include "../AppConfig.h"

RemoteApp::RemoteApp()
    : _deviceRegistry(
          AppConfig::SystemRobotId),

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

      _readiness(
          _deviceRegistry,
          AppConfig::Readiness),
          
     _display(0),

      _ui(
          _display,
          _touch,
          _deviceNetwork,
          _robotStateStore,
          _readiness,
          _clock,
          AppConfig::Joystick,
          AppConfig::DriveTransmission,
          AppConfig::RemoteUi)
{
}

void RemoteApp::begin()
{
    Serial.begin(115200);
    delay(1000);

    _display.begin();
    _display.setBrightness(
        AppConfig::RemoteBrightness.percentage);
    
    if (!_touch.begin())
    {
        Serial.println("[Remote] Touch init failed");
    }

    _readiness.begin();
    
    if (!_deviceNetwork.begin())
    {
        Serial.println("Network initialization failed");
    }

    _ui.begin();
}

void RemoteApp::update()
{
    _deviceNetwork.update();
    _readiness.update();
    _ui.update();
}

#endif
