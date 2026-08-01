#pragma once

#ifdef DEVICE_REMOTE

#include <ArduinoClock.h>
#include <DeviceRegistry.h>
#include <MessageDispatcher.h>
#include <EspNowManager.h>
#include <DeviceNetworkService.h>
#include <ReadinessController.h>
#include <RobotStateStore.h>
#include <RemoteDriveState.h>
#include <Drivers/WaveshareEsp32C6TouchLcd169DisplayDriver.h>
#include <Drivers/Cst816TouchDriver.h>
#include <RemoteUiController.h>
#include <RobotModeRequestStore.h>

class RemoteApp
{
public:
    RemoteApp();

    void begin();
    void update();

private:
    ArduinoClock _clock;

    DeviceRegistry _deviceRegistry;
    MessageDispatcher _messageDispatcher;
    EspNowManager _espNow;
    DeviceNetworkService _deviceNetwork;

    ReadinessController _readiness;
    
    RobotStateStore _robotStateStore;
    RemoteDriveState _remoteDriveState;
    RobotModeRequestStore _robotModeRequestStore;

    WaveshareEsp32C6TouchLcd169DisplayDriver _display;
    Cst816TouchDriver _touch;

    RemoteUiController _ui;
};

#endif