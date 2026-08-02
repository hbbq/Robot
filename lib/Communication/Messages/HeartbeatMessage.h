#pragma once

#include <cstdint>
#include <type_traits>

#include <Messages/MessageHeader.h>

struct HeartbeatMessage
{
    MessageHeader header;

    uint32_t uptimeMs;
};

static_assert(
    sizeof(HeartbeatMessage) == 16
);

static_assert(
    std::is_trivially_copyable_v<HeartbeatMessage>
);

inline HeartbeatMessage makeHeartbeatMessage(
    uint32_t sequenceNumber,
    RobotId robotId,
    uint32_t uptimeMs)
{
    return HeartbeatMessage
    {
        .header =
        {
            .protocolVersion =
                CommunicationProtocolVersion,

            .type =
                MessageType::Heartbeat,

            .messageLength =
                sizeof(HeartbeatMessage),

            .sequenceNumber =
                sequenceNumber,

            .robotId = robotId
        },

        .uptimeMs = uptimeMs
    };
}
