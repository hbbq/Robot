#include <Arduino.h>
#include <unity.h>

#include <AutonomousBehaviorRequestStore.h>
#include <DeviceRegistry.h>
#include <FakeClock.h>
#include <MessageDispatcher.h>
#include <Messages/AnnouncementMessage.h>
#include <Messages/DriveCommandMessage.h>
#include <Messages/RobotStateMessage.h>
#include <Messages/SetAutonomousBehaviorMessage.h>
#include <Messages/SetRobotModeMessage.h>
#include <ReadinessController.h>
#include <RemoteDriveState.h>
#include <RobotModeRequestStore.h>
#include <RobotStateStore.h>

namespace
{
    constexpr RobotId LocalRobotId = 1;
    constexpr RobotId ForeignRobotId = 2;

    constexpr uint8_t RobotMac[6] = {1, 1, 1, 1, 1, 1};
    constexpr uint8_t DisplayMac[6] = {2, 2, 2, 2, 2, 2};
    constexpr uint8_t RemoteMac[6] = {3, 3, 3, 3, 3, 3};

    struct Fixture
    {
        FakeClock clock;
        DeviceRegistry registry{LocalRobotId};
        RobotStateStore robotState;
        RemoteDriveState remoteDrive;
        RobotModeRequestStore modeRequest;
        AutonomousBehaviorRequestStore behaviorRequest;
        MessageDispatcher dispatcher{
            registry,
            robotState,
            remoteDrive,
            modeRequest,
            behaviorRequest,
            clock};

        template<typename T>
        void receive(const uint8_t mac[6], const T& message)
        {
            dispatcher.onReceive(
                mac,
                reinterpret_cast<const uint8_t*>(&message),
                sizeof(message),
                -40);
        }

        void announce(
            const uint8_t mac[6],
            RobotId robotId,
            DeviceType type,
            Capability capabilities)
        {
            const auto message = makeAnnouncementMessage(
                1,
                robotId,
                type,
                capabilities);
            receive(mac, message);
        }
    };
}

void setUp() {}
void tearDown() {}

void matching_robot_id_message_is_accepted()
{
    Fixture fixture;
    fixture.announce(RobotMac, LocalRobotId, DeviceType::Robot, Capability::Motors);

    const auto message = makeRobotStateMessage(
        2,
        LocalRobotId,
        RobotMode::Autonomous,
        RobotMotion::Forward,
        AutonomousBehaviorType::Explore);
    fixture.receive(RobotMac, message);

    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RobotMode::Autonomous),
        static_cast<int>(fixture.robotState.mode()));
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(AutonomousBehaviorType::Explore),
        static_cast<int>(fixture.robotState.autonomousBehavior()));
}

void mismatched_robot_state_is_ignored()
{
    Fixture fixture;
    fixture.announce(RobotMac, LocalRobotId, DeviceType::Robot, Capability::Motors);

    const auto message = makeRobotStateMessage(
        2,
        ForeignRobotId,
        RobotMode::Autonomous,
        RobotMotion::Forward,
        AutonomousBehaviorType::Dance);
    fixture.receive(RobotMac, message);

    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RobotMode::Idle),
        static_cast<int>(fixture.robotState.mode()));
}

void mismatched_drive_command_is_ignored()
{
    Fixture fixture;
    fixture.announce(RemoteMac, LocalRobotId, DeviceType::Remote, Capability::RemoteControl);

    const auto message = makeDriveCommandMessage(
        2,
        ForeignRobotId,
        1.0f,
        0.0f);
    fixture.receive(RemoteMac, message);

    TEST_ASSERT_FALSE(fixture.remoteDrive.hasCommand());
}

void mismatched_requests_are_ignored()
{
    Fixture fixture;
    fixture.announce(RemoteMac, LocalRobotId, DeviceType::Remote, Capability::RemoteControl);

    const auto modeMessage = makeSetRobotModeMessage(
        2,
        ForeignRobotId,
        RobotMode::RemoteControl);
    fixture.receive(RemoteMac, modeMessage);

    const auto behaviorMessage = makeSetAutonomousBehaviorMessage(
        3,
        ForeignRobotId,
        AutonomousBehaviorType::Dance);
    fixture.receive(RemoteMac, behaviorMessage);

    TEST_ASSERT_FALSE(fixture.modeRequest.hasPendingRequest());
    TEST_ASSERT_FALSE(fixture.behaviorRequest.hasPendingRequest());
}

void foreign_display_cannot_satisfy_readiness()
{
    Fixture fixture;
    fixture.announce(DisplayMac, ForeignRobotId, DeviceType::Display, Capability::Display);

    const bool insertedDirectly = fixture.registry.updateDevice(
        DisplayMac,
        ForeignRobotId,
        DeviceType::Display,
        Capability::Display,
        -40,
        fixture.clock.millis());

    constexpr ReadinessConfig config{
        .requiredDeviceType = DeviceType::Display,
        .requiredCapabilities = Capability::Display};
    ReadinessController readiness(fixture.registry, config);
    readiness.begin();
    readiness.update();

    TEST_ASSERT_FALSE(readiness.isReady());
    TEST_ASSERT_FALSE(insertedDirectly);
    TEST_ASSERT_EQUAL_UINT32(0, fixture.registry.count());
}

void matching_robot_id_does_not_bypass_sender_role_validation()
{
    Fixture fixture;
    fixture.announce(DisplayMac, LocalRobotId, DeviceType::Display, Capability::Display);

    const auto driveMessage = makeDriveCommandMessage(
        2,
        LocalRobotId,
        1.0f,
        0.0f);
    fixture.receive(DisplayMac, driveMessage);

    const auto modeMessage = makeSetRobotModeMessage(
        3,
        LocalRobotId,
        RobotMode::RemoteControl);
    fixture.receive(DisplayMac, modeMessage);

    TEST_ASSERT_FALSE(fixture.remoteDrive.hasCommand());
    TEST_ASSERT_FALSE(fixture.modeRequest.hasPendingRequest());
}

void setup()
{
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(matching_robot_id_message_is_accepted);
    RUN_TEST(mismatched_robot_state_is_ignored);
    RUN_TEST(mismatched_drive_command_is_ignored);
    RUN_TEST(mismatched_requests_are_ignored);
    RUN_TEST(foreign_display_cannot_satisfy_readiness);
    RUN_TEST(matching_robot_id_does_not_bypass_sender_role_validation);
    UNITY_END();
}

void loop() {}
