#pragma once

#include <RobotMode.h>

class IBehavior
{
public:
    virtual ~IBehavior() = default;

    virtual void begin() = 0;
    virtual void update() = 0;
    virtual RobotMode mode() const = 0;
};
