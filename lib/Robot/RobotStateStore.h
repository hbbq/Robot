#pragma once

#include <RobotMode.h>
#include <RobotMotion.h>

class RobotStateStore
{
public:
    void setState(
        RobotMode mode,
        RobotMotion motion)
    {
        _mode = mode;
        _motion = motion;
    }

    RobotMode mode() const
    {
        return _mode;
    }

    RobotMotion motion() const
    {
        return _motion;
    }

private:
    RobotMode _mode =
        RobotMode::Idle;

    RobotMotion _motion =
        RobotMotion::Stopped;
};