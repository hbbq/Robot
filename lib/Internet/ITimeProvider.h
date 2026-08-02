#pragma once
#include <cstdint>

class ITimeProvider
{
public:
    virtual ~ITimeProvider() = default;
    virtual bool hasValidTime() const = 0;
    virtual int64_t unixTimestamp() const = 0;
    virtual uint8_t localHour() const = 0;
    virtual uint8_t localMinute() const = 0;
};
