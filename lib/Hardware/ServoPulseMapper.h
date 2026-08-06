#pragma once

#include <ServoControllerConfig.h>

#include <algorithm>
#include <cmath>
#include <cstdint>

inline uint32_t servoPulseMicrosecondsForAngle(
    float degrees,
    const ServoControllerConfig& config)
{
    const float safeDegrees = std::isfinite(degrees)
        ? degrees
        : config.centerAngleDegrees;
    const float clampedDegrees = std::clamp(
        safeDegrees,
        config.minimumAngleDegrees,
        config.maximumAngleDegrees);

    float pulseMicroseconds =
        static_cast<float>(config.centerPulseMicroseconds);

    if (clampedDegrees <= config.centerAngleDegrees)
    {
        const float angleRange =
            config.centerAngleDegrees -
            config.minimumAngleDegrees;
        const float normalized =
            angleRange > 0.0f
                ? (clampedDegrees - config.minimumAngleDegrees) /
                    angleRange
                : 0.0f;

        pulseMicroseconds =
            static_cast<float>(config.minimumPulseMicroseconds) +
            normalized *
                (static_cast<float>(config.centerPulseMicroseconds) -
                 static_cast<float>(config.minimumPulseMicroseconds));
    }
    else
    {
        const float angleRange =
            config.maximumAngleDegrees -
            config.centerAngleDegrees;
        const float normalized =
            angleRange > 0.0f
                ? (clampedDegrees - config.centerAngleDegrees) /
                    angleRange
                : 0.0f;

        pulseMicroseconds =
            static_cast<float>(config.centerPulseMicroseconds) +
            normalized *
                (static_cast<float>(config.maximumPulseMicroseconds) -
                 static_cast<float>(config.centerPulseMicroseconds));
    }

    return static_cast<uint32_t>(std::lround(pulseMicroseconds));
}
