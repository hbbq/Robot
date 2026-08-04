#ifdef DEVICE_ROBOT

#include "Vl53l0xDistanceSensor.h"

#include <Arduino.h>
#include <Wire.h>

#include <IClock.h>

Vl53l0xDistanceSensor::Vl53l0xDistanceSensor(
    IClock& clock,
    const Vl53l0xDistanceSensorConfig& config)
    : _clock(clock),
      _config(config)
{
}

bool Vl53l0xDistanceSensor::begin()
{
    _initialized = false;
    _hasReading = false;

    if (!Wire.begin(
            _config.sdaPin,
            _config.sclPin))
    {
        return false;
    }

    Wire.setClock(_config.i2cFrequencyHz);

    _sensor.setBus(&Wire);
    _sensor.setTimeout(
        _config.measurementTimeoutMs);

    if (!_sensor.init())
    {
        return false;
    }

    if (!_sensor.setMeasurementTimingBudget(
            _config.measurementTimingBudgetUs))
    {
        return false;
    }

    _sensor.startContinuous(
        _config.measurementPeriodMs);

    _lastMeasurementStartedMs =
        _clock.millis();
    _initialized = true;

    return true;
}

void Vl53l0xDistanceSensor::update()
{
    if (!_initialized)
    {
        return;
    }

    const uint32_t nowMs =
        _clock.millis();

    if (nowMs - _lastMeasurementStartedMs <
        _config.measurementPeriodMs)
    {
        return;
    }

    _lastMeasurementStartedMs = nowMs;

    const uint16_t distance =
        _sensor.readRangeContinuousMillimeters();

    if (_sensor.timeoutOccurred())
    {
        _hasReading = false;
        return;
    }

    _distanceMillimeters = distance;
    _lastValidReadingMs = _clock.millis();
    _hasReading = true;

    Serial.print("Distance: ");
    Serial.print(_distanceMillimeters);
    Serial.print(" mm, Freshness: ");
    Serial.print(_clock.millis() - _lastValidReadingMs);
    Serial.println(" ms");  
}

bool Vl53l0xDistanceSensor::hasValidReading() const
{
    return
        _initialized &&
        _hasReading &&
        _clock.millis() - _lastValidReadingMs <=
            _config.readingFreshnessMs;
}

uint16_t Vl53l0xDistanceSensor::distanceMillimeters() const
{
    return _distanceMillimeters;
}

#endif
