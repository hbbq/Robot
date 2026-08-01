#pragma once

class IBehavior
{
public:
    virtual ~IBehavior() = default;

    virtual void begin() = 0;
    virtual void update() = 0;
};