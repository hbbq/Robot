#pragma once

#include <cstdint>
#include <type_traits>

#include <AutonomousBehaviorType.h>
#include <Messages/MessageHeader.h>

struct SetAutonomousBehaviorMessage
{
    MessageHeader header;
    AutonomousBehaviorType behavior;
    uint8_t reserved[3];
};

static_assert(sizeof(SetAutonomousBehaviorMessage) == 16);
static_assert(
    std::is_trivially_copyable_v<SetAutonomousBehaviorMessage>);

inline SetAutonomousBehaviorMessage
makeSetAutonomousBehaviorMessage(
    uint32_t sequenceNumber,
    RobotId robotId,
    AutonomousBehaviorType behavior)
{
    return SetAutonomousBehaviorMessage
    {
        .header =
        {
            .protocolVersion = CommunicationProtocolVersion,
            .type = MessageType::SetAutonomousBehavior,
            .messageLength = sizeof(SetAutonomousBehaviorMessage),
            .sequenceNumber = sequenceNumber,
            .robotId = robotId
        },
        .behavior = behavior,
        .reserved = {0, 0, 0}
    };
}
