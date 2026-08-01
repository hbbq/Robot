#pragma once

class IClock
{
public:
    virtual ~IClock() = default;

    virtual unsigned long millis() const = 0;
};