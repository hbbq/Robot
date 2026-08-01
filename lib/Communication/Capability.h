#pragma once

#include <Arduino.h>
#include <stdint.h>

enum class Capability : uint32_t
{
    None          = 0,
    RemoteControl = 1u << 0,
    Display       = 1u << 1,
    Touch         = 1u << 2,
    Logging       = 1u << 3,
    Motors        = 1u << 4,
    Distance      = 1u << 5
};

constexpr Capability operator|(
    Capability left,
    Capability right)
{
    return static_cast<Capability>(
        static_cast<uint32_t>(left) |
        static_cast<uint32_t>(right));
}

constexpr Capability operator&(
    Capability left,
    Capability right)
{
    return static_cast<Capability>(
        static_cast<uint32_t>(left) &
        static_cast<uint32_t>(right));
}

constexpr Capability& operator|=(
    Capability& left,
    Capability right)
{
    left = left | right;
    return left;
}

constexpr bool hasCapability(
    Capability capabilities,
    Capability capability)
{
    return (capabilities & capability) != Capability::None;
}