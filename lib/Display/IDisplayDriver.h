#pragma once

#include <cstdint>
#include <string>

class IDisplayDriver
{
public:
    virtual ~IDisplayDriver() = default;

    // Lifecycle
    virtual void begin() = 0;
    virtual void flush() = 0;

    // Display
    virtual int16_t width() const = 0;
    virtual int16_t height() const = 0;

    virtual void setRotation(
        uint8_t rotation) = 0;

    virtual void setBrightness(
        uint8_t percentage) = 0;

    // Screen
    virtual void clear(
        uint16_t color) = 0;

    // Primitives
    virtual void drawPixel(
        int16_t x,
        int16_t y,
        uint16_t color) = 0;

    virtual void drawLine(
        int16_t x0,
        int16_t y0,
        int16_t x1,
        int16_t y1,
        uint16_t color) = 0;

    virtual void drawRect(
        int16_t x,
        int16_t y,
        int16_t w,
        int16_t h,
        uint16_t color) = 0;

    virtual void fillRect(
        int16_t x,
        int16_t y,
        int16_t w,
        int16_t h,
        uint16_t color) = 0;

    virtual void drawCircle(
        int16_t x0,
        int16_t y0,
        int16_t r,
        uint16_t color) = 0;

    virtual void fillCircle(
        int16_t x0,
        int16_t y0,
        int16_t r,
        uint16_t color) = 0;

    virtual void drawTriangle(
        int16_t x0,
        int16_t y0,
        int16_t x1,
        int16_t y1,
        int16_t x2,
        int16_t y2,
        uint16_t color) = 0;

    virtual void fillTriangle(
        int16_t x0,
        int16_t y0,
        int16_t x1,
        int16_t y1,
        int16_t x2,
        int16_t y2,
        uint16_t color) = 0;

    // Text
    virtual void setCursor(
        int16_t x,
        int16_t y) = 0;

    virtual void setTextColor(
        uint16_t color) = 0;

    virtual void setTextSize(
        uint8_t size) = 0;

    virtual void print(
        const std::string& text) = 0;
};