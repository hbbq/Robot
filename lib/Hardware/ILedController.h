#pragma once

enum class LedMode
{
    Off,
    On,
    SlowBlink,
    FastBlink,
    Pulse,
};

class ILedController
{
public:
    virtual ~ILedController() = default;

    virtual void begin() = 0;

    virtual void setMode(LedMode mode) = 0;
    virtual LedMode getMode() const = 0;

    virtual void setBrightness(float brightness) = 0;
};