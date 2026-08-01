#pragma once

#include "IClock.h"

class FakeClock : public IClock
{
public:
    unsigned long millis() const override
    {
        return _time;
    }

    void advance(unsigned long ms)
    {
        _time += ms;
    }

    void reset()
    {
        _time = 0;
    }

private:
    unsigned long _time = 0;
};