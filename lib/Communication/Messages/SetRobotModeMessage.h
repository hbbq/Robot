#pragma once

#include <cstdint>
#include <type_traits>

#include <Messages/MessageHeader.h>
#include <RobotMode.h>

struct SetRobotModeMessage
{
    MessageHeader header;

    RobotMode mode;

    uint8_t reserved[3];
};

static_assert(sizeof(SetRobotModeMessage) == 16);

static_assert(
    std::is_trivially_copyable_v<SetRobotModeMessage>
);

inline SetRobotModeMessage makeSetRobotModeMessage(
    uint32_t sequenceNumber,
    RobotId robotId,
    RobotMode mode)
{
    return SetRobotModeMessage
    {
        .header =
        {
            .protocolVersion =
                CommunicationProtocolVersion,

            .type =
                MessageType::SetRobotMode,

            .messageLength =
                sizeof(SetRobotModeMessage),

            .sequenceNumber =
                sequenceNumber,

            .robotId = robotId
        },

        .mode = mode,
        .reserved = {0, 0, 0}
    };
}
