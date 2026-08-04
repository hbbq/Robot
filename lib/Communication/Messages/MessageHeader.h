#pragma once

#include <cstdint>
#include <type_traits>

#include "MessageType.h"
#include <RobotId.h>

inline constexpr uint8_t CommunicationProtocolVersion = 4;

struct MessageHeader
{
    uint8_t protocolVersion;
    MessageType type;
    uint16_t messageLength;
    uint32_t sequenceNumber;
    RobotId robotId;
};

static_assert(sizeof(MessageHeader) == 12);
static_assert(std::is_trivially_copyable_v<MessageHeader>);
