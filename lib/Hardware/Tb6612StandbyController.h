#pragma once

#include <Tb6612StandbyConfig.h>

class Tb6612StandbyController
{
public:
    explicit Tb6612StandbyController(
        const Tb6612StandbyConfig& config);

    void begin();
    void enable();
    void disable();

private:
    const Tb6612StandbyConfig& _config;

    void writeEnabled(bool enabled);
};
