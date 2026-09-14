#include <Arduino.h>
#include <unity.h>

#include <AutonomousBehaviorRequestStore.h>
#include <Capability.h>
#include <DeviceRegistry.h>
#include <DistanceSensorScanner.h>
#include <FakeClock.h>
#include <FakeDistanceSensor.h>
#include <FakeServoController.h>
#include <FrontScanMeasurement.h>
#include <Messages/FrontScanMeasurementMessage.h>
#include <FrontScanMeasurementStore.h>
#include <FrontScanTelemetryConfig.h>
#include <FrontScanTelemetryPublisher.h>
#include <IFrontScanMeasurementSender.h>
#include <MessageDispatcher.h>
#include <MessageSerializer.h>
#include <RemoteDriveState.h>
#include <RobotModeRequestStore.h>
#include <RobotStateStore.h>

#include <cstring>

namespace
{
    constexpr RobotId TestRobotId = 7;
    constexpr uint8_t RobotMac[6] = {1, 2, 3, 4, 5, 6};
    constexpr uint8_t RemoteMac[6] = {6, 5, 4, 3, 2, 1};

    constexpr DistanceSensorPanConfig PanConfig
    {
        .centerAngle = 90.0f,
        .leftAngle = 120.0f,
        .rightAngle = 60.0f,
        .settleTimeMs = 100,
        .readingTimeoutMs = 150,
        .sampleFreshnessMs = 1000
    };

    constexpr FrontScanTelemetryConfig TelemetryConfig
    {
        .minimumSendIntervalMs = 400,
        .snapshotIntervalMs = 2000
    };

    class FakeMeasurementSender : public IFrontScanMeasurementSender
    {
    public:
        bool sendFrontScanMeasurement(
            const FrontScanMeasurement& value) override
        {
            if (!sendResult)
            {
                return false;
            }

            measurement = value;
            ++sendCount;
            return true;
        }

        FrontScanMeasurement measurement;
        uint32_t sendCount = 0;
        bool sendResult = true;
    };

