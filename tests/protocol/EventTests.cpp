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

using namespace rtype::protocol;
using namespace protocol_test;

namespace {

/*
** One event of each type, with a and b as RFC section 6.6 fills them.
*/
const std::vector<Event> EVERY_TYPE{
    Event{.eventId = 1, .type = EventType::PLAYER_FIRED, .a = 101, .b = 500,
        .x = 100.0F, .y = 200.0F},
    Event{.eventId = 2, .type = EventType::ENEMY_FIRED, .a = 300, .b = 501,
        .x = 900.0F, .y = 50.0F},
    Event{.eventId = 3, .type = EventType::ENEMY_KILLED, .a = 101, .b = 300,
        .x = 640.0F, .y = 360.0F},
    Event{.eventId = 4, .type = EventType::PLAYER_KILLED, .a = 301, .b = 102,
        .x = 1.5F, .y = -2.5F},
    Event{.eventId = 5, .type = EventType::PLAYER_JOINED, .a = 3},
    Event{.eventId = 6, .type = EventType::PLAYER_LEFT, .a = 2,
        .b = static_cast<std::uint32_t>(LeaveReason::KICKED)},
    Event{.eventId = 7, .type = EventType::GAME_OVER, .a = 0xFFFFFFFF},
    Event{.eventId = 0xFFFFFFFF, .type = EventType::GAME_RESTARTED},
};

}

TEST(EventTests, EveryTypeRoundTripsInTwentyOneBytes)
{
    for (const Event &event: EVERY_TYPE) {
        SCOPED_TRACE("type " + std::to_string(static_cast<int>(event.type)));
        expectRoundTrip(event);
        EXPECT_EQ(encode(event).size(), 21U);
    }
}

TEST(EventTests, EveryLeaveReasonIsAccepted)
{
    for (const LeaveReason reason: {LeaveReason::QUIT, LeaveReason::TIMEOUT,
             LeaveReason::KICKED})
        expectRoundTrip(Event{.type = EventType::PLAYER_LEFT,
            .b = static_cast<std::uint32_t>(reason)});
}

TEST(EventTests, EveryPrefixOfAPayloadIsTruncated)
{
    expectEveryPrefixTruncated(EVERY_TYPE[0]);
}

TEST(EventTests, AnUnknownTypeIsRejected)
{
    expectOutOfRange(Event{.type = static_cast<EventType>(0)});
    expectOutOfRange(Event{.type = static_cast<EventType>(9)});
}

TEST(EventTests, APlayerJoinedWithAPlayerIdOfFourIsRejected)
{
    expectOutOfRange(Event{.type = EventType::PLAYER_JOINED, .a = 4});
}

TEST(EventTests, APlayerLeftWithAPlayerIdOfFourIsRejected)
{
    expectOutOfRange(Event{.type = EventType::PLAYER_LEFT, .a = 4});
}

TEST(EventTests, APlayerLeftWithAnUnknownReasonIsRejected)
{
    expectOutOfRange(Event{.type = EventType::PLAYER_LEFT, .b = 3});
}

TEST(EventTests, APlayerLeftReasonDoesNotWrapPastAByte)
{
    expectOutOfRange(Event{.type = EventType::PLAYER_LEFT, .b = 256});
}
