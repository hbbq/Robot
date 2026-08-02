#include "WaveshareEsp32C6TouchLcd147DisplayDriver.h"

#ifdef HAS_DISPLAY

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

#include <algorithm>

namespace
{
    constexpr int16_t DisplayWidth = 172;
    constexpr int16_t DisplayHeight = 320;

    constexpr uint8_t LcdSckPin = 1;
    constexpr uint8_t LcdMosiPin = 2;
    constexpr uint8_t LcdCsPin = 14;
    constexpr uint8_t LcdDcPin = 15;
    constexpr uint8_t LcdResetPin = 22;
    constexpr uint8_t LcdBacklightPin = 23;

    constexpr bool IsIps = false;

    constexpr int16_t ColumnOffsetRotation0 = 34;
    constexpr int16_t RowOffsetRotation0 = 0;

    constexpr int16_t ColumnOffsetRotation2 = 34;
    constexpr int16_t RowOffsetRotation2 = 0;

    constexpr uint32_t BacklightPwmFrequency = 5000;
    constexpr uint8_t BacklightPwmResolutionBits = 8;

    constexpr uint32_t BacklightMaxDuty =
        (1u << BacklightPwmResolutionBits) - 1u;
}

WaveshareEsp32C6TouchLcd147DisplayDriver::
WaveshareEsp32C6TouchLcd147DisplayDriver(
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

WaveshareEsp32C6TouchLcd147DisplayDriver::
~WaveshareEsp32C6TouchLcd147DisplayDriver() = default;

void WaveshareEsp32C6TouchLcd147DisplayDriver::begin()
{
    _gfx->begin();

    _canvas = std::make_unique<Arduino_Canvas>(
        _gfx->width(),
        _gfx->height(),
        _gfx.get());

    _canvas->begin();

    // Arduino_Canvas::begin() reinitializes the underlying display.
    // Panel-specific initialization must therefore happen afterwards.
    initializePanelRegisters();

    _canvas->setRotation(_rotation);
    _canvas->setTextWrap(false);

    ledcAttach(
        LcdBacklightPin,
        BacklightPwmFrequency,
        BacklightPwmResolutionBits);

    clear(0x0000);
    flush();
    
    // Keep the backlight off until application policy has selected a
    // safe initial brightness.
    setBrightness(0);
}

void WaveshareEsp32C6TouchLcd147DisplayDriver::flush()
{
    _canvas->flush();
}

int16_t WaveshareEsp32C6TouchLcd147DisplayDriver::width() const
{
    return _canvas
        ? _canvas->width()
        : _gfx->width();
}

int16_t WaveshareEsp32C6TouchLcd147DisplayDriver::height() const
{
    return _canvas
        ? _canvas->height()
        : _gfx->height();
}

void WaveshareEsp32C6TouchLcd147DisplayDriver::setRotation(
    uint8_t rotation)
{
    _rotation = rotation;

    if (_canvas)
    {
        _canvas->setRotation(rotation);
    }
}

void WaveshareEsp32C6TouchLcd147DisplayDriver::setBrightness(
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

void WaveshareEsp32C6TouchLcd147DisplayDriver::clear(
    uint16_t color)
{
    _canvas->fillScreen(color);
}

void WaveshareEsp32C6TouchLcd147DisplayDriver::drawPixel(
    int16_t x,
    int16_t y,
    uint16_t color)
{
    _canvas->drawPixel(x, y, color);
}

void WaveshareEsp32C6TouchLcd147DisplayDriver::drawLine(
    int16_t x0,
    int16_t y0,
    int16_t x1,
    int16_t y1,
    uint16_t color)
{
    _canvas->drawLine(
        x0,
        y0,
        x1,
        y1,
        color);
}

void WaveshareEsp32C6TouchLcd147DisplayDriver::drawRect(
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

void WaveshareEsp32C6TouchLcd147DisplayDriver::fillRect(
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

void WaveshareEsp32C6TouchLcd147DisplayDriver::drawCircle(
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

void WaveshareEsp32C6TouchLcd147DisplayDriver::fillCircle(
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

void WaveshareEsp32C6TouchLcd147DisplayDriver::drawTriangle(
    int16_t x0,
    int16_t y0,
    int16_t x1,
    int16_t y1,
    int16_t x2,
    int16_t y2,
    uint16_t color)
{
    _canvas->drawTriangle(
        x0,
        y0,
        x1,
        y1,
        x2,
        y2,
        color);
}

void WaveshareEsp32C6TouchLcd147DisplayDriver::fillTriangle(
    int16_t x0,
    int16_t y0,
    int16_t x1,
    int16_t y1,
    int16_t x2,
    int16_t y2,
    uint16_t color)
{
    _canvas->fillTriangle(
        x0,
        y0,
        x1,
        y1,
        x2,
        y2,
        color);
}

void WaveshareEsp32C6TouchLcd147DisplayDriver::setCursor(
    int16_t x,
    int16_t y)
{
    _canvas->setCursor(x, y);
}

void WaveshareEsp32C6TouchLcd147DisplayDriver::setTextColor(
    uint16_t color)
{
    _canvas->setTextColor(color);
}

void WaveshareEsp32C6TouchLcd147DisplayDriver::setTextSize(
    uint8_t size)
{
    _canvas->setTextSize(size);
}

void WaveshareEsp32C6TouchLcd147DisplayDriver::print(
    const std::string& text)
{
    _canvas->print(text.c_str());
}

void WaveshareEsp32C6TouchLcd147DisplayDriver::
initializePanelRegisters()
{
    static const uint8_t initOperations[] =
    {
        BEGIN_WRITE,
        WRITE_COMMAND_8, 0x11,
        END_WRITE,

        DELAY, 120,

        BEGIN_WRITE,

        WRITE_C8_D16, 0xDF, 0x98, 0x53,
        WRITE_C8_D8,  0xB2, 0x23,

        WRITE_COMMAND_8, 0xB7,
        WRITE_BYTES, 4,
        0x00, 0x47, 0x00, 0x6F,

        WRITE_COMMAND_8, 0xBB,
        WRITE_BYTES, 6,
        0x1C, 0x1A, 0x55, 0x73, 0x63, 0xF0,

        WRITE_C8_D16, 0xC0, 0x44, 0xA4,
        WRITE_C8_D8,  0xC1, 0x16,

        WRITE_COMMAND_8, 0xC3,
        WRITE_BYTES, 8,
        0x7D, 0x07, 0x14, 0x06,
        0xCF, 0x71, 0x72, 0x77,

        WRITE_COMMAND_8, 0xC4,
        WRITE_BYTES, 12,
        0x00, 0x00, 0xA0, 0x79,
        0x0B, 0x0A, 0x16, 0x79,
        0x0B, 0x0A, 0x16, 0x82,

        WRITE_COMMAND_8, 0xC8,
        WRITE_BYTES, 32,

        0x3F, 0x32, 0x29, 0x29,
        0x27, 0x2B, 0x27, 0x28,
        0x28, 0x26, 0x25, 0x17,
        0x12, 0x0D, 0x04, 0x00,

        0x3F, 0x32, 0x29, 0x29,
        0x27, 0x2B, 0x27, 0x28,
        0x28, 0x26, 0x25, 0x17,
        0x12, 0x0D, 0x04, 0x00,

        WRITE_COMMAND_8, 0xD0,
        WRITE_BYTES, 5,
        0x04, 0x06, 0x6B, 0x0F, 0x00,

        WRITE_C8_D16, 0xD7, 0x00, 0x30,
        WRITE_C8_D8,  0xE6, 0x14,
        WRITE_C8_D8,  0xDE, 0x01,

        WRITE_COMMAND_8, 0xB7,
        WRITE_BYTES, 5,
        0x03, 0x13, 0xEF, 0x35, 0x35,

        WRITE_COMMAND_8, 0xC1,
        WRITE_BYTES, 3,
        0x14, 0x15, 0xC0,

        WRITE_C8_D16, 0xC2, 0x06, 0x3A,
        WRITE_C8_D16, 0xC4, 0x72, 0x12,
        WRITE_C8_D8,  0xBE, 0x00,
        WRITE_C8_D8,  0xDE, 0x02,

        WRITE_COMMAND_8, 0xE5,
        WRITE_BYTES, 3,
        0x00, 0x02, 0x00,

        WRITE_COMMAND_8, 0xE5,
        WRITE_BYTES, 3,
        0x01, 0x02, 0x00,

        WRITE_C8_D8, 0xDE, 0x00,
        WRITE_C8_D8, 0x35, 0x00,

        // RGB565
        WRITE_C8_D8, 0x3A, 0x05,

        // Column address: 34-205
        WRITE_COMMAND_8, 0x2A,
        WRITE_BYTES, 4,
        0x00, 0x22, 0x00, 0xCD,

        // Row address: 0-319
        WRITE_COMMAND_8, 0x2B,
        WRITE_BYTES, 4,
        0x00, 0x00, 0x01, 0x3F,

        WRITE_C8_D8, 0xDE, 0x02,

        WRITE_COMMAND_8, 0xE5,
        WRITE_BYTES, 3,
        0x00, 0x02, 0x00,

        WRITE_C8_D8, 0xDE, 0x00,

        WRITE_C8_D8, 0x36, 0x00,

        // Display inversion on
        WRITE_COMMAND_8, 0x21,

        END_WRITE,

        DELAY, 10,

        BEGIN_WRITE,
        WRITE_COMMAND_8, 0x29,
        END_WRITE
    };

    _bus->batchOperation(
        initOperations,
        sizeof(initOperations));
}

#endif
