#ifdef DEVICE_ROBOT

#include "RobotApp.h"

#include "../AppConfig.h"

namespace
{
    const char* autonomousBehaviorText(
        AutonomousBehaviorType behavior)
    {
        switch (behavior)
        {
            case AutonomousBehaviorType::RandomDrive:
                return "RandomDrive";
            case AutonomousBehaviorType::Explore:
                return "Explore";
            case AutonomousBehaviorType::Dance:
                return "Dance";
        }

        return "Unknown";
    }
}

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
          _frontScanMeasurementStore,
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

      _frontServo(
          AppConfig::FrontServo),

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

      _distanceSensorScanner(
          _frontServo,
          _frontDistanceSensor,
          _clock,
          AppConfig::FrontDistanceSensorPan),

      _frontScanTelemetryPublisher(
          _distanceSensorScanner,
          _deviceNetwork,
          _clock,
          AppConfig::FrontScanTelemetry),

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
            _distanceSensorScanner,
            _clock,
            _random,
            AppConfig::AutonomousBehavior),

        _exploreBehavior(
            _motionController,
            _distanceSensorScanner,
            _clock,
            _random,
            AppConfig::ExploreBehavior),

        _danceBehavior(
            _motionController),

        _remoteControlBehavior(
            _remoteDriveState,
            _driveController,
            _clock,
            AppConfig::RemoteControl),

        _calibrationBehavior(
            _motionController,
            _frontDistanceSensor,
            _distanceSensorScanner,
            _clock,
            AppConfig::CalibrationBehavior),

        _calibrationWebServer(
            _calibrationRequestStore,
            _calibrationBehavior,
            _readiness,
            _behaviorController,
            _driveController,
            AppConfig::Motion,
            AppConfig::CalibrationBehavior,
            AppConfig::CalibrationWeb)
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
    _frontServo.begin();

    _distanceSensorFunctional =
        _frontDistanceSensor.begin();

    _distanceSensorScanner.begin();

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
    _calibrationWebServer.begin();
    
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

    _calibrationWebServer.update(
        _wifiConnection.isConnected());

    _readiness.update();
    handleReadinessTransition();

    handleCalibrationRequests();
    handleAutonomousBehaviorRequest();
    handleModeRequest();

    _frontDistanceSensor.update();
    _distanceSensorScanner.update();
    _frontScanTelemetryPublisher.update();
    _motionController.update();

    if (!_readiness.isReady())
    {
        _motionController.stop();
        _driveController.stop();

        if (_behaviorController.currentMode() !=
            RobotMode::Idle)
        {
            handleReadinessLost();
        }
    }
    else if (_behaviorController.currentMode() ==
                 RobotMode::Calibration &&
             !_wifiConnection.isConnected())
    {
        exitCalibration();
    }
    else
    {
        _behaviorController.update();

        if (_behaviorController.currentMode() ==
                RobotMode::Calibration &&
            _calibrationBehavior.leaseExpired())
        {
            Serial.println(
                "[Calibration] Session lease expired");
            exitCalibration();
        }
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

    const RobotMode currentMode =
        _behaviorController.currentMode();

    const bool selectedBehaviorNeedsDistance =
        _selectedAutonomousBehavior ==
            AutonomousBehaviorType::RandomDrive ||
        _selectedAutonomousBehavior ==
            AutonomousBehaviorType::Explore;

    Serial.printf(
        "[Robot] Mode request: requested=%u current=%u ready=%u "
        "selectedAuto=%s defaultAuto=%s requiresDistance=%u "
        "distanceFunctional=%u\n",
        static_cast<unsigned>(requestedMode),
        static_cast<unsigned>(currentMode),
        _readiness.isReady() ? 1u : 0u,
        autonomousBehaviorText(_selectedAutonomousBehavior),
        autonomousBehaviorText(AutonomousBehaviorType::RandomDrive),
        selectedBehaviorNeedsDistance ? 1u : 0u,
        _distanceSensorFunctional ? 1u : 0u);

    _robotModeRequestStore.clear();
    _distanceSensorScanner.lookCenter();

    switch (requestedMode)
    {
        case RobotMode::Idle:
            _motionController.stop();
            _driveController.stop();

            _behaviorController.setBehavior(
                _idleBehavior);

            break;

        case RobotMode::Autonomous:
            if (!_readiness.isReady())
            {
                _motionController.stop();
                _driveController.stop();
                Serial.println(
                    "[Robot] Autonomous request rejected: not ready");
                break;
            }

            if (!autonomousBehaviorIsAvailable(
                    _selectedAutonomousBehavior))
            {
                _motionController.stop();
                _driveController.stop();

                _behaviorController.setBehavior(
                    _idleBehavior);

                Serial.println(
                    "[Robot] Autonomous request rejected: selected "
                    "behavior requires unavailable distance sensor");

                break;
            }

            _driveController.stop();

            _behaviorController.setBehavior(
                selectedAutonomousBehavior());

            Serial.printf(
                "[Robot] Autonomous request accepted: behavior=%s\n",
                autonomousBehaviorText(
                    _selectedAutonomousBehavior));

            break;

        case RobotMode::RemoteControl:
            _motionController.stop();
            _driveController.stop();

            _behaviorController.setBehavior(
                _remoteControlBehavior);

            break;

        case RobotMode::Calibration:
            // Calibration can only be entered through the local web
            // session, never through the ESP-NOW mode request protocol.
            Serial.println(
                "[Robot] Remote calibration mode request rejected");
            break;
    }

    Serial.printf(
        "[Robot] Mode changed: %u\n",
        static_cast<unsigned>(
            _behaviorController.currentMode()));
}

