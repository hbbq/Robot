#pragma once

#include <cstdint>
#include <type_traits>

#include <Messages/MessageHeader.h>
#include <RobotMode.h>
#include <RobotMotion.h>
#include <AutonomousBehaviorType.h>

struct RobotStateMessage
{
    MessageHeader header;

    RobotMode mode;
    RobotMotion motion;
    AutonomousBehaviorType autonomousBehavior;

    uint8_t reserved;
};

static_assert(sizeof(RobotStateMessage) == 12);
static_assert(
    std::is_trivially_copyable_v<RobotStateMessage>);

inline RobotStateMessage makeRobotStateMessage(
    uint32_t sequenceNumber,
    RobotMode mode,
    RobotMotion motion,
    AutonomousBehaviorType autonomousBehavior)
{
    return RobotStateMessage
    {
        .header =
        {
            .protocolVersion =
                CommunicationProtocolVersion,

            .type =
                MessageType::RobotState,

            .messageLength =
                sizeof(RobotStateMessage),

            .sequenceNumber =
                sequenceNumber
        },

        .mode = mode,
        .motion = motion,
        .autonomousBehavior = autonomousBehavior,
        .reserved = 0
    };
}
