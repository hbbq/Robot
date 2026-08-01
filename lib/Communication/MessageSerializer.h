#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <type_traits>

class MessageSerializer
{
public:
    template<typename T>
    static std::optional<T> deserialize(
        const uint8_t* data,
        size_t size)
    {
        static_assert(
            std::is_trivially_copyable_v<T>,
            "Messages must be trivially copyable");

        if (data == nullptr || size != sizeof(T))
        {
            return std::nullopt;
        }

        T message{};
        std::memcpy(&message, data, sizeof(T));

        return message;
    }

    template<typename T>
    static bool serialize(
        const T& message,
        uint8_t* destination,
        size_t destinationSize)
    {
        static_assert(
            std::is_trivially_copyable_v<T>,
            "Messages must be trivially copyable");

        if (destination == nullptr ||
            destinationSize < sizeof(T))
        {
            return false;
        }

        std::memcpy(
            destination,
            &message,
            sizeof(T));

        return true;
    }
};