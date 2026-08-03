#ifdef HAS_DISPLAY_169

#include "WaveshareEsp32C6TouchLcd169DisplayDriver.h"

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

#include <algorithm>

namespace
{
    constexpr int16_t DisplayWidth = 240;
    constexpr int16_t DisplayHeight = 280;

    constexpr uint8_t LcdSckPin = 1;
    constexpr uint8_t LcdMosiPin = 2;
    constexpr uint8_t LcdCsPin = 5;
    constexpr uint8_t LcdDcPin = 3;
    constexpr uint8_t LcdResetPin = 4;
    constexpr uint8_t LcdBacklightPin = 6;

    constexpr bool IsIps = true;

    constexpr int16_t ColumnOffsetRotation0 = 0;
    constexpr int16_t RowOffsetRotation0 = 20;
    constexpr int16_t ColumnOffsetRotation2 = 0;
    constexpr int16_t RowOffsetRotation2 = 20;

    constexpr uint32_t BacklightPwmFrequency = 5000;
    constexpr uint8_t BacklightPwmResolutionBits = 8;

    constexpr uint32_t BacklightMaxDuty =
        (1u << BacklightPwmResolutionBits) - 1u;
}

WaveshareEsp32C6TouchLcd169DisplayDriver::
WaveshareEsp32C6TouchLcd169DisplayDriver(
    uint8_t rotation)
    : _rotation(rotation)
{
    _bus = std::make_unique<Arduino_HWSPI>(
        LcdDcPin,
        LcdCsPin,
        LcdSckPin,
        LcdMosiPin);

    _gfx = std::make_unique<Arduino_ST7789>(
        _bus.get(),
        LcdResetPin,
        0,
        IsIps,
        DisplayWidth,
        DisplayHeight,
        ColumnOffsetRotation0,
        RowOffsetRotation0,
        ColumnOffsetRotation2,
        RowOffsetRotation2);
}

WaveshareEsp32C6TouchLcd169DisplayDriver::
~WaveshareEsp32C6TouchLcd169DisplayDriver() = default;

void WaveshareEsp32C6TouchLcd169DisplayDriver::begin()
{
    _gfx->begin();

    _canvas = std::make_unique<Arduino_Canvas>(
        _gfx->width(),
        _gfx->height(),
        _gfx.get());

    _canvas->begin();

    _canvas->setRotation(_rotation);
    _canvas->setTextWrap(false);

    ledcAttach(
        LcdBacklightPin,
        BacklightPwmFrequency,
        BacklightPwmResolutionBits);

    // Application policy selects the visible brightness after begin().
    setBrightness(0);
}

void WaveshareEsp32C6TouchLcd169DisplayDriver::flush()
{
    _canvas->flush();
}

int16_t WaveshareEsp32C6TouchLcd169DisplayDriver::width() const
{
    return _canvas
        ? _canvas->width()
        : _gfx->width();
}

int16_t WaveshareEsp32C6TouchLcd169DisplayDriver::height() const
{
    return _canvas
        ? _canvas->height()
        : _gfx->height();
}

void WaveshareEsp32C6TouchLcd169DisplayDriver::setRotation(
    uint8_t rotation)
{
    _rotation = rotation;

    if (_canvas)
    {
        _canvas->setRotation(rotation);
    }
}

void WaveshareEsp32C6TouchLcd169DisplayDriver::setBrightness(
    uint8_t percentage)
{
    percentage =
        std::min<uint8_t>(percentage, 100);

    const uint32_t duty =
        static_cast<uint32_t>(percentage) *
        BacklightMaxDuty /
        100u;

    ledcWrite(
        LcdBacklightPin,
        duty);
}

void WaveshareEsp32C6TouchLcd169DisplayDriver::clear(
    uint16_t color)
{
    _canvas->fillScreen(color);
}

void WaveshareEsp32C6TouchLcd169DisplayDriver::drawPixel(
    int16_t x,
    int16_t y,
    uint16_t color)
{
    _canvas->drawPixel(x, y, color);
}

void WaveshareEsp32C6TouchLcd169DisplayDriver::drawLine(
    int16_t x0,
    int16_t y0,
    int16_t x1,
    int16_t y1,
    uint16_t color)
{
    _canvas->drawLine(
        x0, y0,
        x1, y1,
        color);
}

void WaveshareEsp32C6TouchLcd169DisplayDriver::drawRect(
    int16_t x,
    int16_t y,
    int16_t width,
    int16_t height,
    uint16_t color)
{
    _canvas->drawRect(
        x,
        y,
        width,
        height,
        color);
}

void WaveshareEsp32C6TouchLcd169DisplayDriver::fillRect(
    int16_t x,
    int16_t y,
    int16_t width,
    int16_t height,
    uint16_t color)
{
    _canvas->fillRect(
        x,
        y,
        width,
        height,
        color);
}

void WaveshareEsp32C6TouchLcd169DisplayDriver::drawCircle(
    int16_t x0,
    int16_t y0,
    int16_t radius,
    uint16_t color)
{
    _canvas->drawCircle(
        x0,
        y0,
        radius,
        color);
}

void WaveshareEsp32C6TouchLcd169DisplayDriver::fillCircle(
    int16_t x0,
    int16_t y0,
    int16_t radius,
    uint16_t color)
{
    _canvas->fillCircle(
        x0,
        y0,
        radius,
        color);
}

void WaveshareEsp32C6TouchLcd169DisplayDriver::drawTriangle(
    int16_t x0,
    int16_t y0,
    int16_t x1,
    int16_t y1,
    int16_t x2,
    int16_t y2,
    uint16_t color)
{
    _canvas->drawTriangle(
        x0, y0,
        x1, y1,
        x2, y2,
        color);
}

void WaveshareEsp32C6TouchLcd169DisplayDriver::fillTriangle(
    int16_t x0,
    int16_t y0,
    int16_t x1,
    int16_t y1,
    int16_t x2,
    int16_t y2,
    uint16_t color)
{
    _canvas->fillTriangle(
        x0, y0,
        x1, y1,
        x2, y2,
        color);
}

void WaveshareEsp32C6TouchLcd169DisplayDriver::setCursor(
    int16_t x,
    int16_t y)
{
    _canvas->setCursor(x, y);
}

void WaveshareEsp32C6TouchLcd169DisplayDriver::setTextColor(
    uint16_t color)
{
    _canvas->setTextColor(color);
}

void WaveshareEsp32C6TouchLcd169DisplayDriver::setTextSize(
    uint8_t size)
{
    _canvas->setTextSize(size);
}

void WaveshareEsp32C6TouchLcd169DisplayDriver::print(
    const std::string& text)
{
    _canvas->print(text.c_str());
}

#endif
