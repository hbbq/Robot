#pragma once

#include <cstdint>
#include <type_traits>

#include "MessageType.h"

inline constexpr uint8_t CommunicationProtocolVersion = 2;

struct MessageHeader
{
    uint8_t protocolVersion;
    MessageType type;
    uint16_t messageLength;
    uint32_t sequenceNumber;
};

static_assert(sizeof(MessageHeader) == 8);
static_assert(std::is_trivially_copyable_v<MessageHeader>);
