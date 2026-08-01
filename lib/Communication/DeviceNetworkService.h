#pragma once

#include <cstdint>

#include <DeviceNetworkConfig.h>
#include <RobotActivity.h>
#include <RobotMode.h>
#include <RobotMotion.h>
#include <AutonomousBehaviorType.h>

class DeviceRegistry;
class EspNowManager;
class IClock;

class DeviceNetworkService
{
public:
    DeviceNetworkService(
        EspNowManager& espNow,
        DeviceRegistry& registry,
        IClock& clock,
        const DeviceNetworkConfig& config);

    bool begin();
    void update();

    bool setCapabilities(
        Capability capabilities);

    bool sendAnnouncement();
    bool sendHeartbeat();
    bool sendRobotState(
        RobotMode mode,
        RobotMotion motion,
        AutonomousBehaviorType autonomousBehavior);
    bool sendDriveCommand(
        float linear,
        float angular);
    bool sendSetRobotMode(
        RobotMode mode);
    bool sendSetAutonomousBehavior(
        AutonomousBehaviorType behavior);

private:
    EspNowManager& _espNow;
    DeviceRegistry& _registry;
    IClock& _clock;
    DeviceNetworkConfig _config;

    bool _started = false;

    uint32_t _sequenceNumber = 0;
    uint32_t _lastAnnouncementMs = 0;
    uint32_t _lastHeartbeatMs = 0;

    uint32_t nextSequenceNumber();
};
