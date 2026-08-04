#include "RemoteControlBehavior.h"

#include <RemoteDriveState.h>
#include <IDriveController.h>
#include <IClock.h>
#include <Arduino.h>
#include <algorithm>

RemoteControlBehavior::RemoteControlBehavior(
    RemoteDriveState& remoteDriveState,
    IDriveController& driveController,
    IClock& clock,
    const RemoteControlBehaviorConfig& config)
    : _remoteDriveState(remoteDriveState),
      _driveController(driveController),
      _clock(clock),
      _config(config)
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

    if (age > _config.commandTimeoutMs)
    {
        _driveController.stop();

        return;
    }

    const float maxLinearSpeed =
        std::clamp(_config.maxLinearSpeed, 0.0f, 1.0f);

    const float maxAngularSpeed =
        std::clamp(_config.maxAngularSpeed, 0.0f, 1.0f);

    const float linear =
        std::clamp(
            _remoteDriveState.linear(),
            -1.0f,
            1.0f) *
        maxLinearSpeed;

    const float angular =
        std::clamp(
            _remoteDriveState.angular(),
            -1.0f,
            1.0f) *
        maxAngularSpeed;

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
