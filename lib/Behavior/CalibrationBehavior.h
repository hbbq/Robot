#pragma once

#include <cstdint>

#include <CalibrationBehaviorConfig.h>
#include <CalibrationRequestStore.h>
#include <IBehavior.h>

class IMotionController;
class IDistanceSensor;
class DistanceSensorScanner;
class IClock;

class CalibrationBehavior : public IBehavior
{
public:
    CalibrationBehavior(
        IMotionController& motionController,
        IDistanceSensor& distanceSensor,
        DistanceSensorScanner& distanceSensorScanner,
        IClock& clock,
        const CalibrationBehaviorConfig& config);

    void begin() override;
    void update() override;
    RobotMode mode() const override;

    bool submitMotion(
        CalibrationMotionCommand command,
        float value);
    void stop();
    void refreshLease();
    bool leaseExpired() const;
    bool isBusy() const;

private:
    enum class State
    {
        Idle,
        CenteringForForward,
        MovingForward,
        MovingOther
    };

    IMotionController& _motionController;
    IDistanceSensor& _distanceSensor;
    DistanceSensorScanner& _distanceSensorScanner;
    IClock& _clock;
    const CalibrationBehaviorConfig& _config;

    State _state = State::Idle;
    float _pendingForwardMeters = 0.0f;
    uint32_t _lastLeaseRefreshMs = 0;
    bool _sensorReadingLost = false;
    uint32_t _sensorReadingLostAtMs = 0;

    void updateCenteringForForward();
    void updateMovingForward();
};
