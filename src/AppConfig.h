#pragma once

#include <Tb6612MotorControllerConfig.h>
#include <MotionControllerConfig.h>
#include <LedHardwareConfig.h>
#include <LedAnimationConfig.h>
#include <CommunicationConfig.h>
#include <ReadinessConfig.h>
#include <Vl53l0xDistanceSensorConfig.h>
#include <RandomDriveBehaviorConfig.h>
#include <ExploreBehaviorConfig.h>
#include <BrightnessControllerConfig.h>
#include <WifiConnectionConfig.h>
#include <NtpTimeServiceConfig.h>
#include <RemoteControlBehaviorConfig.h>
#include <RobotStateReportingConfig.h>
#include <JoystickConfig.h>
#include <DriveCommandTransmissionConfig.h>
#include <RemoteUiConfig.h>
#include <FixedBrightnessConfig.h>

#if __has_include("LocalSecrets.h")
    #include "LocalSecrets.h"
#else
    #include "LocalSecretsFallback.h"
#endif

namespace AppConfig
{
    inline constexpr uint8_t WifiChannel = 3;

    #ifdef DEVICE_ROBOT

        inline constexpr RobotId SystemRobotId = 1;

        inline constexpr LedHardwareConfig StatusLedHardware
        {
            .pin = 7,
            .activeHigh = true,
            .pwmFrequencyHz = 5000,
            .pwmResolutionBits = 8
        };

        inline constexpr LedAnimationConfig StatusLedAnimation
        {
            .updateIntervalMs = 15,
            .slowBlinkIntervalMs = 500,
            .fastBlinkIntervalMs = 150,
            .pulseDurationMs = 1500,
            .maximumBrightness = 1.0f,
            .minimumPulseBrightness = 0.05f
        };

        inline constexpr RemoteControlBehaviorConfig RemoteControl
        {
            .commandTimeoutMs = 500
        };

        inline constexpr RobotStateReportingConfig RobotStateReporting
        {
            .snapshotIntervalMs = 2000
        };

        inline constexpr WifiConnectionConfig InternetWifi
        {
            .ssid = LocalSecrets::WifiSsid,
            .password = LocalSecrets::WifiPassword,
            .expectedChannel = WifiChannel,
            .reconnectIntervalMs = 30000
        };

        // Sweden: CET in winter and CEST in summer.
        inline constexpr NtpTimeServiceConfig WallClockTime
        {
            .timezone = "CET-1CEST,M3.5.0,M10.5.0/3",
            .primaryServer = "pool.ntp.org",
            .secondaryServer = "time.cloudflare.com",
            .synchronizationCheckIntervalMs = 1000,
            .resyncIntervalMs = 21600000
        };

        inline constexpr CommunicationConfig Communication
        {
            .wifiChannel = WifiChannel,

            .network =
            {
                .robotId = SystemRobotId,
                .deviceType = DeviceType::Robot,

                .capabilities =
                    Capability::Motors,

                .announcementIntervalMs = 10000,
                .heartbeatIntervalMs = 1000,
                .deviceTimeoutMs = 3000
            }
        };

        inline constexpr ReadinessConfig Readiness
{
            .requiredDeviceType =
                DeviceType::Display,

            .requiredCapabilities =
                Capability::Display,

            .startingLedMode =
                LedMode::SlowBlink,

            .readyLedMode =
                LedMode::On,

            .notReadyLedMode =
                LedMode::Pulse
        };

        // Provisional front VL53L0X wiring. GPIO18/19 are currently
        // unused by the Robot and avoid motor, LED, USB and UART pins.
        inline constexpr Vl53l0xDistanceSensorConfig FrontDistanceSensor
        {
            .sdaPin = 18,
            .sclPin = 19,
            .i2cFrequencyHz = 400000,
            .measurementPeriodMs = 50,
            .measurementTimeoutMs = 25,
            .measurementTimingBudgetUs = 20000,
            .readingFreshnessMs = 150
        };

        inline constexpr RandomDriveBehaviorConfig AutonomousBehavior
        {
            .forwardChancePercent = 70,
            .minimumWaitMs = 500,
            .maximumWaitMs = 2000,
            .minimumForwardDistanceMeters = 0.2f,
            .maximumForwardDistanceMeters = 1.0f,
            .minimumTurnDegrees = 30.0f,
            .maximumTurnDegrees = 150.0f,
            .obstacleThresholdMillimeters = 250,
            .backupDistanceMeters = 0.15f,
            .minimumAvoidanceTurnDegrees = 60.0f,
            .maximumAvoidanceTurnDegrees = 120.0f,
            .sensorLossTimeoutMs = 200
        };

