#pragma once

enum class CalibrationMotionCommand
{
    Forward,
    Backward,
    TurnLeft,
    TurnRight
};

struct CalibrationMotionRequest
{
    CalibrationMotionCommand command;
    float value;
};

class CalibrationRequestStore
{
public:
    void requestEnter()
    {
        _enterRequested = true;
    }

    void requestExit()
    {
        _exitRequested = true;
    }

    void requestKeepAlive()
    {
        _keepAliveRequested = true;
    }

    void requestStop()
    {
        _stopRequested = true;
        _enterRequested = false;
        _motionPending = false;
    }

    bool requestMotion(
        CalibrationMotionCommand command,
        float value)
    {
        if (_motionPending || _stopRequested)
        {
            return false;
        }

        _motionRequest = {command, value};
        _motionPending = true;
        return true;
    }

    bool takeEnterRequest()
    {
        return take(_enterRequested);
    }

    bool takeExitRequest()
    {
        return take(_exitRequested);
    }

    bool takeKeepAliveRequest()
    {
        return take(_keepAliveRequested);
    }

    bool takeStopRequest()
    {
        return take(_stopRequested);
    }

    bool hasMotionRequest() const
    {
        return _motionPending;
    }

    CalibrationMotionRequest takeMotionRequest()
    {
        _motionPending = false;
        return _motionRequest;
    }

    void clearMotionRequest()
    {
        _motionPending = false;
    }

private:
    bool _enterRequested = false;
    bool _exitRequested = false;
    bool _keepAliveRequested = false;
    bool _stopRequested = false;
    bool _motionPending = false;
    CalibrationMotionRequest _motionRequest{
        CalibrationMotionCommand::Forward,
        0.0f};

    static bool take(bool& value)
    {
        const bool result = value;
        value = false;
        return result;
    }
};
