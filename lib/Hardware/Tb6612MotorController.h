#pragma once

#include <IMotorController.h>
#include <Tb6612MotorControllerConfig.h>

class Tb6612MotorController : public IMotorController
{
public:
    explicit Tb6612MotorController(
        const Tb6612MotorControllerConfig& config
    );

    void begin() override;

    void setSpeed(float speed) override;
    float getSpeed() const override;

    void stop() override;

private:
    const Tb6612MotorControllerConfig& _config;

    float _speed = 0.0f;
    uint32_t _maxDuty = 0;

    void setDirection(bool forward);
    void writePwm(float magnitude);
};