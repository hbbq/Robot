#include <Arduino.h>
#include <unity.h>

#include <FakeClock.h>
#include <FakeDriveController.h>
#include <RemoteControlBehavior.h>
#include <RemoteDriveState.h>

namespace
{
    constexpr RemoteControlBehaviorConfig BehaviorConfig
    {
        .commandTimeoutMs = 500,
        .maxLinearSpeed = 0.30f,
        .maxAngularSpeed = 0.12f
    };

    void applyCommand(
        float linear,
        float angular,
        FakeClock& clock,
        RemoteDriveState& state,
        RemoteControlBehavior& behavior)
    {
        state.setCommand(linear, angular, clock.millis());
        behavior.update();
    }
}

void test_full_forward_is_limited()
{
    FakeClock clock;
    FakeDriveController drive;
    RemoteDriveState state;
    RemoteControlBehavior behavior(state, drive, clock, BehaviorConfig);

    applyCommand(1.0f, 0.0f, clock, state, behavior);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.30f, drive.getLinearSpeed());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, drive.getAngularSpeed());
}

void test_full_reverse_is_limited_symmetrically()
{
    FakeClock clock;
    FakeDriveController drive;
    RemoteDriveState state;
    RemoteControlBehavior behavior(state, drive, clock, BehaviorConfig);

    applyCommand(-1.0f, 0.0f, clock, state, behavior);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, -0.30f, drive.getLinearSpeed());
}

void test_full_turns_are_limited_symmetrically()
{
    FakeClock clock;
    FakeDriveController drive;
    RemoteDriveState state;
    RemoteControlBehavior behavior(state, drive, clock, BehaviorConfig);

    applyCommand(0.0f, -1.0f, clock, state, behavior);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -0.12f, drive.getAngularSpeed());

    applyCommand(0.0f, 1.0f, clock, state, behavior);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.12f, drive.getAngularSpeed());
}

void test_combined_input_is_scaled_independently()
{
    FakeClock clock;
    FakeDriveController drive;
    RemoteDriveState state;
    RemoteControlBehavior behavior(state, drive, clock, BehaviorConfig);

    applyCommand(0.5f, -0.5f, clock, state, behavior);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.15f, drive.getLinearSpeed());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -0.06f, drive.getAngularSpeed());
}

void test_timeout_stops_the_robot()
{
    FakeClock clock;
    FakeDriveController drive;
    RemoteDriveState state;
    RemoteControlBehavior behavior(state, drive, clock, BehaviorConfig);

    applyCommand(1.0f, 0.0f, clock, state, behavior);
    clock.advance(501);
    behavior.update();

    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(DriveState::Stopped),
        static_cast<int>(drive.getState()));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, drive.getLinearSpeed());
    TEST_ASSERT_TRUE(drive.getStopCallCount() > 0);
}

void setup()
{
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_full_forward_is_limited);
    RUN_TEST(test_full_reverse_is_limited_symmetrically);
    RUN_TEST(test_full_turns_are_limited_symmetrically);
    RUN_TEST(test_combined_input_is_scaled_independently);
    RUN_TEST(test_timeout_stops_the_robot);
    UNITY_END();
}

void loop()
{
}
