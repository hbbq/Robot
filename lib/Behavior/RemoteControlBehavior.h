#pragma once

#include <cstdint>

#include <IBehavior.h>

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
        uint32_t commandTimeoutMs = 500);

    void begin() override;
    void update() override;

private:
    RemoteDriveState& _remoteDriveState;
    IDriveController& _driveController;
    IClock& _clock;

    uint32_t _commandTimeoutMs;
};