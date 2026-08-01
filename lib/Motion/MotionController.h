#pragma once

#include <cstdint>

#include <IDriveController.h>
#include <IMotionController.h>
#include <MotionControllerConfig.h>
#include <IClock.h>

enum class MotionState
{
    Idle,
    MovingForward,
    MovingBackward,
    TurningLeft,
    TurningRight
};

class MotionController : public IMotionController
{
public:
    MotionController(
        IDriveController& drive,
        IClock& clock,
        const MotionControllerConfig& config
    );

    void update() override;

    void goForward(float meters) override;
    void goBackward(float meters) override;

    void turnLeft(float degrees) override;
    void turnRight(float degrees) override;
    void turn(float degrees) override;

    void stop() override;

    bool isBusy() const override;

    MotionState getState() const;
    uint32_t getDurationMs() const;

private:
    IDriveController& _drive;
    IClock& _clock;

    const MotionControllerConfig& _config;

    MotionState _state = MotionState::Idle;

    uint32_t _startedAtMs = 0;
    uint32_t _durationMs = 0;

    void start(MotionState state, uint32_t durationMs);
};