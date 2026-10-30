/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** EventOutboxTests
*/

#include <gtest/gtest.h>

#include <vector>

#include "rtype/game/Commands.hpp"
#include "rtype/game/Events.hpp"

using rtype::game::EventOutbox;
using rtype::game::GameEvent;
using rtype::game::GameOver;
using rtype::game::JoinPlayer;
using rtype::game::LeaveReason;
using rtype::game::PlayerCommand;
using rtype::game::PlayerJoined;
using rtype::game::PlayerLeft;
using rtype::game::SetInput;

TEST(EventOutbox, DrainReturnsTheEventsInPushOrderThenNothing)
{
    EventOutbox outbox;
    const std::vector<GameEvent> expected{PlayerJoined{2},
        PlayerLeft{1, LeaveReason::TIMEOUT}, GameOver{1500}};

    for (const auto &event : expected)
        outbox.push(event);
    EXPECT_EQ(outbox.size(), 3U);
    EXPECT_EQ(outbox.drain(), expected);
    EXPECT_EQ(outbox.size(), 0U);
    EXPECT_TRUE(outbox.drain().empty());
}

TEST(EventOutbox, EventsCompareByValue)
{
    const GameEvent joined{PlayerJoined{1}};
    const GameEvent otherJoined{PlayerJoined{2}};
    const GameEvent left{PlayerLeft{1, LeaveReason::QUIT}};

    EXPECT_EQ(joined, GameEvent{PlayerJoined{1}});
    EXPECT_NE(joined, otherJoined);
    EXPECT_NE(joined, left);
}

TEST(PlayerCommand, CommandsCompareByValue)
{
    const PlayerCommand ana{JoinPlayer{0, "Ana"}};
    const PlayerCommand sameAna{JoinPlayer{0, "Ana"}};
    const PlayerCommand bob{JoinPlayer{0, "Bob"}};
    const PlayerCommand input{SetInput{0, 5, {}}};
    const PlayerCommand newerInput{SetInput{0, 6, {}}};

    EXPECT_EQ(ana, sameAna);
    EXPECT_NE(ana, bob);
    EXPECT_NE(input, newerInput);
}