void RobotApp::handleReadinessTransition()
{
    const bool ready = _readiness.isReady();

    if (_wasReady && !ready)
    {
        handleReadinessLost();
    }
    else if (!_wasReady && ready)
    {
        Serial.printf(
            "[Robot] Readiness restored: mode=%u selectedAuto=%s; "
            "waiting for explicit mode request\n",
            static_cast<unsigned>(
                _behaviorController.currentMode()),
            autonomousBehaviorText(
                _selectedAutonomousBehavior));
    }

    _wasReady = ready;
}

void RobotApp::handleReadinessLost()
{
    const RobotMode previousMode =
        _behaviorController.currentMode();
    const bool clearedModeRequest =
        _robotModeRequestStore.hasPendingRequest();

    _motionController.stop();
    _driveController.stop();
    _distanceSensorScanner.lookCenter();
    _robotModeRequestStore.clear();
    _behaviorController.setBehavior(_idleBehavior);

    Serial.printf(
        "[Robot] Readiness lost: previousMode=%u -> Idle, "
        "motion stopped, modeRequestCleared=%u, "
        "selectedAuto=%s preserved, autonomousRequestPending=%u\n",
        static_cast<unsigned>(previousMode),
        clearedModeRequest ? 1u : 0u,
        autonomousBehaviorText(_selectedAutonomousBehavior),
        _autonomousBehaviorRequestStore.hasPendingRequest()
            ? 1u
            : 0u);
}

void RobotApp::handleCalibrationRequests()
{
    if (_calibrationRequestStore.takeStopRequest())
    {
        _calibrationBehavior.stop();
        _motionController.stop();
        _driveController.stop();
        return;
    }

    if (_calibrationRequestStore.takeExitRequest())
    {
        if (_behaviorController.currentMode() ==
            RobotMode::Calibration)
        {
            exitCalibration();
        }
    }

    if (_calibrationRequestStore.takeEnterRequest())
    {
        if (_readiness.isReady() &&
            _wifiConnection.isConnected() &&
            _distanceSensorFunctional)
        {
            _motionController.stop();
            _driveController.stop();
            _distanceSensorScanner.lookCenter();
            _behaviorController.setBehavior(
                _calibrationBehavior);
        }
        else
        {
            Serial.println(
                "[Calibration] Enter request rejected: not ready");
        }
    }

    if (_calibrationRequestStore.takeKeepAliveRequest() &&
        _behaviorController.currentMode() ==
            RobotMode::Calibration &&
        _wifiConnection.isConnected())
    {
        _calibrationBehavior.refreshLease();
    }

    if (!_calibrationRequestStore.hasMotionRequest())
    {
        return;
    }

    const CalibrationMotionRequest request =
        _calibrationRequestStore.takeMotionRequest();

    if (_behaviorController.currentMode() !=
            RobotMode::Calibration ||
        !_readiness.isReady() ||
        !_wifiConnection.isConnected() ||
        _calibrationBehavior.leaseExpired())
    {
        Serial.println(
            "[Calibration] Motion request rejected: session inactive");
        return;
    }

    if (!_calibrationBehavior.submitMotion(
            request.command,
            request.value))
    {
        Serial.println(
            "[Calibration] Motion request rejected: busy or invalid");
    }
}

void RobotApp::exitCalibration()
{
    _calibrationBehavior.stop();
    _motionController.stop();
    _driveController.stop();
    _distanceSensorScanner.lookCenter();
    _behaviorController.setBehavior(_idleBehavior);
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
        _distanceSensorScanner.lookCenter();
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
