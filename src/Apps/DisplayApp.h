#pragma once

#ifdef DEVICE_DISPLAY

#include <ArduinoClock.h>
#include <DeviceRegistry.h>
#include <MessageDispatcher.h>
#include <EspNowManager.h>
#include <DeviceNetworkService.h>
#include <ReadinessController.h>
#include <LedController.h>
#include <RobotStateStore.h>
#include <Face/FaceController.h>
#include <Drivers/WaveshareEsp32C6TouchLcd147DisplayDriver.h>
#include <ArduinoRandom.h>
#include <RemoteDriveState.h>

class DisplayApp
{
public:
    DisplayApp();

    void begin();
    void update();

private:
    ArduinoClock _clock;
    ArduinoRandom _random;

    DeviceRegistry _deviceRegistry;
    MessageDispatcher _messageDispatcher;
    EspNowManager _espNow;
    DeviceNetworkService _deviceNetwork;

    LedController _statusLed;
    ReadinessController _readiness;
    
    RobotStateStore _robotStateStore;
    RemoteDriveState _remoteDriveState;

    WaveshareEsp32C6TouchLcd147DisplayDriver _display;
    FaceController _faceController;

    // DisplayController _display;
    // TouchController _touch;
    // FaceController _face;
};

#endif