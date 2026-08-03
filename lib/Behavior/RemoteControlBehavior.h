#pragma once

#include <cstdint>

#include <IBehavior.h>
#include <RemoteControlBehaviorConfig.h>

class RemoteDriveState;
class IDriveController;
class IClock;

class RemoteControlBehavior : public IBehavior
{
public:
    RemoteControlBehavior(
        RemoteDriveState& remoteDriveState,
        IDriveController& driveController,
        IClock& clock,
        const RemoteControlBehaviorConfig& config);

    void begin() override;
    void update() override;
    RobotMode mode() const override;

private:
    RemoteDriveState& _remoteDriveState;
    IDriveController& _driveController;
    IClock& _clock;

    const RemoteControlBehaviorConfig& _config;
};
