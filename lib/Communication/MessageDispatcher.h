#pragma once

#include "IEspNowReceiver.h"

class DeviceRegistry;
class IClock;
class RobotStateStore;
class RemoteDriveState;
class RobotModeRequestStore;
class AutonomousBehaviorRequestStore;

class MessageDispatcher : public IEspNowReceiver
{
public:
    MessageDispatcher(
        DeviceRegistry& deviceRegistry,
        RobotStateStore& robotStateStore,
        RemoteDriveState& remoteDriveState,
        RobotModeRequestStore& robotModeRequestStore,
        AutonomousBehaviorRequestStore& autonomousBehaviorRequestStore,
        IClock& clock);

    void onReceive(
        const uint8_t senderMac[6],
        const uint8_t* data,
        size_t size,
        int8_t rssi) override;

private:
    DeviceRegistry& _deviceRegistry;
    RobotStateStore& _robotStateStore;
    RemoteDriveState& _remoteDriveState;
    RobotModeRequestStore& _robotModeRequestStore;
    AutonomousBehaviorRequestStore& _autonomousBehaviorRequestStore;
    IClock& _clock;

    void handleAnnouncement(
        const uint8_t senderMac[6],
        const uint8_t* data,
        size_t size,
        int8_t rssi);

    void handleHeartbeat(
        const uint8_t senderMac[6],
        const uint8_t* data,
        size_t size,
        int8_t rssi);

    void handleRobotState(
        const uint8_t senderMac[6],
        const uint8_t* data,
        size_t size,
        int8_t rssi);

    void handleDriveCommand(
        const uint8_t senderMac[6],
        const uint8_t* data,
        size_t size,
        int8_t rssi);

    void handleSetRobotMode(
        const uint8_t senderMac[6],
        const uint8_t* data,
        size_t size,
        int8_t rssi);

    void handleSetAutonomousBehavior(
        const uint8_t senderMac[6],
        const uint8_t* data,
        size_t size,
        int8_t rssi);
};
