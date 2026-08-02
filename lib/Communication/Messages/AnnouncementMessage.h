#pragma once

#include <cstdint>
#include <type_traits>

#include <Capability.h>
#include <DeviceType.h>
#include <Messages/MessageHeader.h>

struct AnnouncementMessage
{
    MessageHeader header;

    DeviceType deviceType;

    uint8_t reserved[3];

    Capability capabilities;
};

static_assert(sizeof(AnnouncementMessage) == 20);
static_assert(std::is_trivially_copyable_v<AnnouncementMessage>);

inline AnnouncementMessage makeAnnouncementMessage(
    uint32_t sequenceNumber,
    RobotId robotId,
    DeviceType deviceType,
    Capability capabilities)
{
    return AnnouncementMessage
    {
        .header =
        {
            .protocolVersion = CommunicationProtocolVersion,
            .type = MessageType::Announcement,
            .messageLength = sizeof(AnnouncementMessage),
            .sequenceNumber = sequenceNumber,
            .robotId = robotId
        },
        .deviceType = deviceType,
        .reserved = {0, 0, 0},
        .capabilities = capabilities
    };
}
