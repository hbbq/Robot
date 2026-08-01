#pragma once

#include <RobotMode.h>

class RobotModeRequestStore
{
public:
    void request(RobotMode mode)
    {
        _requestedMode = mode;
        _pending = true;
    }

    bool hasPendingRequest() const
    {
        return _pending;
    }

    RobotMode requestedMode() const
    {
        return _requestedMode;
    }

    void clear()
    {
        _pending = false;
    }

private:
    RobotMode _requestedMode =
        RobotMode::Idle;

    bool _pending = false;
};