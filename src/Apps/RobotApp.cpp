#ifdef DEVICE_ROBOT

#include "RobotApp.h"

#include "../AppConfig.h"

RobotApp::RobotApp()
    : _robotStateReportingConfig(
          AppConfig::RobotStateReporting),

      _wifiConnection(
          _clock,
          AppConfig::InternetWifi),

      _timeService(
          _clock,
          AppConfig::WallClockTime),

      _deviceRegistry(
          AppConfig::SystemRobotId),

      _messageDispatcher(
          _deviceRegistry,
          _robotStateStore,
          _remoteDriveState,
            _robotModeRequestStore,
          _autonomousBehaviorRequestStore,
          _clock),

      _espNow(
          _messageDispatcher,
          AppConfig::Communication.wifiChannel),

      _deviceNetwork(
          _espNow,
          _deviceRegistry,
          _clock,
          AppConfig::Communication.network),

      _motorStandby(
          AppConfig::MotorStandby),

      _leftMotor(
          AppConfig::LeftMotor),

      _rightMotor(
          AppConfig::RightMotor),

      _driveController(
          _leftMotor,
          _rightMotor),

    _motionController(
        _driveController,
        _clock,
        AppConfig::Motion),

    #ifndef USE_FAKE_DISTANCE_SENSOR
      _frontDistanceSensor(
          _clock,
          AppConfig::FrontDistanceSensor),
    #endif

      _statusLed(
          AppConfig::StatusLedHardware,
          AppConfig::StatusLedAnimation),

      _readiness(
          _deviceRegistry,
          _statusLed,
          AppConfig::Readiness),

      _idleBehavior(
          _motionController),

        _randomDriveBehavior(
            _motionController,
            _frontDistanceSensor,
            _clock,
            _random,
            AppConfig::AutonomousBehavior),

        _exploreBehavior(
            _motionController,
            _frontDistanceSensor,
            _clock,
            _random,
            AppConfig::ExploreBehavior),

        _danceBehavior(
            _motionController),

        _remoteControlBehavior(
            _remoteDriveState,
            _driveController,
            _clock,
            AppConfig::RemoteControl)
{
}

void RobotApp::begin()
{
    Serial.begin(115200);
    delay(1000);

    _statusLed.begin();
    _motorStandby.begin();
    _driveController.begin();
    _motorStandby.enable();

    _distanceSensorFunctional =
        _frontDistanceSensor.begin();

    #ifdef USE_FAKE_DISTANCE_SENSOR
    if (_distanceSensorFunctional)
    {
        _frontDistanceSensor.setReading(1000);
    }
    #endif

    if (_distanceSensorFunctional)
    {
        _deviceNetwork.setCapabilities(
            Capability::Motors |
            Capability::Distance);
    }
    else
    {
        Serial.println(
            "[Robot] Front distance sensor initialization failed");
    }

    _readiness.begin();

    if (!_deviceNetwork.begin())
    {
        Serial.println("Network initialization failed");
        return;
    }

    _wifiConnection.begin();
    _timeService.begin();
    
    _behaviorController.setBehavior(
        _idleBehavior);

    updateRobotState();
}

void RobotApp::update()
{
    _deviceNetwork.update();
    _wifiConnection.update();
    _timeService.update(
        _wifiConnection.isConnected());

    _readiness.update();

    handleAutonomousBehaviorRequest();
    handleModeRequest();

    _frontDistanceSensor.update();
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

    const RobotMode mode =
        _behaviorController.currentMode();

    const bool stateChanged =
        mode != _lastSentMode ||
        motion != _lastSentMotion ||
        _selectedAutonomousBehavior !=
            _lastSentAutonomousBehavior;

    const bool periodicUpdate =
        nowMs - _lastRobotStateSentMs >=
        _robotStateReportingConfig.snapshotIntervalMs;

    if (!stateChanged &&
        !periodicUpdate)
    {
        return;
    }

    if (!_deviceNetwork.sendRobotState(
            mode,
            motion,
            _selectedAutonomousBehavior))
    {
        return;
    }

    _lastSentMode = mode;
    _lastSentMotion = motion;
    _lastSentAutonomousBehavior =
        _selectedAutonomousBehavior;
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

            break;

        case RobotMode::Autonomous:
            if (!autonomousBehaviorIsAvailable(
                    _selectedAutonomousBehavior))
            {
                _motionController.stop();
                _driveController.stop();

                _behaviorController.setBehavior(
                    _idleBehavior);

                break;
            }

            _driveController.stop();

            _behaviorController.setBehavior(
                selectedAutonomousBehavior());

            break;

        case RobotMode::RemoteControl:
            _motionController.stop();
            _driveController.stop();

            _behaviorController.setBehavior(
                _remoteControlBehavior);

            break;
    }

    Serial.printf(
        "[Robot] Mode changed: %u\n",
        static_cast<unsigned>(
            _behaviorController.currentMode()));
}

void RobotApp::handleAutonomousBehaviorRequest()
{
    if (!_autonomousBehaviorRequestStore.hasPendingRequest())
    {
        return;
    }

    const AutonomousBehaviorType requestedBehavior =
        _autonomousBehaviorRequestStore.requestedBehavior();

    _autonomousBehaviorRequestStore.clear();

    if (!autonomousBehaviorIsAvailable(requestedBehavior))
    {
        Serial.printf(
            "[Robot] Autonomous behavior rejected: %u\n",
            static_cast<unsigned>(requestedBehavior));
        return;
    }

    if (requestedBehavior == _selectedAutonomousBehavior)
    {
        return;
    }

    const bool currentlyAutonomous =
        _behaviorController.currentMode() ==
            RobotMode::Autonomous;

    if (currentlyAutonomous)
    {
        _motionController.stop();
        _driveController.stop();
    }

    _selectedAutonomousBehavior = requestedBehavior;

    if (currentlyAutonomous)
    {
        _behaviorController.setBehavior(
            selectedAutonomousBehavior());
    }

    Serial.printf(
        "[Robot] Autonomous behavior selected: %u\n",
        static_cast<unsigned>(_selectedAutonomousBehavior));
}

IBehavior& RobotApp::selectedAutonomousBehavior()
{
    switch (_selectedAutonomousBehavior)
    {
        case AutonomousBehaviorType::RandomDrive:
            return _randomDriveBehavior;

        case AutonomousBehaviorType::Explore:
            return _exploreBehavior;

        case AutonomousBehaviorType::Dance:
            return _danceBehavior;

        default:
            return _randomDriveBehavior;
    }
}

bool RobotApp::autonomousBehaviorIsAvailable(
    AutonomousBehaviorType behavior) const
{
    switch (behavior)
    {
        case AutonomousBehaviorType::RandomDrive:
        case AutonomousBehaviorType::Explore:
            return _distanceSensorFunctional;

        case AutonomousBehaviorType::Dance:
            return true;

        default:
            return false;
    }
}

#endif
