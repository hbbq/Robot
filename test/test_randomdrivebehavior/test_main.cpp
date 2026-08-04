#include <Arduino.h>
#include <unity.h>

#include <DistanceSensorScanner.h>
#include <FakeClock.h>
#include <FakeDistanceSensor.h>
#include <FakeDriveController.h>
#include <FakeRandom.h>
#include <FakeServoController.h>
#include <MotionController.h>
#include <RandomDriveBehavior.h>

namespace
{
    constexpr MotionControllerConfig MotionConfig
    {
        .moveSpeed = 0.5f,
        .turnSpeed = 0.4f,
        .millisecondsPerMeter = 1000.0f,
        .millisecondsPerDegree = 1.0f
    };

    constexpr DistanceSensorPanConfig PanConfig
    {
        .centerAngle = 90.0f,
        .leftAngle = 120.0f,
        .rightAngle = 60.0f,
        .settleTimeMs = 100,
        .readingTimeoutMs = 100
    };

    constexpr RandomDriveBehaviorConfig BehaviorConfig
    {
        .forwardChancePercent = 70,
        .minimumWaitMs = 500,
        .maximumWaitMs = 2000,
        .minimumForwardDistanceMeters = 0.2f,
        .maximumForwardDistanceMeters = 1.0f,
        .minimumTurnDegrees = 30.0f,
        .maximumTurnDegrees = 150.0f,
        .obstacleThresholdMillimeters = 250,
        .backupDistanceMeters = 0.1f,
        .minimumAvoidanceTurnDegrees = 60.0f,
        .maximumAvoidanceTurnDegrees = 120.0f,
        .sensorLossTimeoutMs = 200
    };

    struct Harness
    {
        FakeClock clock;
        FakeDistanceSensor sensor;
        FakeServoController servo;
        DistanceSensorScanner scanner{
            servo, sensor, clock, PanConfig};
        FakeDriveController drive;
        FakeRandom random;
        MotionController motion{
            drive, clock, MotionConfig};
        RandomDriveBehavior behavior{
            motion,
            sensor,
            scanner,
            clock,
            random,
            BehaviorConfig};

        void startForward()
        {
            random.addInt(500);
            random.addInt(0);
            random.addFloat(1.0f);
            behavior.begin();

            finishScanWithReading(1000, false);
            clock.advance(400);
            behavior.update();
        }

        void triggerObstacle()
        {
            sensor.setReading(200);
            behavior.update();
        }

        void finishBackup()
        {
            clock.advance(100);
            motion.update();
            behavior.update();
        }

        void finishScanWithReading(
            uint16_t distanceMillimeters,
            bool updateBehavior = true)
        {
            clock.advance(PanConfig.settleTimeMs);
            scanner.update();
            sensor.setReading(distanceMillimeters);
            scanner.update();

            if (updateBehavior)
            {
                behavior.update();
            }
        }

        void finishInvalidScan()
        {
            clock.advance(PanConfig.settleTimeMs);
            scanner.update();
            clock.advance(PanConfig.readingTimeoutMs);
            scanner.update();
            behavior.update();
        }
    };
}

void test_no_obstacle_keeps_forward_motion_active()
{
    Harness harness;
    harness.startForward();
    harness.sensor.setReading(1000);
    harness.behavior.update();

    TEST_ASSERT_TRUE(harness.motion.isBusy());
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::Forward),
        static_cast<int>(harness.drive.getState()));
}

void test_obstacle_stops_forward_and_backs_up()
{
    Harness harness;
    harness.startForward();
    harness.triggerObstacle();

    TEST_ASSERT_TRUE(harness.motion.isBusy());
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::Backward),
        static_cast<int>(harness.drive.getState()));
    TEST_ASSERT_EQUAL_UINT32(100, harness.motion.getDurationMs());
}

void test_backup_completion_starts_left_scan()
{
    Harness harness;
    harness.startForward();
    harness.triggerObstacle();
    harness.finishBackup();

    TEST_ASSERT_FALSE(harness.motion.isBusy());
    TEST_ASSERT_TRUE(harness.scanner.isBusy());
    TEST_ASSERT_FLOAT_WITHIN(
        0.001f, PanConfig.leftAngle, harness.servo.getAngle());
}

void test_clearer_left_side_turns_left()
{
    Harness harness;
    harness.startForward();
    harness.triggerObstacle();
    harness.finishBackup();
    harness.finishScanWithReading(900);
    harness.finishScanWithReading(350);
    harness.random.addFloat(90.0f);
    harness.finishScanWithReading(1000);

    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::RotateLeft),
        static_cast<int>(harness.drive.getState()));
    TEST_ASSERT_EQUAL_UINT32(90, harness.motion.getDurationMs());
}

void test_only_valid_right_side_turns_right()
{
    Harness harness;
    harness.startForward();
    harness.triggerObstacle();
    harness.finishBackup();
    harness.finishInvalidScan();
    harness.finishScanWithReading(700);
    harness.random.addFloat(80.0f);
    harness.finishScanWithReading(1000);

    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::RotateRight),
        static_cast<int>(harness.drive.getState()));
}

void test_no_valid_side_keeps_robot_stopped()
{
    Harness harness;
    harness.startForward();
    harness.triggerObstacle();
    harness.finishBackup();
    harness.finishInvalidScan();
    harness.finishInvalidScan();
    harness.finishScanWithReading(1000);

    TEST_ASSERT_FALSE(harness.motion.isBusy());
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::Stopped),
        static_cast<int>(harness.drive.getState()));
}

void test_invalid_center_reading_prevents_forward_motion()
{
    Harness harness;
    harness.random.addInt(500);
    harness.random.addInt(0);
    harness.behavior.begin();
    harness.finishInvalidScan();
    harness.clock.advance(400);
    harness.behavior.update();

    TEST_ASSERT_FALSE(harness.motion.isBusy());
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::Stopped),
        static_cast<int>(harness.drive.getState()));
}

void test_wait_timing_is_safe_across_millis_overflow()
{
    Harness harness;
    harness.clock.advance(UINT32_MAX - 100u);
    harness.random.addInt(200);
    harness.random.addInt(0);
    harness.random.addFloat(0.5f);
    harness.behavior.begin();
    harness.finishScanWithReading(1000, false);

    harness.clock.advance(99);
    harness.behavior.update();
    TEST_ASSERT_FALSE(harness.motion.isBusy());

    harness.clock.advance(1);
    harness.behavior.update();
    TEST_ASSERT_TRUE(harness.motion.isBusy());
}

void setup()
{
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_no_obstacle_keeps_forward_motion_active);
    RUN_TEST(test_obstacle_stops_forward_and_backs_up);
    RUN_TEST(test_backup_completion_starts_left_scan);
    RUN_TEST(test_clearer_left_side_turns_left);
    RUN_TEST(test_only_valid_right_side_turns_right);
    RUN_TEST(test_no_valid_side_keeps_robot_stopped);
    RUN_TEST(test_invalid_center_reading_prevents_forward_motion);
    RUN_TEST(test_wait_timing_is_safe_across_millis_overflow);
    UNITY_END();
}

void loop()
{
}
