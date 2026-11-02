/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** EventTests
*/

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "MessageTestHelpers.hpp"
#include "rtype/protocol/Event.hpp"

namespace proto = rtype::protocol;
using protocol_test::encode;
using protocol_test::expectEveryPrefixTruncated;
using protocol_test::expectOutOfRange;
using protocol_test::expectRoundTrip;

namespace {

/*
** One event of each type, with a and b as RFC section 6.6 fills them.
*/
const std::vector<proto::Event> EVERY_TYPE{
    proto::Event{.eventId = 1, .type = proto::EventType::PLAYER_FIRED, .a = 101,
        .b = 500, .x = 100.0F, .y = 200.0F},
    proto::Event{.eventId = 2, .type = proto::EventType::ENEMY_FIRED, .a = 300,
        .b = 501, .x = 900.0F, .y = 50.0F},
    proto::Event{.eventId = 3, .type = proto::EventType::ENEMY_KILLED, .a = 101,
        .b = 300, .x = 640.0F, .y = 360.0F},
    proto::Event{.eventId = 4, .type = proto::EventType::PLAYER_KILLED,
        .a = 301, .b = 102, .x = 1.5F, .y = -2.5F},
    proto::Event{.eventId = 5, .type = proto::EventType::PLAYER_JOINED, .a = 3},
    proto::Event{.eventId = 6, .type = proto::EventType::PLAYER_LEFT, .a = 2,
        .b = static_cast<std::uint32_t>(proto::LeaveReason::KICKED)},
    proto::Event{.eventId = 7, .type = proto::EventType::GAME_OVER,
        .a = 0xFFFFFFFF},
    proto::Event{.eventId = 0xFFFFFFFF,
        .type = proto::EventType::GAME_RESTARTED},
};

}

TEST(EventTests, EveryTypeRoundTripsInTwentyOneBytes)
{
    for (const proto::Event &event: EVERY_TYPE) {
        SCOPED_TRACE("type " + std::to_string(static_cast<int>(event.type)));
        expectRoundTrip(event);
        EXPECT_EQ(encode(event).size(), 21U);
    }
}

TEST(EventTests, EveryLeaveReasonIsAccepted)
{
    for (const proto::LeaveReason reason: {proto::LeaveReason::QUIT,
             proto::LeaveReason::TIMEOUT, proto::LeaveReason::KICKED})
        expectRoundTrip(proto::Event{.type = proto::EventType::PLAYER_LEFT,
            .b = static_cast<std::uint32_t>(reason)});
}

TEST(EventTests, EveryPrefixOfAPayloadIsTruncated)
{
    expectEveryPrefixTruncated(EVERY_TYPE[0]);
}

TEST(EventTests, AnUnknownTypeIsRejected)
{
    expectOutOfRange(proto::Event{.type = static_cast<proto::EventType>(0)});
    expectOutOfRange(proto::Event{.type = static_cast<proto::EventType>(9)});
}

TEST(EventTests, APlayerJoinedWithAPlayerIdOfFourIsRejected)
{
    expectOutOfRange(proto::Event{.type = proto::EventType::PLAYER_JOINED,
        .a = 4});
}

TEST(EventTests, APlayerLeftWithAPlayerIdOfFourIsRejected)
{
    expectOutOfRange(proto::Event{.type = proto::EventType::PLAYER_LEFT,
        .a = 4});
}

TEST(EventTests, APlayerLeftWithAnUnknownReasonIsRejected)
{
    expectOutOfRange(proto::Event{.type = proto::EventType::PLAYER_LEFT,
        .b = 3});
}

TEST(EventTests, APlayerLeftReasonDoesNotWrapPastAByte)
{
    expectOutOfRange(proto::Event{.type = proto::EventType::PLAYER_LEFT,
        .b = 256});
}
