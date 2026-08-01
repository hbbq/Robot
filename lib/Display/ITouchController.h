#pragma once

#include <cstdint>

class ITouchController
{
public:
    virtual ~ITouchController() = default;

    virtual bool begin() = 0;

    virtual bool newTouch(
        int16_t& x,
        int16_t& y) = 0;

    virtual bool isPressed() const = 0;
};