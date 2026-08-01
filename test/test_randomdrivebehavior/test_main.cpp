#include <Arduino.h>
#include <unity.h>

#include <FakeClock.h>
#include <FakeDistanceSensor.h>
#include <FakeDriveController.h>
#include <FakeRandom.h>
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

    constexpr RandomDriveBehaviorConfig BehaviorConfig
    {
        .obstacleThresholdMillimeters = 250,
        .backupDistanceMeters = 0.1f,
        .minimumAvoidanceTurnDegrees = 60.0f,
        .maximumAvoidanceTurnDegrees = 120.0f,
        .sensorLossTimeoutMs = 200
    };

    void queueForward(
        FakeRandom& random,
        uint32_t waitMs = 500,
        float distanceMeters = 1.0f)
    {
        random.addInt(waitMs);
        random.addInt(0);
        random.addFloat(distanceMeters);
    }

    void startForward(
        RandomDriveBehavior& behavior,
        FakeClock& clock)
    {
        behavior.begin();
        clock.advance(500);
        behavior.update();
    }
}

void test_no_obstacle_keeps_forward_motion_active()
{
    FakeClock clock;
    FakeDistanceSensor sensor;
    FakeDriveController drive;
    FakeRandom random;
    MotionController motion(drive, clock, MotionConfig);
    RandomDriveBehavior behavior(
        motion, sensor, clock, random, BehaviorConfig);

    sensor.setReading(1000);
    queueForward(random);
    startForward(behavior, clock);

    behavior.update();

    TEST_ASSERT_TRUE(motion.isBusy());
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::Forward),
        static_cast<int>(drive.getState()));
}

void test_obstacle_interrupts_forward_and_starts_backup()
{
    FakeClock clock;
    FakeDistanceSensor sensor;
    FakeDriveController drive;
    FakeRandom random;
    MotionController motion(drive, clock, MotionConfig);
    RandomDriveBehavior behavior(
        motion, sensor, clock, random, BehaviorConfig);

    sensor.setReading(1000);
    queueForward(random);
    startForward(behavior, clock);

    sensor.setReading(200);
    behavior.update();

    TEST_ASSERT_TRUE(motion.isBusy());
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::Backward),
        static_cast<int>(drive.getState()));
    TEST_ASSERT_EQUAL_UINT32(100, motion.getDurationMs());
}

void test_backup_completion_starts_avoidance_turn()
{
    FakeClock clock;
    FakeDistanceSensor sensor;
    FakeDriveController drive;
    FakeRandom random;
    MotionController motion(drive, clock, MotionConfig);
    RandomDriveBehavior behavior(
        motion, sensor, clock, random, BehaviorConfig);

    sensor.setReading(1000);
    queueForward(random);
    startForward(behavior, clock);
    sensor.setReading(200);
    behavior.update();

    random.addFloat(90.0f);
    random.addInt(1);
    clock.advance(100);
    motion.update();
    behavior.update();

    TEST_ASSERT_TRUE(motion.isBusy());
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::RotateRight),
        static_cast<int>(drive.getState()));
    TEST_ASSERT_EQUAL_UINT32(90, motion.getDurationMs());
}

void test_avoidance_turn_completion_resumes_normal_behavior()
{
    FakeClock clock;
    FakeDistanceSensor sensor;
    FakeDriveController drive;
    FakeRandom random;
    MotionController motion(drive, clock, MotionConfig);
    RandomDriveBehavior behavior(
        motion, sensor, clock, random, BehaviorConfig);

    sensor.setReading(1000);
    queueForward(random);
    startForward(behavior, clock);
    sensor.setReading(200);
    behavior.update();

    random.addFloat(90.0f);
    random.addInt(1);
    clock.advance(100);
    motion.update();
    behavior.update();

    random.addInt(500);
    clock.advance(90);
    motion.update();
    behavior.update();

    sensor.setReading(1000);
    random.addInt(0);
    random.addFloat(0.5f);
    clock.advance(500);
    behavior.update();

    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::Forward),
        static_cast<int>(drive.getState()));
}

void test_invalid_reading_prevents_forward_motion()
{
    FakeClock clock;
    FakeDistanceSensor sensor;
    FakeDriveController drive;
    FakeRandom random;
    MotionController motion(drive, clock, MotionConfig);
    RandomDriveBehavior behavior(
        motion, sensor, clock, random, BehaviorConfig);

    sensor.invalidate();
    random.addInt(500);
    random.addInt(0);
    behavior.begin();
    clock.advance(500);
    behavior.update();

    TEST_ASSERT_FALSE(motion.isBusy());
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::Stopped),
        static_cast<int>(drive.getState()));
}

void test_wait_timing_is_safe_across_millis_overflow()
{
    FakeClock clock;
    FakeDistanceSensor sensor;
    FakeDriveController drive;
    FakeRandom random;
    MotionController motion(drive, clock, MotionConfig);
    RandomDriveBehavior behavior(
        motion, sensor, clock, random, BehaviorConfig);

    sensor.setReading(1000);
    clock.advance(UINT32_MAX - 100u);
    random.addInt(200);
    random.addInt(0);
    random.addFloat(0.5f);
    behavior.begin();

    clock.advance(199);
    behavior.update();
    TEST_ASSERT_FALSE(motion.isBusy());

    clock.advance(1);
    behavior.update();
    TEST_ASSERT_TRUE(motion.isBusy());
}

void setup()
{
    delay(2000);
    UNITY_BEGIN();

    RUN_TEST(test_no_obstacle_keeps_forward_motion_active);
    RUN_TEST(test_obstacle_interrupts_forward_and_starts_backup);
    RUN_TEST(test_backup_completion_starts_avoidance_turn);
    RUN_TEST(test_avoidance_turn_completion_resumes_normal_behavior);
    RUN_TEST(test_invalid_reading_prevents_forward_motion);
    RUN_TEST(test_wait_timing_is_safe_across_millis_overflow);

    UNITY_END();
}

void loop()
{
}
