#pragma once

#include <ArduinoClock.h>
#include <DeviceRegistry.h>
#include <MessageDispatcher.h>
#include <EspNowManager.h>
#include <DeviceNetworkService.h>
#include <ReadinessController.h>
#include <DriveController.h>
#include <LedController.h>
#include <IdleBehavior.h>
#include <BehaviorController.h>
#include <RandomDriveBehavior.h>
#include <ArduinoRandom.h>
#include <MotionController.h>
#include <RobotStateStore.h>
#include <RemoteDriveState.h>
#include <RobotMode.h>
#include <RobotMotion.h>
#include <RemoteControlBehavior.h>
#include <RobotModeRequestStore.h>
#include <AutonomousBehaviorType.h>
#include <AutonomousBehaviorRequestStore.h>
#include <ExploreBehavior.h>
#include <DanceBehavior.h>

#if defined(USE_FAKE_MOTORS)
#include <FakeMotorController.h>
using RobotMotorController = FakeMotorController;
#else
#include <Tb6612MotorController.h>
using RobotMotorController = Tb6612MotorController;
#endif

#if defined(USE_FAKE_DISTANCE_SENSOR)
#include <FakeDistanceSensor.h>
using RobotDistanceSensor = FakeDistanceSensor;
#else
#include <Vl53l0xDistanceSensor.h>
using RobotDistanceSensor = Vl53l0xDistanceSensor;
#endif

class RobotApp
{
public:
    RobotApp();

    void begin();
    void update();

private:
    ArduinoClock _clock;

    DeviceRegistry _deviceRegistry;
    RemoteDriveState _remoteDriveState;
    MessageDispatcher _messageDispatcher;
    EspNowManager _espNow;
    DeviceNetworkService _deviceNetwork;
    RobotMotorController  _leftMotor;
    RobotMotorController  _rightMotor;
    DriveController _driveController;
    MotionController _motionController;
    RobotDistanceSensor _frontDistanceSensor;
    ReadinessController _readiness;
    LedController _statusLed;
    IdleBehavior _idleBehavior;
    ArduinoRandom _random;
    RandomDriveBehavior _randomDriveBehavior;
    ExploreBehavior _exploreBehavior;
    DanceBehavior _danceBehavior;
    BehaviorController _behaviorController;
    RemoteControlBehavior _remoteControlBehavior;

    bool _distanceSensorFunctional = false;
    
    RobotStateStore _robotStateStore;

    AutonomousBehaviorType _selectedAutonomousBehavior =
        AutonomousBehaviorType::RandomDrive;

    RobotMode _lastSentMode = RobotMode::Idle;
    RobotMotion _lastSentMotion = RobotMotion::Stopped;
    AutonomousBehaviorType _lastSentAutonomousBehavior =
        AutonomousBehaviorType::RandomDrive;

    RobotModeRequestStore _robotModeRequestStore;
    AutonomousBehaviorRequestStore _autonomousBehaviorRequestStore;

    static constexpr uint32_t RobotStateIntervalMs = 2000;

    uint32_t _lastRobotStateSentMs = 0;

    RobotActivity getActivity() const;
    void updateRobotState();
    void handleModeRequest();
    void handleAutonomousBehaviorRequest();
    IBehavior& selectedAutonomousBehavior();
    bool autonomousBehaviorIsAvailable(
        AutonomousBehaviorType behavior) const;

    // Senare:
    // MotionController _motion;
    // BehaviorController _behaviors;
};
