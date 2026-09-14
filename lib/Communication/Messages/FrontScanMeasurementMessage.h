#pragma once

#include <cstdint>
#include <type_traits>

#include <FrontScanMeasurement.h>
#include <Messages/MessageHeader.h>

enum class FrontScanSectorFlags : uint8_t
{
    None = 0,
    HasSample = 1 << 0,
    Valid = 1 << 1
};

struct FrontScanSectorMessage
{
    uint16_t distanceMillimeters;
    uint16_t ageMs;
    FrontScanSectorFlags flags;
    uint8_t reserved;
};

struct FrontScanMeasurementMessage
{
    MessageHeader header;
    FrontScanSectorMessage left;
    FrontScanSectorMessage center;
    FrontScanSectorMessage right;
    uint16_t sampleFreshnessMs;
};

static_assert(sizeof(FrontScanSectorMessage) == 6);
static_assert(sizeof(FrontScanMeasurementMessage) == 32);
static_assert(std::is_trivially_copyable_v<FrontScanMeasurementMessage>);

inline FrontScanSectorMessage makeFrontScanSectorMessage(
    const FrontScanSectorMeasurement& sector)
{
    uint8_t flags = 0;
    if (sector.hasSample)
    {
        flags |= static_cast<uint8_t>(FrontScanSectorFlags::HasSample);
    }
    if (sector.valid)
    {
        flags |= static_cast<uint8_t>(FrontScanSectorFlags::Valid);
    }

    return FrontScanSectorMessage
    {
        .distanceMillimeters = sector.distanceMillimeters,
        .ageMs = sector.ageMs,
        .flags = static_cast<FrontScanSectorFlags>(flags),
        .reserved = 0
    };
}

inline FrontScanMeasurementMessage makeFrontScanMeasurementMessage(
    uint32_t sequenceNumber,
    RobotId robotId,
    const FrontScanMeasurement& measurement)
{
    return FrontScanMeasurementMessage
    {
        .header =
        {
            .protocolVersion = CommunicationProtocolVersion,
            .type = MessageType::FrontScanMeasurement,
            .messageLength = sizeof(FrontScanMeasurementMessage),
            .sequenceNumber = sequenceNumber,
            .robotId = robotId
        },
        .left = makeFrontScanSectorMessage(measurement.left),
        .center = makeFrontScanSectorMessage(measurement.center),
        .right = makeFrontScanSectorMessage(measurement.right),
        .sampleFreshnessMs = measurement.sampleFreshnessMs
    };
}

inline bool isValidFrontScanSectorMessage(
    const FrontScanSectorMessage& sector)
{
    constexpr uint8_t KnownFlags =
        static_cast<uint8_t>(FrontScanSectorFlags::HasSample) |
        static_cast<uint8_t>(FrontScanSectorFlags::Valid);
    const uint8_t flags = static_cast<uint8_t>(sector.flags);
    const bool hasSample =
        (flags & static_cast<uint8_t>(FrontScanSectorFlags::HasSample)) != 0;
    const bool valid =
        (flags & static_cast<uint8_t>(FrontScanSectorFlags::Valid)) != 0;

    return (flags & ~KnownFlags) == 0 &&
        (!valid || hasSample) &&
        sector.reserved == 0;
}

inline FrontScanSectorMeasurement toFrontScanSectorMeasurement(
    const FrontScanSectorMessage& sector)
{
    const uint8_t flags = static_cast<uint8_t>(sector.flags);
    return FrontScanSectorMeasurement
    {
        .hasSample =
            (flags & static_cast<uint8_t>(FrontScanSectorFlags::HasSample)) != 0,
        .valid =
            (flags & static_cast<uint8_t>(FrontScanSectorFlags::Valid)) != 0,
        .distanceMillimeters = sector.distanceMillimeters,
        .ageMs = sector.ageMs
    };
}
