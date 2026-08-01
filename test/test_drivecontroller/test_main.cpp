#include <Arduino.h>
#include <unity.h>
#include <TestUtil.h>

#include <FakeMotorController.h>
#include <DriveController.h>

void test_forward_sets_both_motors()
{
    FakeMotorController left;
    FakeMotorController right;
    DriveController drive(left, right);

    drive.forward(0.5f);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, left.getSpeed());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, right.getSpeed());
}

void test_backward_sets_both_motors_negative()
{
    FakeMotorController left;
    FakeMotorController right;
    DriveController drive(left, right);

    drive.backward(0.4f);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, -0.4f, left.getSpeed());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -0.4f, right.getSpeed());
}

void test_rotate_left_drives_motors_in_opposite_directions()
{
    FakeMotorController left;
    FakeMotorController right;
    DriveController drive(left, right);

    drive.rotateLeft(0.25f);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, -0.25f, left.getSpeed());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.25f, right.getSpeed());
}

void test_rotate_right_drives_motors_in_opposite_directions()
{
    FakeMotorController left;
    FakeMotorController right;
    DriveController drive(left, right);

    drive.rotateRight(0.25f);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.25f, left.getSpeed());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -0.25f, right.getSpeed());
}

void test_curve_right_makes_left_motor_faster()
{
    FakeMotorController left;
    FakeMotorController right;
    DriveController drive(left, right);

    drive.setDrive(0.5f, 0.2f);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.7f, left.getSpeed());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.3f, right.getSpeed());
}

void test_stop_sets_both_motors_to_zero()
{
    FakeMotorController left;
    FakeMotorController right;
    DriveController drive(left, right);

    drive.forward(0.5f);
    drive.stop();

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, left.getSpeed());
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, right.getSpeed());
}

void setup()
{
    TestUtil::waitForTestSerial();

    UNITY_BEGIN();
    RUN_TEST(test_forward_sets_both_motors);
    RUN_TEST(test_backward_sets_both_motors_negative);
    RUN_TEST(test_rotate_left_drives_motors_in_opposite_directions);
    RUN_TEST(test_rotate_right_drives_motors_in_opposite_directions);
    RUN_TEST(test_curve_right_makes_left_motor_faster);
    RUN_TEST(test_stop_sets_both_motors_to_zero);
    UNITY_END();
}

void loop()
{
}