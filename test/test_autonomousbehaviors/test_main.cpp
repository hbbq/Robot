#include <Arduino.h>
#include <unity.h>

#include <AutonomousBehaviorRequestStore.h>
#include <DanceBehavior.h>
#include <ExploreBehavior.h>
#include <FakeClock.h>
#include <FakeDistanceSensor.h>
#include <FakeDriveController.h>
#include <FakeRandom.h>
#include <FakeServoController.h>
#include <DistanceSensorScanner.h>
#include <MotionController.h>

namespace
{
    constexpr MotionControllerConfig MotionConfig
    {
        .moveSpeed = 0.5f,
        .turnSpeed = 0.4f,
        .millisecondsPerMeter = 1000.0f,
        .millisecondsPerDegree = 1.0f
    };

    constexpr ExploreBehaviorConfig ExploreConfig
    {
        .obstacleThresholdMillimeters = 250,
        .backupDistanceMeters = 0.1f,
        .minimumAvoidanceTurnDegrees = 60.0f,
        .maximumAvoidanceTurnDegrees = 100.0f,
        .sensorLossTimeoutMs = 200,
        .minimumForwardDistanceMeters = 1.0f,
        .maximumForwardDistanceMeters = 2.0f,
        .courseCorrectionChancePercent = 20,
        .minimumCourseCorrectionDegrees = 20.0f,
        .maximumCourseCorrectionDegrees = 40.0f,
        .pauseBetweenMovesMs = 100
    };

    constexpr DistanceSensorPanConfig PanConfig
    {
        .centerAngle = 90.0f,
        .leftAngle = 120.0f,
        .rightAngle = 60.0f,
        .settleTimeMs = 10,
        .readingTimeoutMs = 10,
        .sampleFreshnessMs = 1000
    };

    void completeSample(
        DistanceSensorScanner& scanner,
        FakeDistanceSensor& sensor,
        FakeClock& clock,
        uint16_t millimeters)
    {
        clock.advance(PanConfig.settleTimeMs);
        scanner.update();
        sensor.setReading(millimeters);
        scanner.update();
    }

    void primeClearFront(
        DistanceSensorScanner& scanner,
        FakeDistanceSensor& sensor,
        FakeClock& clock)
    {
        completeSample(scanner, sensor, clock, 1000);
        completeSample(scanner, sensor, clock, 1000);
        completeSample(scanner, sensor, clock, 1000);
        completeSample(scanner, sensor, clock, 1000);
    }

    void primeBlockedFront(
        DistanceSensorScanner& scanner,
        FakeDistanceSensor& sensor,
        FakeClock& clock)
    {
        completeSample(scanner, sensor, clock, 200);
        completeSample(scanner, sensor, clock, 200);
        completeSample(scanner, sensor, clock, 200);
        completeSample(scanner, sensor, clock, 200);
    }
}

void test_explore_starts_a_long_forward_movement()
{
    FakeClock clock;
    FakeDistanceSensor sensor;
    FakeDriveController drive;
    FakeRandom random;
    FakeServoController servo;
    DistanceSensorScanner scanner(
        servo, sensor, clock, PanConfig);
    MotionController motion(drive, clock, MotionConfig);
    ExploreBehavior behavior(
        motion, scanner, clock, random, ExploreConfig);

    random.addFloat(1.5f);
    behavior.begin();
    primeClearFront(scanner, sensor, clock);
    clock.advance(60);

    TEST_ASSERT_TRUE(scanner.isContinuousSweepActive());
    TEST_ASSERT_TRUE(scanner.isBusy());
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(FrontScanAssessment::Clear),
        static_cast<int>(scanner.assessFront(
            ExploreConfig.obstacleThresholdMillimeters)));

    behavior.update();

    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::Forward),
        static_cast<int>(drive.getState()));
    TEST_ASSERT_EQUAL_UINT32(1500, motion.getDurationMs());
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RobotMode::Autonomous),
        static_cast<int>(behavior.mode()));
    TEST_ASSERT_TRUE(scanner.isContinuousSweepActive());
}

void test_explore_obstacle_starts_safe_avoidance()
{
    FakeClock clock;
    FakeDistanceSensor sensor;
    FakeDriveController drive;
    FakeRandom random;
    FakeServoController servo;
    DistanceSensorScanner scanner(
        servo, sensor, clock, PanConfig);
    MotionController motion(drive, clock, MotionConfig);
    ExploreBehavior behavior(
        motion, scanner, clock, random, ExploreConfig);

    random.addFloat(1.5f);
    behavior.begin();
    primeClearFront(scanner, sensor, clock);
    clock.advance(60);
    behavior.update();

    completeSample(scanner, sensor, clock, 200);
    behavior.update();

    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::Backward),
        static_cast<int>(drive.getState()));

    random.addFloat(90.0f);
    random.addInt(1);
    clock.advance(100);
    motion.update();
    behavior.update();

    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::RotateRight),
        static_cast<int>(drive.getState()));
}