    void completeSample(
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

    FrontScanMeasurement validMeasurement()
    {
        return FrontScanMeasurement
        {
            .left = {true, true, 410, 30},
            .center = {true, false, 220, 20},
            .right = {false, false, 0, 0},
            .sampleFreshnessMs = 1000
        };
    }

    struct DispatcherFixture
    {
        FakeClock clock;
        DeviceRegistry registry{TestRobotId};
        RobotStateStore robotState;
        RemoteDriveState remoteDrive;
        RobotModeRequestStore modeRequests;
        AutonomousBehaviorRequestStore behaviorRequests;
        FrontScanMeasurementStore measurements;
        MessageDispatcher dispatcher
        {
            registry,
            robotState,
            remoteDrive,
            modeRequests,
            behaviorRequests,
            measurements,
            clock
        };
    };
}

void test_message_serializes_sector_validity_distance_and_age()
{
    const FrontScanMeasurement expected = validMeasurement();
    const auto message = makeFrontScanMeasurementMessage(
        42,
        TestRobotId,
        expected);
    uint8_t bytes[sizeof(message)]{};

    TEST_ASSERT_TRUE(MessageSerializer::serialize(message, bytes, sizeof(bytes)));
    const auto decoded =
        MessageSerializer::deserialize<FrontScanMeasurementMessage>(
            bytes,
            sizeof(bytes));

    TEST_ASSERT_TRUE(decoded.has_value());
    TEST_ASSERT_EQUAL_UINT8(
        static_cast<uint8_t>(MessageType::FrontScanMeasurement),
        static_cast<uint8_t>(decoded->header.type));
    TEST_ASSERT_EQUAL_UINT16(410, decoded->left.distanceMillimeters);
    TEST_ASSERT_EQUAL_UINT16(30, decoded->left.ageMs);
    TEST_ASSERT_TRUE(isValidFrontScanSectorMessage(decoded->left));
    TEST_ASSERT_TRUE(isValidFrontScanSectorMessage(decoded->center));
    TEST_ASSERT_TRUE(isValidFrontScanSectorMessage(decoded->right));
}

void test_store_uses_sender_age_and_local_elapsed_for_freshness()
{
    FrontScanMeasurementStore store;
    const FrontScanMeasurement measurement = validMeasurement();
    store.setMeasurement(measurement, 5000);

    TEST_ASSERT_TRUE(store.isFresh(store.measurement().left, 5900));
    TEST_ASSERT_FALSE(store.isFresh(store.measurement().left, 6001));
    TEST_ASSERT_FALSE(store.isFresh(store.measurement().center, 5000));
    TEST_ASSERT_FALSE(store.isFresh(store.measurement().right, 5000));
}

void test_publisher_rate_limits_samples_and_resends_periodically()
{
    FakeClock clock;
    FakeDistanceSensor sensor;
    FakeServoController servo;
    DistanceSensorScanner scanner(servo, sensor, clock, PanConfig);
    FakeMeasurementSender sender;
    FrontScanTelemetryPublisher publisher(
        scanner,
        sender,
        clock,
        TelemetryConfig);

    scanner.startContinuousSweep();
    completeSample(scanner, sensor, clock, 800);
    publisher.update();
    TEST_ASSERT_EQUAL_UINT32(1, sender.sendCount);

    completeSample(scanner, sensor, clock, 700);
    publisher.update();
    TEST_ASSERT_EQUAL_UINT32(1, sender.sendCount);

    clock.advance(200);
    publisher.update();
    TEST_ASSERT_EQUAL_UINT32(2, sender.sendCount);
    TEST_ASSERT_TRUE(sender.measurement.left.hasSample);
    TEST_ASSERT_EQUAL_UINT16(700, sender.measurement.left.distanceMillimeters);

    clock.advance(TelemetryConfig.snapshotIntervalMs);
    publisher.update();
    TEST_ASSERT_EQUAL_UINT32(3, sender.sendCount);
}

void test_publisher_reports_timeout_invalid_sample()
{
    FakeClock clock;
    FakeDistanceSensor sensor;
    FakeServoController servo;
    DistanceSensorScanner scanner(servo, sensor, clock, PanConfig);
    FakeMeasurementSender sender;
    FrontScanTelemetryPublisher publisher(
        scanner,
        sender,
        clock,
        TelemetryConfig);

    scanner.lookCenter();
    clock.advance(PanConfig.settleTimeMs);
    scanner.update();
    clock.advance(PanConfig.readingTimeoutMs);
    scanner.update();
    publisher.update();

    TEST_ASSERT_EQUAL_UINT32(1, sender.sendCount);
    TEST_ASSERT_TRUE(sender.measurement.center.hasSample);
    TEST_ASSERT_FALSE(sender.measurement.center.valid);
}

void test_publisher_rate_limits_failed_send_attempts()
{
    FakeClock clock;
    FakeDistanceSensor sensor;
    FakeServoController servo;
    DistanceSensorScanner scanner(servo, sensor, clock, PanConfig);
    FakeMeasurementSender sender;
    sender.sendResult = false;
    FrontScanTelemetryPublisher publisher(
        scanner,
        sender,
        clock,
        TelemetryConfig);

    scanner.startContinuousSweep();
    completeSample(scanner, sensor, clock, 800);
    publisher.update();
    publisher.update();
    TEST_ASSERT_EQUAL_UINT32(0, sender.sendCount);

    sender.sendResult = true;
    clock.advance(TelemetryConfig.minimumSendIntervalMs - 1);
    publisher.update();
    TEST_ASSERT_EQUAL_UINT32(0, sender.sendCount);

    clock.advance(1);
    publisher.update();
    TEST_ASSERT_EQUAL_UINT32(1, sender.sendCount);
}

void test_dispatcher_accepts_only_matching_robot_sender()
{
    DispatcherFixture fixture;
    fixture.registry.updateDevice(
        RobotMac,
        TestRobotId,
        DeviceType::Robot,
        Capability::Distance,
        -40,
        0);
    fixture.registry.updateDevice(
        RemoteMac,
        TestRobotId,
        DeviceType::Remote,
        Capability::RemoteControl,
        -40,
        0);

    auto message = makeFrontScanMeasurementMessage(
        1,
        TestRobotId,
        validMeasurement());
    fixture.dispatcher.onReceive(
        RemoteMac,
        reinterpret_cast<const uint8_t*>(&message),
        sizeof(message),
        -40);
    TEST_ASSERT_FALSE(fixture.measurements.hasMeasurement());

    message.header.robotId = 99;
    fixture.dispatcher.onReceive(
        RobotMac,
        reinterpret_cast<const uint8_t*>(&message),
        sizeof(message),
        -40);
    TEST_ASSERT_FALSE(fixture.measurements.hasMeasurement());

    message.header.robotId = TestRobotId;
    fixture.clock.advance(25);
    fixture.dispatcher.onReceive(
        RobotMac,
        reinterpret_cast<const uint8_t*>(&message),
        sizeof(message),
        -40);
    TEST_ASSERT_TRUE(fixture.measurements.hasMeasurement());
    TEST_ASSERT_EQUAL_UINT32(25, fixture.measurements.receivedAtMs());
}

void setup()
{
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_message_serializes_sector_validity_distance_and_age);
    RUN_TEST(test_store_uses_sender_age_and_local_elapsed_for_freshness);
    RUN_TEST(test_publisher_rate_limits_samples_and_resends_periodically);
    RUN_TEST(test_publisher_reports_timeout_invalid_sample);
    RUN_TEST(test_publisher_rate_limits_failed_send_attempts);
    RUN_TEST(test_dispatcher_accepts_only_matching_robot_sender);
    UNITY_END();
}

void loop()
{
}
