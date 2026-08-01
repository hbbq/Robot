#ifdef HAS_DISPLAY_169

#pragma once

#include <cstdint>

#include <ITouchController.h>

class TwoWire;
class TouchDrvCSTXXX;

class Cst816TouchDriver
    : public ITouchController
{
public:
    Cst816TouchDriver();

    ~Cst816TouchDriver() override;

    bool begin() override;

    bool newTouch(
        int16_t& x,
        int16_t& y) override;

    bool isPressed() const override;

private:
    static void interruptHandler();

    static volatile bool _interruptTriggered;

    TouchDrvCSTXXX* _touch = nullptr;

    bool _pressed = false;
};

#endif