void test_explore_waiting_with_complete_obstacle_scan_starts_recovery()
{
    FakeClock clock;
    FakeDistanceSensor sensor;
    FakeDriveController drive;
    FakeRandom random;
    FakeServoController servo;
    DistanceSensorScanner scanner(
        servo, sensor, clock, PanConfig);
    MotionController motion(drive, clock, MotionConfig);
    ExploreBehavior behavior(
        motion, scanner, clock, random, ExploreConfig);

    behavior.begin();
    primeBlockedFront(scanner, sensor, clock);
    clock.advance(60);

    TEST_ASSERT_TRUE(scanner.isContinuousSweepActive());
    TEST_ASSERT_TRUE(scanner.isBusy());
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(FrontScanAssessment::Obstacle),
        static_cast<int>(scanner.assessFront(
            ExploreConfig.obstacleThresholdMillimeters)));

    behavior.update();

    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::Backward),
        static_cast<int>(drive.getState()));
    TEST_ASSERT_TRUE(motion.isBusy());
    TEST_ASSERT_FALSE(scanner.isContinuousSweepActive());
}

void test_explore_does_not_start_without_sensor_reading()
{
    FakeClock clock;
    FakeDistanceSensor sensor;
    FakeDriveController drive;
    FakeRandom random;
    FakeServoController servo;
    DistanceSensorScanner scanner(
        servo, sensor, clock, PanConfig);
    MotionController motion(drive, clock, MotionConfig);
    ExploreBehavior behavior(
        motion, scanner, clock, random, ExploreConfig);

    sensor.invalidate();
    behavior.begin();
    clock.advance(1000);
    behavior.update();

    TEST_ASSERT_FALSE(motion.isBusy());
}

void test_explore_reacts_to_left_sector_obstacle_with_clear_center()
{
    FakeClock clock;
    FakeDistanceSensor sensor;
    FakeDriveController drive;
    FakeRandom random;
    FakeServoController servo;
    DistanceSensorScanner scanner(
        servo, sensor, clock, PanConfig);
    MotionController motion(drive, clock, MotionConfig);
    ExploreBehavior behavior(
        motion, scanner, clock, random, ExploreConfig);

    random.addFloat(1.5f);
    behavior.begin();
    primeClearFront(scanner, sensor, clock);
    clock.advance(60);
    behavior.update();

    completeSample(scanner, sensor, clock, 1000);
    behavior.update();
    completeSample(scanner, sensor, clock, 200);
    behavior.update();

    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::Backward),
        static_cast<int>(drive.getState()));
}

void test_dance_advances_without_blocking()
{
    FakeClock clock;
    FakeDriveController drive;
    MotionController motion(drive, clock, MotionConfig);
    DanceBehavior behavior(motion);

    behavior.begin();
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::RotateLeft),
        static_cast<int>(drive.getState()));

    clock.advance(60);
    motion.update();
    behavior.update();

    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::RotateRight),
        static_cast<int>(drive.getState()));
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RobotMode::Autonomous),
        static_cast<int>(behavior.mode()));
}

void test_autonomous_behavior_request_store_is_non_authoritative()
{
    AutonomousBehaviorRequestStore store;

    store.request(AutonomousBehaviorType::Explore);

    TEST_ASSERT_TRUE(store.hasPendingRequest());
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(AutonomousBehaviorType::Explore),
        static_cast<int>(store.requestedBehavior()));

    store.clear();
    TEST_ASSERT_FALSE(store.hasPendingRequest());
}

void setup()
{
    delay(2000);
    UNITY_BEGIN();

    RUN_TEST(test_explore_starts_a_long_forward_movement);
    RUN_TEST(test_explore_obstacle_starts_safe_avoidance);
    RUN_TEST(test_explore_waiting_with_complete_obstacle_scan_starts_recovery);
    RUN_TEST(test_explore_does_not_start_without_sensor_reading);
    RUN_TEST(test_explore_reacts_to_left_sector_obstacle_with_clear_center);
    RUN_TEST(test_dance_advances_without_blocking);
    RUN_TEST(test_autonomous_behavior_request_store_is_non_authoritative);

    UNITY_END();
}

void loop()
{
}
