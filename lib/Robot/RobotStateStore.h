#pragma once

#include <RobotMode.h>
#include <RobotMotion.h>
#include <AutonomousBehaviorType.h>

class RobotStateStore
{
public:
    void setState(
        RobotMode mode,
        RobotMotion motion,
        AutonomousBehaviorType autonomousBehavior)
    {
        _mode = mode;
        _motion = motion;
        _autonomousBehavior = autonomousBehavior;
    }

    RobotMode mode() const
    {
        return _mode;
    }

    RobotMotion motion() const
    {
        return _motion;
    }

    AutonomousBehaviorType autonomousBehavior() const
    {
        return _autonomousBehavior;
    }

private:
    RobotMode _mode =
        RobotMode::Idle;

    RobotMotion _motion =
        RobotMotion::Stopped;

    AutonomousBehaviorType _autonomousBehavior =
        AutonomousBehaviorType::RandomDrive;
};
