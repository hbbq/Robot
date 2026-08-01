#pragma once

#include <Arduino.h>

namespace TestUtil {
    void waitForTestSerial(){
        Serial.begin(115200);
        delay(3000);
    }
}