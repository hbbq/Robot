#include "RemoteControlBehavior.h"

#include <RemoteDriveState.h>
#include <IDriveController.h>
#include <IClock.h>
#include <Arduino.h>

RemoteControlBehavior::RemoteControlBehavior(
    RemoteDriveState& remoteDriveState,
    IDriveController& driveController,
    IClock& clock,
    uint32_t commandTimeoutMs)
    : _remoteDriveState(remoteDriveState),
      _driveController(driveController),
      _clock(clock),
      _commandTimeoutMs(commandTimeoutMs)
{
}

void RemoteControlBehavior::begin()
{
    _driveController.stop();
}

void RemoteControlBehavior::update()
{
    if (!_remoteDriveState.hasCommand())
    {
        _driveController.stop();
        return;
    }

    const uint32_t age =
        _clock.millis() -
        _remoteDriveState.receivedAtMs();

    if (age > _commandTimeoutMs)
    {
        _driveController.stop();

        return;
    }

    const float linear =
        _remoteDriveState.linear();

    const float angular =
        _remoteDriveState.angular();

    _driveController.setDrive(
        linear,
        angular);

        Serial.printf(
            "RemoteControlBehavior: linear=%.2f, angular=%.2f\n",
            linear,
            angular);
}

RobotMode RemoteControlBehavior::mode() const
{
    return RobotMode::RemoteControl;
}
