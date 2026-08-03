#pragma once

#include <cstdint>
#include <JoystickConfig.h>

class JoystickModel
{
public:
    JoystickModel(
        int16_t centerX,
        int16_t centerY,
        int16_t radius,
        const JoystickConfig& config);

    void setTouch(
        int16_t x,
        int16_t y);

    void release();

    bool active() const;

    float linear() const;
    float angular() const;

    int16_t knobX() const;
    int16_t knobY() const;

    int16_t centerX() const;
    int16_t centerY() const;
    int16_t radius() const;

private:
    int16_t _centerX;
    int16_t _centerY;
    int16_t _radius;

    float _deadZone;

    bool _active = false;

    float _linear = 0.0f;
    float _angular = 0.0f;

    int16_t _knobX;
    int16_t _knobY;
};
