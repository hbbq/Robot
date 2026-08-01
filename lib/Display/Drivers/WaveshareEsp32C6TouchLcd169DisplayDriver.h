// WaveshareEsp32C6TouchLcd169DisplayDriver.h
#pragma once

#ifdef HAS_DISPLAY_169

#include <cstdint>
#include <memory>
#include <string>

#include <IDisplayDriver.h>
//#include <ITouchController.h>

class Arduino_DataBus;
class Arduino_GFX;
class Arduino_Canvas;

class WaveshareEsp32C6TouchLcd169DisplayDriver
    : public IDisplayDriver//,      public ITouchController
{
public:
    explicit WaveshareEsp32C6TouchLcd169DisplayDriver(
        uint8_t rotation = 0);

    ~WaveshareEsp32C6TouchLcd169DisplayDriver() override;

    void begin() override;
    void flush() override;

    int16_t width() const override;
    int16_t height() const override;

    void setRotation(uint8_t rotation) override;
    void setBrightness(uint8_t percentage) override;

    void clear(uint16_t color) override;

    void drawPixel(
        int16_t x,
        int16_t y,
        uint16_t color) override;

    void drawLine(
        int16_t x0,
        int16_t y0,
        int16_t x1,
        int16_t y1,
        uint16_t color) override;

    void drawRect(
        int16_t x,
        int16_t y,
        int16_t width,
        int16_t height,
        uint16_t color) override;

    void fillRect(
        int16_t x,
        int16_t y,
        int16_t width,
        int16_t height,
        uint16_t color) override;

    void drawCircle(
        int16_t x0,
        int16_t y0,
        int16_t radius,
        uint16_t color) override;

    void fillCircle(
        int16_t x0,
        int16_t y0,
        int16_t radius,
        uint16_t color) override;

    void drawTriangle(
        int16_t x0,
        int16_t y0,
        int16_t x1,
        int16_t y1,
        int16_t x2,
        int16_t y2,
        uint16_t color) override;

    void fillTriangle(
        int16_t x0,
        int16_t y0,
        int16_t x1,
        int16_t y1,
        int16_t x2,
        int16_t y2,
        uint16_t color) override;

    void setCursor(
        int16_t x,
        int16_t y) override;

    void setTextColor(
        uint16_t color) override;

    void setTextSize(
        uint8_t size) override;

    void print(
        const std::string& text) override;

    /*bool newTouch(
        int16_t& x,
        int16_t& y) override;

    bool isPressed() const override;*/

private:
    std::unique_ptr<Arduino_DataBus> _bus;
    std::unique_ptr<Arduino_GFX> _gfx;
    std::unique_ptr<Arduino_Canvas> _canvas;

    uint8_t _rotation;
};

#endif