#pragma once

#include <cstdint>

#include <IDistanceSensor.h>
#include <VL53L0X.h>
#include <Vl53l0xDistanceSensorConfig.h>

class IClock;

class Vl53l0xDistanceSensor : public IDistanceSensor
{
public:
    Vl53l0xDistanceSensor(
        IClock& clock,
        const Vl53l0xDistanceSensorConfig& config);

    bool begin() override;
    void update() override;

    bool hasValidReading() const override;
    uint16_t distanceMillimeters() const override;

private:
    IClock& _clock;
    const Vl53l0xDistanceSensorConfig& _config;
    VL53L0X _sensor;

    bool _initialized = false;
    bool _hasReading = false;
    uint16_t _distanceMillimeters = 0;
    uint32_t _lastMeasurementStartedMs = 0;
    uint32_t _lastValidReadingMs = 0;
};
