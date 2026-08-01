#include "JoystickModel.h"

#include <algorithm>
#include <cmath>

JoystickModel::JoystickModel(
    int16_t centerX,
    int16_t centerY,
    int16_t radius,
    float deadZone)
    : _centerX(centerX),
      _centerY(centerY),
      _radius(radius),
      _deadZone(deadZone),
      _knobX(centerX),
      _knobY(centerY)
{
}

void JoystickModel::setTouch(
    int16_t x,
    int16_t y)
{
    float dx =
        static_cast<float>(x - _centerX);

    float dy =
        static_cast<float>(y - _centerY);

    const float distance =
        std::sqrt(dx * dx + dy * dy);

    if (distance > _radius)
    {
        const float scale =
            static_cast<float>(_radius) /
            distance;

        dx *= scale;
        dy *= scale;
    }

    _knobX =
        _centerX +
        static_cast<int16_t>(dx);

    _knobY =
        _centerY +
        static_cast<int16_t>(dy);

    _linear =
        -dy /
        static_cast<float>(_radius);

    _angular =
        dx /
        static_cast<float>(_radius);

    if (std::abs(_linear) < _deadZone)
    {
        _linear = 0.0f;
    }

    if (std::abs(_angular) < _deadZone)
    {
        _angular = 0.0f;
    }

    _linear =
        std::clamp(
            _linear,
            -1.0f,
            1.0f);

    _angular =
        std::clamp(
            _angular,
            -1.0f,
            1.0f);

    _active = true;
}

void JoystickModel::release()
{
    _active = false;

    _linear = 0.0f;
    _angular = 0.0f;

    _knobX = _centerX;
    _knobY = _centerY;
}

bool JoystickModel::active() const
{
    return _active;
}

float JoystickModel::linear() const
{
    return _linear;
}

float JoystickModel::angular() const
{
    return _angular;
}

int16_t JoystickModel::knobX() const
{
    return _knobX;
}

int16_t JoystickModel::knobY() const
{
    return _knobY;
}

int16_t JoystickModel::centerX() const
{
    return _centerX;
}

int16_t JoystickModel::centerY() const
{
    return _centerY;
}

int16_t JoystickModel::radius() const
{
    return _radius;
}