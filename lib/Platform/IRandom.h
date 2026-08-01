#pragma once

#include <cstdint>

class IRandom
{
public:
    virtual ~IRandom() = default;

    virtual int32_t next(
        int32_t min,
        int32_t max) = 0;

    virtual float nextFloat(
        float min,
        float max) = 0;
};