#include "DeviceNetworkService.h"

#include <DeviceRegistry.h>
#include <EspNowManager.h>
#include <Messages/AnnouncementMessage.h>
#include <Messages/HeartbeatMessage.h>
#include <Messages/RobotStateMessage.h>
#include <Messages/DriveCommandMessage.h>
#include <Messages/SetRobotModeMessage.h>
#include <IClock.h>

DeviceNetworkService::DeviceNetworkService(
    EspNowManager& espNow,
    DeviceRegistry& registry,
    IClock& clock,
    const DeviceNetworkConfig& config)
    : _espNow(espNow),
      _registry(registry),
      _clock(clock),
      _config(config)
{
}

bool DeviceNetworkService::begin()
{
    if (_started)
    {
        return true;
    }

    if (!_espNow.begin())
    {
        return false;
    }

    _started = true;

    const uint32_t nowMs = _clock.millis();

    _lastAnnouncementMs = nowMs;
    _lastHeartbeatMs = nowMs;

    return sendAnnouncement();
}

void DeviceNetworkService::update()
{
    if (!_started)
    {
        return;
    }

    const uint32_t nowMs = _clock.millis();

    if (
        nowMs - _lastAnnouncementMs >=
        _config.announcementIntervalMs
    )
    {
        sendAnnouncement();
    }

    if (
        nowMs - _lastHeartbeatMs >=
        _config.heartbeatIntervalMs
    )
    {
        sendHeartbeat();
    }

    _registry.updatePresence(
        nowMs,
        _config.deviceTimeoutMs);
}

bool DeviceNetworkService::setCapabilities(
    Capability capabilities)
{
    if (_started)
    {
        return false;
    }

    _config.capabilities = capabilities;
    return true;
}

bool DeviceNetworkService::sendAnnouncement()
{
    if (!_started)
    {
        return false;
    }

    const AnnouncementMessage message =
        makeAnnouncementMessage(
            nextSequenceNumber(),
            _config.deviceType,
            _config.capabilities);

    const bool queued = _espNow.broadcast(
        &message,
        sizeof(message));

    if (queued)
    {
        _lastAnnouncementMs = _clock.millis();
    }

    return queued;
}

bool DeviceNetworkService::sendHeartbeat()
{
    if (!_started)
    {
        return false;
    }

    const uint32_t nowMs = _clock.millis();

    const HeartbeatMessage message =
        makeHeartbeatMessage(
            nextSequenceNumber(),
            nowMs);

    const bool queued = _espNow.broadcast(
        &message,
        sizeof(message));

    if (queued)
    {
        _lastHeartbeatMs = nowMs;
    }

    return queued;
}

bool DeviceNetworkService::sendRobotState(
    RobotMode mode,
    RobotMotion motion,
    AutonomousBehaviorType autonomousBehavior)
{
    if (!_started)
    {
        return false;
    }

    const RobotStateMessage message =
        makeRobotStateMessage(
            nextSequenceNumber(),
            mode,
            motion,
            autonomousBehavior);

    return _espNow.broadcast(
        &message,
        sizeof(message));
}

bool DeviceNetworkService::sendDriveCommand(
    float linear,
    float angular)
{
    if (!_started)
    {
        return false;
    }
    
    linear =
    std::clamp(
            linear,
            -1.0f,
            1.0f);

    angular =
        std::clamp(
            angular,
            -1.0f,
            1.0f);

    const DriveCommandMessage message =
        makeDriveCommandMessage(
            nextSequenceNumber(),
            linear,
            angular);

    return _espNow.broadcast(
        &message,
        sizeof(message));
}

bool DeviceNetworkService::sendSetRobotMode(
    RobotMode mode)
{
    if (!_started)
    {
        return false;
    }

    const auto message =
        makeSetRobotModeMessage(
            nextSequenceNumber(),
            mode);

    return _espNow.broadcast(
        &message,
        sizeof(message));
}

uint32_t DeviceNetworkService::nextSequenceNumber()
{
    return _sequenceNumber++;
}
