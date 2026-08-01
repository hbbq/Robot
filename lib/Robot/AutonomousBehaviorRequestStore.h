#pragma once

#include <AutonomousBehaviorType.h>

class AutonomousBehaviorRequestStore
{
public:
    void request(
        AutonomousBehaviorType behavior)
    {
        _requestedBehavior = behavior;
        _pending = true;
    }

    bool hasPendingRequest() const
    {
        return _pending;
    }

    AutonomousBehaviorType requestedBehavior() const
    {
        return _requestedBehavior;
    }

    void clear()
    {
        _pending = false;
    }

private:
    AutonomousBehaviorType _requestedBehavior =
        AutonomousBehaviorType::RandomDrive;

    bool _pending = false;
};
