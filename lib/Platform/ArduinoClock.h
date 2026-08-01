#pragma once

#include "IClock.h"

class ArduinoClock : public IClock
{
public:
    unsigned long millis() const override;
};