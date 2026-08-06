#include <Arduino.h>
#include <unity.h>

#include <CalibrationBehavior.h>
#include <CalibrationRequestStore.h>
#include <DistanceSensorScanner.h>
#include <FakeClock.h>
#include <FakeDistanceSensor.h>
#include <FakeDriveController.h>
#include <FakeServoController.h>
#include <MotionController.h>

namespace
{
    constexpr MotionControllerConfig MotionConfig
    {
        .moveSpeed = 0.5f,
        .turnSpeed = 0.3f,
        .millisecondsPerMeter = 1000.0f,
        .millisecondsPerDegree = 10.0f
    };

    constexpr DistanceSensorPanConfig PanConfig
    {
        .centerAngle = 90.0f,
        .leftAngle = 120.0f,
        .rightAngle = 60.0f,
        .settleTimeMs = 100,
        .readingTimeoutMs = 100,
        .sampleFreshnessMs = 1000
    };

    constexpr CalibrationBehaviorConfig BehaviorConfig
    {
        .maximumDistanceMeters = 3.0f,
        .maximumTurnDegrees = 360.0f,
        .obstacleThresholdMillimeters = 250,
        .sensorLossTimeoutMs = 200,
        .sessionLeaseMs = 1000
    };

    struct Harness
    {
        FakeClock clock;
        FakeDriveController drive;
        FakeDistanceSensor sensor;
        FakeServoController servo;
        MotionController motion{drive, clock, MotionConfig};
        DistanceSensorScanner scanner{
            servo, sensor, clock, PanConfig};
        CalibrationBehavior behavior{
            motion, sensor, scanner, clock, BehaviorConfig};

        Harness()
        {
            behavior.begin();
        }

        void completeCenteredScan(uint16_t millimeters)
        {
            clock.advance(PanConfig.settleTimeMs);
            scanner.update();
            sensor.setReading(millimeters);
            scanner.update();
            behavior.update();
        }
    };
}

void test_forward_requires_fresh_centered_clear_reading()
{
    Harness harness;
    TEST_ASSERT_TRUE(harness.behavior.submitMotion(
        CalibrationMotionCommand::Forward, 1.0f));
    TEST_ASSERT_TRUE(harness.behavior.isBusy());
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::Stopped),
        static_cast<int>(harness.drive.getState()));

    harness.completeCenteredScan(1000);

    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::Forward),
        static_cast<int>(harness.drive.getState()));
}

void test_invalid_or_obstructed_center_reading_refuses_forward()
{
    Harness invalid;
    invalid.behavior.submitMotion(
        CalibrationMotionCommand::Forward, 1.0f);
    invalid.clock.advance(PanConfig.settleTimeMs);
    invalid.scanner.update();
    invalid.clock.advance(PanConfig.readingTimeoutMs);
    invalid.scanner.update();
    invalid.behavior.update();
    TEST_ASSERT_FALSE(invalid.behavior.isBusy());

    Harness obstructed;
    obstructed.behavior.submitMotion(
        CalibrationMotionCommand::Forward, 1.0f);
    obstructed.completeCenteredScan(200);
    TEST_ASSERT_FALSE(obstructed.behavior.isBusy());
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::Stopped),
        static_cast<int>(obstructed.drive.getState()));
}

void test_obstacle_or_sensor_loss_stops_active_forward()
{
    Harness obstacle;
    obstacle.behavior.submitMotion(
        CalibrationMotionCommand::Forward, 1.0f);
    obstacle.completeCenteredScan(1000);
    obstacle.sensor.setReading(200);
    obstacle.behavior.update();
    TEST_ASSERT_FALSE(obstacle.behavior.isBusy());

    Harness lost;
    lost.behavior.submitMotion(
        CalibrationMotionCommand::Forward, 1.0f);
    lost.completeCenteredScan(1000);
    lost.sensor.invalidate();
    lost.behavior.update();
    lost.clock.advance(BehaviorConfig.sensorLossTimeoutMs);
    lost.behavior.update();
    TEST_ASSERT_FALSE(lost.behavior.isBusy());
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::Stopped),
        static_cast<int>(lost.drive.getState()));
}

void test_busy_bounds_and_session_lease_are_enforced()
{
    Harness harness;
    TEST_ASSERT_FALSE(harness.behavior.submitMotion(
        CalibrationMotionCommand::Backward, 4.0f));
    TEST_ASSERT_FALSE(harness.behavior.submitMotion(
        CalibrationMotionCommand::TurnLeft, 361.0f));
    TEST_ASSERT_TRUE(harness.behavior.submitMotion(
        CalibrationMotionCommand::Backward, 1.0f));
    TEST_ASSERT_FALSE(harness.behavior.submitMotion(
        CalibrationMotionCommand::TurnRight, 90.0f));

    harness.clock.advance(BehaviorConfig.sessionLeaseMs);
    harness.behavior.update();
    TEST_ASSERT_TRUE(harness.behavior.leaseExpired());
    TEST_ASSERT_FALSE(harness.behavior.isBusy());
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::Stopped),
        static_cast<int>(harness.drive.getState()));
}

void test_stop_request_has_priority_over_pending_motion_and_enter()
{
    CalibrationRequestStore store;
    store.requestEnter();
    TEST_ASSERT_TRUE(store.requestMotion(
        CalibrationMotionCommand::Forward, 1.0f));
    store.requestStop();

    TEST_ASSERT_TRUE(store.takeStopRequest());
    TEST_ASSERT_FALSE(store.takeEnterRequest());
    TEST_ASSERT_FALSE(store.hasMotionRequest());
}

void setup()
{
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_forward_requires_fresh_centered_clear_reading);
    RUN_TEST(test_invalid_or_obstructed_center_reading_refuses_forward);
    RUN_TEST(test_obstacle_or_sensor_loss_stops_active_forward);
    RUN_TEST(test_busy_bounds_and_session_lease_are_enforced);
    RUN_TEST(test_stop_request_has_priority_over_pending_motion_and_enter);
    UNITY_END();
}

void loop()
{
}
