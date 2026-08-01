#include "MessageDispatcher.h"

#include <Arduino.h>

#include <DeviceRegistry.h>
#include <MessageSerializer.h>
#include <Messages/HeartbeatMessage.h>
#include <Messages/AnnouncementMessage.h>
#include <Messages/RobotStateMessage.h>
#include <Messages/DriveCommandMessage.h>
#include <Messages/SetRobotModeMessage.h>
#include <Messages/MessageHeader.h>
#include <RobotStateStore.h>
#include <RemoteDriveState.h>
#include <IClock.h>
#include <RobotModeRequestStore.h>

#include <cstring>

MessageDispatcher::MessageDispatcher(
    DeviceRegistry& deviceRegistry,
    RobotStateStore& robotStateStore,
    RemoteDriveState& remoteDriveState,
    RobotModeRequestStore& robotModeRequestStore,
    IClock& clock)
    : _deviceRegistry(deviceRegistry),
      _robotStateStore(robotStateStore),
        _remoteDriveState(remoteDriveState),
        _robotModeRequestStore(robotModeRequestStore),
      _clock(clock)
{
}

void MessageDispatcher::onReceive(
    const uint8_t senderMac[6],
    const uint8_t* data,
    size_t size,
    int8_t rssi)
{
    if (senderMac == nullptr ||
        data == nullptr ||
        size < sizeof(MessageHeader))
    {
        return;
    }

    MessageHeader header{};

    std::memcpy(
        &header,
        data,
        sizeof(header));

    if (header.protocolVersion !=
        CommunicationProtocolVersion)
    {
        return;
    }

    if (header.messageLength != size)
    {
        return;
    }

    switch (header.type)
    {
        case MessageType::Announcement:
            handleAnnouncement(
                senderMac,
                data,
                size,
                rssi);
            break;

        case MessageType::Heartbeat:
            handleHeartbeat(
                senderMac,
                data,
                size,
                rssi);
            break;

        case MessageType::RobotState:
            handleRobotState(
                senderMac,
                data,
                size,
                rssi);
            break;
            
        case MessageType::DriveCommand:
            handleDriveCommand(
                senderMac,
                data,
                size,
                rssi);
            break;

        case MessageType::SetRobotMode:
            handleSetRobotMode(
                senderMac,
                data,
                size,
                rssi);
            break;

        default:
            break;
    }
}

void MessageDispatcher::handleAnnouncement(
    const uint8_t senderMac[6],
    const uint8_t* data,
    size_t size,
    int8_t rssi)
{
    const auto message =
        MessageSerializer::deserialize<
            AnnouncementMessage>(
                data,
                size);

    if (!message)
    {
        return;
    }

    _deviceRegistry.updateDevice(
        senderMac,
        message->deviceType,
        message->capabilities,
        rssi,
        _clock.millis());
}

void MessageDispatcher::handleHeartbeat(
    const uint8_t senderMac[6],
    const uint8_t* data,
    size_t size,
    int8_t rssi)
{
    const auto message =
        MessageSerializer::deserialize<
            HeartbeatMessage>(
                data,
                size);

    if (!message)
    {
        return;
    }

    const bool knownDevice =
        _deviceRegistry.markSeen(
            senderMac,
            rssi,
            _clock.millis());

    if (!knownDevice)
    {
        // Avsändaren kanske startade innan vi hann ta emot
        // dess announcement. Ignorera heartbeat tills vidare.
        return;
    }
}

void MessageDispatcher::handleRobotState(
    const uint8_t senderMac[6],
    const uint8_t* data,
    size_t size,
    int8_t rssi)
{
    const auto message =
        MessageSerializer::deserialize<
            RobotStateMessage>(
                data,
                size);

    if (!message)
    {
        return;
    }

    _robotStateStore.setState(
        message->mode,
        message->motion);
}

void MessageDispatcher::handleDriveCommand(
    const uint8_t senderMac[6],
    const uint8_t* data,
    size_t size,
    int8_t rssi)
{
    const auto message =
        MessageSerializer::deserialize<
            DriveCommandMessage>(
                data,
                size);

    if (!message)
    {
        return;
    }

    _remoteDriveState.setCommand(
        message->linear,
        message->angular,
        _clock.millis());
}

void MessageDispatcher::handleSetRobotMode(
    const uint8_t senderMac[6],
    const uint8_t* data,
    size_t size,
    int8_t rssi)
{
    const auto message =
        MessageSerializer::deserialize<
            SetRobotModeMessage>(
                data,
                size);

    if (!message)
    {
        return;
    }

    Serial.printf(
        "[Robot] Mode requested: %u\n",
        static_cast<unsigned>(
            message->mode));

    _robotModeRequestStore.request(
        message->mode);
}