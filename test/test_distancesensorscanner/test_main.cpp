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
        .readingTimeoutMs = 150,
        .sampleFreshnessMs = 1000
    };

    void completeSweepSample(
        DistanceSensorScanner& scanner,
        FakeDistanceSensor& sensor,
        FakeClock& clock,
        uint16_t distanceMillimeters)
    {
        clock.advance(PanConfig.settleTimeMs);
        scanner.update();
        sensor.setReading(distanceMillimeters);
        scanner.update();
    }
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

void test_continuous_sweep_uses_center_left_center_right_sequence()
{
    FakeClock clock;
    FakeDistanceSensor sensor;
    FakeServoController servo;
    DistanceSensorScanner scanner(servo, sensor, clock, PanConfig);

    scanner.startContinuousSweep();
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 90.0f, servo.getAngle());
    completeSweepSample(scanner, sensor, clock, 900);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 120.0f, servo.getAngle());
    completeSweepSample(scanner, sensor, clock, 800);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 90.0f, servo.getAngle());
    completeSweepSample(scanner, sensor, clock, 700);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 60.0f, servo.getAngle());
    completeSweepSample(scanner, sensor, clock, 600);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 90.0f, servo.getAngle());
}

void test_fresh_readings_are_attributed_to_the_target_sector()
{
    FakeClock clock;
    FakeDistanceSensor sensor;
    FakeServoController servo;
    DistanceSensorScanner scanner(servo, sensor, clock, PanConfig);

    scanner.startContinuousSweep();
    completeSweepSample(scanner, sensor, clock, 900);
    completeSweepSample(scanner, sensor, clock, 400);
    completeSweepSample(scanner, sensor, clock, 800);
    completeSweepSample(scanner, sensor, clock, 600);

    const FrontScanState& state = scanner.frontScanState();
    TEST_ASSERT_EQUAL_UINT16(400, state.left.distanceMillimeters);
    TEST_ASSERT_EQUAL_UINT16(800, state.center.distanceMillimeters);
    TEST_ASSERT_EQUAL_UINT16(600, state.right.distanceMillimeters);
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(FrontScanAssessment::Clear),
        static_cast<int>(scanner.assessFront(250)));
}

void test_stale_or_invalid_sector_is_not_clear()
{
    FakeClock clock;
    FakeDistanceSensor sensor;
    FakeServoController servo;
    DistanceSensorScanner scanner(servo, sensor, clock, PanConfig);

    scanner.startContinuousSweep();
    completeSweepSample(scanner, sensor, clock, 900);
    completeSweepSample(scanner, sensor, clock, 800);
    completeSweepSample(scanner, sensor, clock, 900);
    completeSweepSample(scanner, sensor, clock, 700);
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(FrontScanAssessment::Clear),
        static_cast<int>(scanner.assessFront(250)));

    clock.advance(PanConfig.sampleFreshnessMs + 1);
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(FrontScanAssessment::Incomplete),
        static_cast<int>(scanner.assessFront(250)));
}

void setup()
{
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_semantic_directions_use_configured_angles);
    RUN_TEST(test_reading_from_settle_period_is_not_accepted);
    RUN_TEST(test_missing_new_reading_completes_as_invalid);
    RUN_TEST(test_continuous_sweep_uses_center_left_center_right_sequence);
    RUN_TEST(test_fresh_readings_are_attributed_to_the_target_sector);
    RUN_TEST(test_stale_or_invalid_sector_is_not_clear);
    UNITY_END();
}

void loop()
{
}
