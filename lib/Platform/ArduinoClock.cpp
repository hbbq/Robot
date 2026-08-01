#include "ArduinoClock.h"
#include <Arduino.h>

unsigned long ArduinoClock::millis() const
{
    return ::millis();
}