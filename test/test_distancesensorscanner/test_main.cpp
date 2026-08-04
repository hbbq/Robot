#include <Arduino.h>
#include <unity.h>

#include <DistanceSensorScanner.h>
#include <FakeClock.h>
#include <FakeDistanceSensor.h>
#include <FakeServoController.h>

namespace
{
    constexpr DistanceSensorPanConfig PanConfig
    {
        .centerAngle = 90.0f,
        .leftAngle = 120.0f,
        .rightAngle = 60.0f,
        .settleTimeMs = 100,
        .readingTimeoutMs = 150
    };
}

void test_semantic_directions_use_configured_angles()
{
    FakeClock clock;
    FakeDistanceSensor sensor;
    FakeServoController servo;
    DistanceSensorScanner scanner(servo, sensor, clock, PanConfig);

    scanner.lookCenter();
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 90.0f, servo.getAngle());
    scanner.lookLeft();
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 120.0f, servo.getAngle());
    scanner.lookRight();
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 60.0f, servo.getAngle());
}

void test_reading_from_settle_period_is_not_accepted()
{
    FakeClock clock;
    FakeDistanceSensor sensor;
    FakeServoController servo;
    DistanceSensorScanner scanner(servo, sensor, clock, PanConfig);

    scanner.lookLeft();
    sensor.setReading(200);
    clock.advance(PanConfig.settleTimeMs);
    scanner.update();

    TEST_ASSERT_TRUE(scanner.isBusy());
    TEST_ASSERT_FALSE(scanner.hasValidReading());

    sensor.setReading(800);
    scanner.update();

    TEST_ASSERT_TRUE(scanner.isComplete());
    TEST_ASSERT_TRUE(scanner.hasValidReading());
    TEST_ASSERT_EQUAL_UINT16(800, scanner.distanceMillimeters());
}

void test_missing_new_reading_completes_as_invalid()
{
    FakeClock clock;
    FakeDistanceSensor sensor;
    FakeServoController servo;
    DistanceSensorScanner scanner(servo, sensor, clock, PanConfig);

    scanner.lookRight();
    clock.advance(PanConfig.settleTimeMs);
    scanner.update();
    clock.advance(PanConfig.readingTimeoutMs);
    scanner.update();

    TEST_ASSERT_TRUE(scanner.isComplete());
    TEST_ASSERT_FALSE(scanner.hasValidReading());
}

void setup()
{
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_semantic_directions_use_configured_angles);
    RUN_TEST(test_reading_from_settle_period_is_not_accepted);
    RUN_TEST(test_missing_new_reading_completes_as_invalid);
    UNITY_END();
}

void loop()
{
}
