#pragma once

#include <cstdint>

class IDisplayDriver;
class ITouchController;
class DeviceNetworkService;
class IClock;

class RemoteJoystickController
{
public:
    RemoteJoystickController(
        IDisplayDriver& display,
        ITouchController& touch,
        DeviceNetworkService& network,
        IClock& clock);

    void begin();
    void update();

private:
    IDisplayDriver& _display;
    ITouchController& _touch;
    DeviceNetworkService& _network;
    IClock& _clock;

    static constexpr int16_t CenterX = 120;
    static constexpr int16_t CenterY = 160;
    static constexpr int16_t Radius = 95;
    static constexpr int16_t KnobRadius = 18;

    static constexpr float DeadZone = 0.12f;
    static constexpr uint32_t SendIntervalMs = 100;

    bool _active = false;

    float _linear = 0.0f;
    float _angular = 0.0f;

    int16_t _knobX = CenterX;
    int16_t _knobY = CenterY;

    uint32_t _lastSendMs = 0;

    void updateTouch();
    void setPosition(int16_t x, int16_t y);
    void release();

    void sendCommand(bool force = false);

    void draw();
};