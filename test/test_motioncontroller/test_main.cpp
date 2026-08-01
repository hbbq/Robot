#include <Arduino.h>
#include <unity.h>

#include <TestUtil.h>

#include <MotionController.h>
#include <FakeClock.h>
#include <FakeDriveController.h>

constexpr MotionControllerConfig testConfig
{
    .moveSpeed = 0.5f,
    .turnSpeed = 0.4f,
    .millisecondsPerMeter = 2000.0f,
    .millisecondsPerDegree = 8.0f
};

void setUp()
{
}

void tearDown()
{
}

void test_go_forward_starts_forward_motion()
{
    FakeClock clock;
    FakeDriveController drive;
    MotionController motion(
        drive,
        clock,
        testConfig
    );

    motion.goForward(1.0f);

    TEST_ASSERT_TRUE(motion.isBusy());
    TEST_ASSERT_EQUAL(
        static_cast<int>(MotionState::MovingForward),
        static_cast<int>(motion.getState())
    );

    TEST_ASSERT_EQUAL(
        static_cast<int>(DriveState::Forward),
        static_cast<int>(drive.getState())
    );

    TEST_ASSERT_FLOAT_WITHIN(
        0.001f,
        0.5f,
        drive.getLinearSpeed()
    );

    TEST_ASSERT_EQUAL_UINT32(
        2000,
        motion.getDurationMs()
    );
}

void test_motion_continues_before_duration_has_passed()
{
    FakeClock clock;
    FakeDriveController drive;
    MotionController motion(
        drive,
        clock,
        testConfig
    );

    motion.goForward(1.0f);

    clock.advance(1999);
    motion.update();

    TEST_ASSERT_TRUE(motion.isBusy());

    TEST_ASSERT_EQUAL(
        static_cast<int>(DriveState::Forward),
        static_cast<int>(drive.getState())
    );
}

void test_motion_stops_when_duration_has_passed()
{
    FakeClock clock;
    FakeDriveController drive;
    MotionController motion(
        drive,
        clock,
        testConfig
    );

    motion.goForward(1.0f);

    clock.advance(2000);
    motion.update();

    TEST_ASSERT_FALSE(motion.isBusy());

    TEST_ASSERT_EQUAL(
        static_cast<int>(MotionState::Idle),
        static_cast<int>(motion.getState())
    );

    TEST_ASSERT_EQUAL(
        static_cast<int>(DriveState::Stopped),
        static_cast<int>(drive.getState())
    );
}

void test_turn_left_calculates_duration()
{
    FakeClock clock;
    FakeDriveController drive;
    MotionController motion(
        drive,
        clock,
        testConfig
    );

    motion.turnLeft(90.0f);

    TEST_ASSERT_TRUE(motion.isBusy());

    TEST_ASSERT_EQUAL(
        static_cast<int>(MotionState::TurningLeft),
        static_cast<int>(motion.getState())
    );

    TEST_ASSERT_EQUAL(
        static_cast<int>(DriveState::RotateLeft),
        static_cast<int>(drive.getState())
    );

    TEST_ASSERT_EQUAL_UINT32(
        720,
        motion.getDurationMs()
    );
}

void test_zero_distance_does_not_start_motion()
{
    FakeClock clock;
    FakeDriveController drive;
    MotionController motion(
        drive,
        clock,
        testConfig
    );

    motion.goForward(0.0f);

    TEST_ASSERT_FALSE(motion.isBusy());

    TEST_ASSERT_EQUAL(
        static_cast<int>(DriveState::Stopped),
        static_cast<int>(drive.getState())
    );
}

void test_new_motion_replaces_current_motion()
{
    FakeClock clock;
    FakeDriveController drive;
    MotionController motion(
        drive,
        clock,
        testConfig
    );

    motion.goForward(1.0f);
    motion.turnRight(90.0f);

    TEST_ASSERT_TRUE(motion.isBusy());

    TEST_ASSERT_EQUAL(
        static_cast<int>(MotionState::TurningRight),
        static_cast<int>(motion.getState())
    );

    TEST_ASSERT_EQUAL(
        static_cast<int>(DriveState::RotateRight),
        static_cast<int>(drive.getState())
    );
}

void setup()
{
    TestUtil::waitForTestSerial();

    UNITY_BEGIN();

    RUN_TEST(test_go_forward_starts_forward_motion);
    RUN_TEST(test_motion_continues_before_duration_has_passed);
    RUN_TEST(test_motion_stops_when_duration_has_passed);
    RUN_TEST(test_turn_left_calculates_duration);
    RUN_TEST(test_zero_distance_does_not_start_motion);
    RUN_TEST(test_new_motion_replaces_current_motion);

    UNITY_END();
}

void loop()
{
}