        inline constexpr ExploreBehaviorConfig ExploreBehavior
        {
            .obstacleThresholdMillimeters = 250,
            .backupDistanceMeters = 0.15f,
            .minimumAvoidanceTurnDegrees = 55.0f,
            .maximumAvoidanceTurnDegrees = 110.0f,
            .sensorLossTimeoutMs = 200,
            .minimumForwardDistanceMeters = 0.8f,
            .maximumForwardDistanceMeters = 1.8f,
            .courseCorrectionChancePercent = 20,
            .minimumCourseCorrectionDegrees = 20.0f,
            .maximumCourseCorrectionDegrees = 45.0f,
            .pauseBetweenMovesMs = 250
        };

    #elifdef DEVICE_DISPLAY

        inline constexpr RobotId SystemRobotId = 1;

        inline constexpr LedHardwareConfig StatusLedHardware
        {
            .pin = 7,
            .activeHigh = true,
            .pwmFrequencyHz = 5000,
            .pwmResolutionBits = 8
        };

        inline constexpr LedAnimationConfig StatusLedAnimation
        {
            .updateIntervalMs = 15,
            .slowBlinkIntervalMs = 500,
            .fastBlinkIntervalMs = 150,
            .pulseDurationMs = 1500,
            .maximumBrightness = 1.0f,
            .minimumPulseBrightness = 0.05f
        };

        inline constexpr BrightnessControllerConfig DisplayBrightness
        {
            .minimumPercent = 1,
            .maximumPercent = 50,
            .fadeDurationMs = 1500
        };

        inline constexpr CommunicationConfig Communication
        {
            .wifiChannel = WifiChannel,

            .network =
            {
                .robotId = SystemRobotId,
                .deviceType = DeviceType::Display,

                .capabilities =
                    Capability::Display |
                    Capability::Touch,

                .announcementIntervalMs = 10000,
                .heartbeatIntervalMs = 1000,
                .deviceTimeoutMs = 3000
            }
        };
        
        inline constexpr ReadinessConfig Readiness
        {
            .requiredDeviceType =
                DeviceType::Robot,

            .requiredCapabilities =
                Capability::Motors,

            .startingLedMode =
                LedMode::SlowBlink,

            .readyLedMode =
                LedMode::On,

            .notReadyLedMode =
                LedMode::Pulse
        };

    #elifdef DEVICE_REMOTE

        inline constexpr RobotId SystemRobotId = 1;

        inline constexpr JoystickConfig Joystick
        {
            .deadZone = 0.12f
        };

        inline constexpr DriveCommandTransmissionConfig DriveTransmission
        {
            .sendIntervalMs = 100
        };

        inline constexpr RemoteUiConfig RemoteUi
        {
            .selectionReleaseMs = 150
        };

        inline constexpr FixedBrightnessConfig RemoteBrightness
        {
            .percentage = 50
        };

        inline constexpr CommunicationConfig Communication
        {
            .wifiChannel = WifiChannel,

            .network =
            {
                .robotId = SystemRobotId,
                .deviceType = DeviceType::Remote,

                .capabilities =
                    Capability::RemoteControl |
                    Capability::Logging,

                .announcementIntervalMs = 10000,
                .heartbeatIntervalMs = 1000,
                .deviceTimeoutMs = 3000
            }
        };
        
        inline constexpr ReadinessConfig Readiness
        {
            .requiredDeviceType =
                DeviceType::Robot,

            .requiredCapabilities =
                Capability::Motors,

            .startingLedMode =
                LedMode::SlowBlink,

            .readyLedMode =
                LedMode::On,

            .notReadyLedMode =
                LedMode::Pulse
        };


    #else
        #error "No device type selected"
    #endif

    inline constexpr uint8_t MotorStandbyPin = 10;

    inline constexpr Tb6612MotorControllerConfig LeftMotor
    {
        .in1Pin = 2,
        .in2Pin = 3,
        .pwmPin = 4,
        .inverted = false,
        .pwmFrequency = 20000,
        .pwmResolutionBits = 8,
        .minimumSpeed = 0.0f
    };

    inline constexpr Tb6612MotorControllerConfig RightMotor
    {
        .in1Pin = 5,
        .in2Pin = 6,
        .pwmPin = 7,
        .inverted = true,
        .pwmFrequency = 20000,
        .pwmResolutionBits = 8,
        .minimumSpeed = 0.0f
    };

    inline constexpr MotionControllerConfig Motion
    {
        .moveSpeed = 0.5f,
        .turnSpeed = 0.4f,
        .millisecondsPerMeter = 2000.0f,
        .millisecondsPerDegree = 8.0f
    };
}
