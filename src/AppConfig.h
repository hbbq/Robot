#pragma once

#include <Tb6612MotorControllerConfig.h>
#include <MotionControllerConfig.h>
#include <LedControllerConfig.h>
#include <CommunicationConfig.h>
#include <ReadinessConfig.h>

namespace AppConfig
{
    inline constexpr uint8_t WifiChannel = 6;

    #ifdef DEVICE_ROBOT

        inline constexpr CommunicationConfig Communication
        {
            .wifiChannel = WifiChannel,

            .network =
            {
                .deviceType = DeviceType::Robot,

                .capabilities =
                    Capability::Motors |
                    Capability::Distance,

                .announcementIntervalMs = 10000,
                .heartbeatIntervalMs = 1000,
                .deviceTimeoutMs = 3000
            }
        };

        inline constexpr ReadinessConfig Readiness
{
            .requiredCapabilities =
                Capability::Display,

            .startingLedMode =
                LedMode::SlowBlink,

            .readyLedMode =
                LedMode::On,

            .notReadyLedMode =
                LedMode::Pulse
        };

    #elifdef DEVICE_DISPLAY

        inline constexpr CommunicationConfig Communication
        {
            .wifiChannel = WifiChannel,

            .network =
            {
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

        inline constexpr CommunicationConfig Communication
        {
            .wifiChannel = WifiChannel,

            .network =
            {
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

    inline constexpr LedControllerConfig StatusLed
    {
        .pin = 7,
    };

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