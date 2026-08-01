#include <Arduino.h>

#if defined(DEVICE_ROBOT)
    #include "Apps/RobotApp.h"
    RobotApp app;
#elif defined(DEVICE_DISPLAY)
    #include "Apps/DisplayApp.h"
    DisplayApp app;
#elif defined(DEVICE_REMOTE)
    #include "Apps/RemoteApp.h"
    RemoteApp app;
#else
    #error "No device type selected"
#endif

void setup()
{
    app.begin();
}

void loop()
{
    app.update();
}