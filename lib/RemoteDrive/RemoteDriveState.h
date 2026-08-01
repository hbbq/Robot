#pragma once

#include <cstdint>

class RemoteDriveState
{
public:
    void setCommand(
        float linear,
        float angular,
        uint32_t receivedAtMs)
    {
        _linear = linear;
        _angular = angular;
        _receivedAtMs = receivedAtMs;
        _hasCommand = true;
    }

    float linear() const
    {
        return _linear;
    }

    float angular() const
    {
        return _angular;
    }

    uint32_t receivedAtMs() const
    {
        return _receivedAtMs;
    }

    bool hasCommand() const
    {
        return _hasCommand;
    }

private:
    float _linear = 0.0f;
    float _angular = 0.0f;

    uint32_t _receivedAtMs = 0;

    bool _hasCommand = false;
};