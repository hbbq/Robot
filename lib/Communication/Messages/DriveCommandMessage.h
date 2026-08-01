#pragma once

#include <cstdint>
#include <type_traits>

#include <Messages/MessageHeader.h>

struct DriveCommandMessage
{
    MessageHeader header;

    float linear;
    float angular;
};

static_assert(
    std::is_trivially_copyable_v<DriveCommandMessage>
);

inline DriveCommandMessage makeDriveCommandMessage(
    uint32_t sequenceNumber,
    float linear,
    float angular)
{
    return DriveCommandMessage
    {
        .header =
        {
            .protocolVersion =
                CommunicationProtocolVersion,

            .type =
                MessageType::DriveCommand,

            .messageLength =
                sizeof(DriveCommandMessage),

            .sequenceNumber =
                sequenceNumber
        },

        .linear = linear,
        .angular = angular
    };
}