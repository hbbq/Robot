#ifdef DEVICE_ROBOT

#include "RobotApp.h"

#include "../AppConfig.h"

RobotApp::RobotApp()
    : _messageDispatcher(
          _deviceRegistry,
          _robotStateStore,
          _remoteDriveState,
            _robotModeRequestStore,
          _clock),

      _espNow(
          _messageDispatcher,
          AppConfig::Communication.wifiChannel),

      _deviceNetwork(
          _espNow,
          _deviceRegistry,
          _clock,
          AppConfig::Communication.network),

    #ifndef USE_FAKE_MOTORS
      _leftMotor(
          AppConfig::LeftMotor),

      _rightMotor(
          AppConfig::RightMotor),
    #endif

      _driveController(
          _leftMotor,
          _rightMotor),

    _motionController(
        _driveController,
        _clock,
        AppConfig::Motion),

      _statusLed(
          AppConfig::StatusLed),

      _readiness(
          _deviceRegistry,
          _statusLed,
          AppConfig::Readiness),

      _idleBehavior(
          _motionController),

        _randomDriveBehavior(
            _motionController,
            _clock,
            _random),

        _remoteControlBehavior(
            _remoteDriveState,
            _driveController,
            _clock)
{
}

void RobotApp::begin()
{
    Serial.begin(115200);
    delay(1000);

    _statusLed.begin();
    _driveController.begin();

    _readiness.begin();

    if (!_deviceNetwork.begin())
    {
        Serial.println("Network initialization failed");
        return;
    }
    
    _mode = RobotMode::Idle;

    _behaviorController.setBehavior(
        _idleBehavior);

    updateRobotState();
}

void RobotApp::update()
{
    _deviceNetwork.update();

    _readiness.update();

    handleModeRequest();

    _motionController.update();

    if (!_readiness.isReady())
    {
        _motionController.stop();
        _driveController.stop();
    }
    else
    {
        _behaviorController.update();
    }

    updateRobotState();
}

RobotActivity RobotApp::getActivity() const
{
    if (!_readiness.isReady())
    {
        return RobotActivity::NotReady;
    }

    switch (_motionController.getState())
    {
        case MotionState::MovingForward:
            return RobotActivity::MovingForward;

        case MotionState::MovingBackward:
            return RobotActivity::MovingBackward;

        case MotionState::TurningLeft:
            return RobotActivity::TurningLeft;

        case MotionState::TurningRight:
            return RobotActivity::TurningRight;

        case MotionState::Idle:
        default:
            return RobotActivity::Idle;
    }
}

void RobotApp::updateRobotState()
{
    const uint32_t nowMs =
        _clock.millis();

    const RobotMotion motion =
        _driveController.getMotion();

    const bool stateChanged =
        _mode != _lastSentMode ||
        motion != _lastSentMotion;

    const bool periodicUpdate =
        nowMs - _lastRobotStateSentMs >=
        RobotStateIntervalMs;

    if (!stateChanged &&
        !periodicUpdate)
    {
        return;
    }

    if (!_deviceNetwork.sendRobotState(
            _mode,
            motion))
    {
        return;
    }

    _lastSentMode = _mode;
    _lastSentMotion = motion;
    _lastRobotStateSentMs = nowMs;
}

void RobotApp::handleModeRequest()
{
    if (!_robotModeRequestStore.hasPendingRequest())
    {
        return;
    }

    const RobotMode requestedMode =
        _robotModeRequestStore.requestedMode();

    _robotModeRequestStore.clear();

    switch (requestedMode)
    {
        case RobotMode::Idle:
            _motionController.stop();
            _driveController.stop();

            _behaviorController.setBehavior(
                _idleBehavior);

            _mode = RobotMode::Idle;
            break;

        case RobotMode::Autonomous:
            _driveController.stop();

            _behaviorController.setBehavior(
                _randomDriveBehavior);

            _mode = RobotMode::Autonomous;
            break;

        case RobotMode::RemoteControl:
            _motionController.stop();
            _driveController.stop();

            _behaviorController.setBehavior(
                _remoteControlBehavior);

            _mode = RobotMode::RemoteControl;
            break;
    }

    Serial.printf(
        "[Robot] Mode changed: %u\n",
        static_cast<unsigned>(_mode));
}

#endif