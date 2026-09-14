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
#include <Tb6612StandbyConfig.h>
#include <ServoControllerConfig.h>
#include <DistanceSensorPanConfig.h>
#include <CalibrationBehaviorConfig.h>
#include <CalibrationWebServerConfig.h>
#include <FrontScanTelemetryConfig.h>
#include <soc/soc_caps.h>

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
            .pwmChannel = 0,
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
            .commandTimeoutMs = 500,
            .maxLinearSpeed = 0.65,
            .maxAngularSpeed = 0.45f
        };

        inline constexpr RobotStateReportingConfig RobotStateReporting
        {
            .snapshotIntervalMs = 2000
        };

        inline constexpr FrontScanTelemetryConfig FrontScanTelemetry
        {
            .minimumSendIntervalMs = 400,
            .snapshotIntervalMs = 2000
        };

        inline constexpr CalibrationBehaviorConfig CalibrationBehavior
        {
            .maximumDistanceMeters = 3.0f,
            .maximumTurnDegrees = 360.0f,
            .obstacleThresholdMillimeters = 250,
            .sensorLossTimeoutMs = 200,
            .sessionLeaseMs = 3000
        };

        inline constexpr CalibrationWebServerConfig CalibrationWeb
        {
            .port = 80
        };

        inline constexpr Tb6612StandbyConfig MotorStandby
        {
            .pin = 23,
            .activeHigh = true
        };

        inline constexpr ServoControllerConfig FrontServo
        {
            .pin = 10,
            .pwmChannel = 4,
            .minimumPulseMicroseconds = 530,
            .centerPulseMicroseconds = 1500,
            .maximumPulseMicroseconds = 2610,
            .minimumAngleDegrees = 0.0f,
            .maximumAngleDegrees = 180.0f,
            .centerAngleDegrees = 90.0f
        };

        inline constexpr DistanceSensorPanConfig FrontDistanceSensorPan
        {
            .centerAngle = 90.0f,
            .leftAngle = 120.0f,
            .rightAngle = 60.0f,
            .settleTimeMs = 250,
            .readingTimeoutMs = 200,
            .sampleFreshnessMs = 2500
        };

        inline constexpr Tb6612MotorControllerConfig LeftMotor
        {
            .in1Pin = 20,
            .in2Pin = 21,
            .pwmPin = 22,
            .pwmChannel = 2,
            .inverted = false,
            .pwmFrequency = 20000,
            .pwmResolutionBits = 8,
            .minimumSpeed = 0.0f
        };

        inline constexpr Tb6612MotorControllerConfig RightMotor
        {
            .in1Pin = 4,
            .in2Pin = 5,
            .pwmPin = 6,
            .pwmChannel = 3,
            .inverted = true,
            .pwmFrequency = 20000,
            .pwmResolutionBits = 8,
            .minimumSpeed = 0.0f
        };

        // Arduino-ESP32 3.x assigns one LEDC timer to each channel pair.
        inline constexpr uint8_t ledcTimerForChannel(
            uint8_t channel)
        {
            return channel / 2;
        }

        static_assert(
            StatusLedHardware.pwmChannel < SOC_LEDC_CHANNEL_NUM &&
            LeftMotor.pwmChannel < SOC_LEDC_CHANNEL_NUM &&
            RightMotor.pwmChannel < SOC_LEDC_CHANNEL_NUM &&
            FrontServo.pwmChannel < SOC_LEDC_CHANNEL_NUM,
            "Configured LEDC channel is unavailable on this target");

        static_assert(
            StatusLedHardware.pwmChannel != LeftMotor.pwmChannel &&
            StatusLedHardware.pwmChannel != RightMotor.pwmChannel &&
            StatusLedHardware.pwmChannel != FrontServo.pwmChannel &&
            LeftMotor.pwmChannel != RightMotor.pwmChannel &&
            LeftMotor.pwmChannel != FrontServo.pwmChannel &&
            RightMotor.pwmChannel != FrontServo.pwmChannel,
            "Robot LEDC consumers must use unique channels");

        static_assert(
            LeftMotor.pwmFrequency == RightMotor.pwmFrequency &&
            LeftMotor.pwmResolutionBits ==
                RightMotor.pwmResolutionBits,
            "Motor channels sharing a timer require matching PWM settings");

        static_assert(
            ledcTimerForChannel(LeftMotor.pwmChannel) ==
                ledcTimerForChannel(RightMotor.pwmChannel),
            "Left and right motors should share their compatible LEDC timer");

        static_assert(
            ledcTimerForChannel(StatusLedHardware.pwmChannel) !=
                ledcTimerForChannel(LeftMotor.pwmChannel) &&
            ledcTimerForChannel(StatusLedHardware.pwmChannel) !=
                ledcTimerForChannel(FrontServo.pwmChannel) &&
            ledcTimerForChannel(LeftMotor.pwmChannel) !=
                ledcTimerForChannel(FrontServo.pwmChannel),
            "Incompatible Robot PWM consumers must use different timers");

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
            .pwmChannel = 0,
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

    inline constexpr MotionControllerConfig Motion
    {
        .moveSpeed = 0.65f,
        .turnSpeed = 0.45f,
        .millisecondsPerMeter = 2540.0f,
        .millisecondsPerDegree = 10.0f
    };
}
