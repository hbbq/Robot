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
    sizeof(HeartbeatMessage) == 12
);

static_assert(
    std::is_trivially_copyable_v<HeartbeatMessage>
);

inline HeartbeatMessage makeHeartbeatMessage(
    uint32_t sequenceNumber,
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
                sequenceNumber
        },

        .uptimeMs = uptimeMs
    };
}