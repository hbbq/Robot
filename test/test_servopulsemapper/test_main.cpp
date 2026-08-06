#include <Arduino.h>
#include <unity.h>

#include <ServoPulseMapper.h>

namespace
{
    constexpr ServoControllerConfig ServoConfig
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
}

void test_measured_calibration_points_map_exactly()
{
    TEST_ASSERT_EQUAL_UINT32(
        530, servoPulseMicrosecondsForAngle(0.0f, ServoConfig));
    TEST_ASSERT_EQUAL_UINT32(
        1500, servoPulseMicrosecondsForAngle(90.0f, ServoConfig));
    TEST_ASSERT_EQUAL_UINT32(
        2610, servoPulseMicrosecondsForAngle(180.0f, ServoConfig));
}

void test_each_side_uses_its_own_interpolation_slope()
{
    TEST_ASSERT_EQUAL_UINT32(
        1015, servoPulseMicrosecondsForAngle(45.0f, ServoConfig));
    TEST_ASSERT_EQUAL_UINT32(
        2055, servoPulseMicrosecondsForAngle(135.0f, ServoConfig));
}

void test_angles_are_clamped_to_supported_range()
{
    TEST_ASSERT_EQUAL_UINT32(
        530, servoPulseMicrosecondsForAngle(-20.0f, ServoConfig));
    TEST_ASSERT_EQUAL_UINT32(
        2610, servoPulseMicrosecondsForAngle(220.0f, ServoConfig));
}

void setup()
{
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_measured_calibration_points_map_exactly);
    RUN_TEST(test_each_side_uses_its_own_interpolation_slope);
    RUN_TEST(test_angles_are_clamped_to_supported_range);
    UNITY_END();
}

void loop()
{
}
