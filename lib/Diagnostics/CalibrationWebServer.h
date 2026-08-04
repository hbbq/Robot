#pragma once

#ifdef DEVICE_ROBOT

#include <WebServer.h>

#include <CalibrationBehaviorConfig.h>
#include <CalibrationWebServerConfig.h>

class CalibrationRequestStore;
class CalibrationBehavior;
class ReadinessController;
class BehaviorController;
class DriveController;
struct MotionControllerConfig;

class CalibrationWebServer
{
public:
    CalibrationWebServer(
        CalibrationRequestStore& requestStore,
        CalibrationBehavior& calibrationBehavior,
        ReadinessController& readiness,
        BehaviorController& behaviorController,
        DriveController& driveController,
        const MotionControllerConfig& motionConfig,
        const CalibrationBehaviorConfig& behaviorConfig,
        const CalibrationWebServerConfig& serverConfig);

    void begin();
    void update(bool wifiConnected);

private:
    CalibrationRequestStore& _requestStore;
    CalibrationBehavior& _calibrationBehavior;
    ReadinessController& _readiness;
    BehaviorController& _behaviorController;
    DriveController& _driveController;
    const MotionControllerConfig& _motionConfig;
    const CalibrationBehaviorConfig& _behaviorConfig;
    const CalibrationWebServerConfig& _serverConfig;
    WebServer _server;
    bool _routesConfigured = false;
    bool _running = false;

    void configureRoutes();
    void handleStatus();
    void handleSession();
    void handleMotion();
    void handleStop();
    bool parsePositiveValue(float& value);
};

#endif
