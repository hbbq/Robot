#pragma once

#include <IServoController.h>
#include <ServoControllerConfig.h>

class Esp32ServoController : public IServoController
{
public:
    explicit Esp32ServoController(
        const ServoControllerConfig& config);

    void begin() override;
    void setAngle(float degrees) override;
    float getAngle() const override;

private:
    const ServoControllerConfig& _config;
    float _angleDegrees;
    uint32_t _maximumDuty = 0;

    void writePulse(uint32_t pulseMicroseconds);
};
