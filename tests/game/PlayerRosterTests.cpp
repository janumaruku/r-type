/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** PlayerRosterTests
*/

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "engine/core/Logger.hpp"
#include "rtype/game/PlayerRoster.hpp"

using rtype::game::NO_NET_ID;
using rtype::game::PlayerRoster;
using rtype::game::PlayerSlot;
using rtype::game::SlotState;

namespace {

constexpr std::uint8_t LIVES = 3;
constexpr float RESPAWN_DELAY = 2.0F;
constexpr rtype::game::NetId SHIP = 7;

void expectSameSlot(const PlayerSlot &actual, const PlayerSlot &expected)
{
    EXPECT_EQ(actual.playerId, expected.playerId);
    EXPECT_EQ(actual.name, expected.name);
    EXPECT_EQ(actual.lives, expected.lives);
    EXPECT_EQ(actual.score, expected.score);
    EXPECT_EQ(actual.state, expected.state);
    EXPECT_EQ(actual.respawnTimer, expected.respawnTimer);
    EXPECT_EQ(actual.shipNetId, expected.shipNetId);
    EXPECT_EQ(actual.lastInputSequence, expected.lastInputSequence);
}

/*
** Brings a fresh roster's slot 0 to `state` through the transitions only.
*/
void reach(PlayerRoster &roster, const SlotState state)
{
    ASSERT_EQ(roster.reserve(), 0);
    if (state == SlotState::OUT) {
        ASSERT_TRUE(roster.joinWaiting(0, "Ana"));
        return;
    }
    if (state == SlotState::EMPTY)
        return;
    ASSERT_TRUE(roster.join(0, "Ana", LIVES));
    if (state == SlotState::ALIVE)
        ASSERT_TRUE(roster.onShipSpawned(0, SHIP));
}

}

/*
** The log output is process-wide: each test captures it and gives
** std::cerr back when it ends.
*/
class PlayerRosterLog: public testing::Test {
protected:
    void SetUp() override
    {
        engine::core::log::setOutput(_output);
    }

    void TearDown() override
    {
        engine::core::log::setOutput(std::cerr);
    }

    [[nodiscard]] std::size_t loggedLines() const
    {
        const std::string text = _output.str();

        return static_cast<std::size_t>(std::ranges::count(text, '\n'));
    }

    std::ostringstream _output;
};

TEST(PlayerRoster, ReservesTheLowestFreeIdUntilFull)
{
    PlayerRoster roster;

    EXPECT_EQ(roster.reserve(), 0);
    EXPECT_EQ(roster.reserve(), 1);
    EXPECT_EQ(roster.reserve(), 2);
    EXPECT_EQ(roster.reserve(), 3);
    EXPECT_EQ(roster.reserve(), std::nullopt);
    EXPECT_EQ(roster.active(), 4U);
    roster.release(1);
    EXPECT_EQ(roster.active(), 3U);
    EXPECT_EQ(roster.reserve(), 1);
}

TEST(PlayerRoster, ReservedSlotStartsEmpty)
{
    PlayerRoster roster;

    ASSERT_EQ(roster.reserve(), 0);
    ASSERT_NE(roster.get(0), nullptr);
    expectSameSlot(*roster.get(0), PlayerSlot{.playerId = 0});
}

TEST(PlayerRoster, ReleaseIgnoresAFreeSlotAndAnIdOutOfRange)
{
    PlayerRoster roster;

    roster.release(0);
    roster.release(4);
    roster.release(255);
    EXPECT_EQ(roster.active(), 0U);
    EXPECT_EQ(roster.get(0), nullptr);
    EXPECT_EQ(roster.get(4), nullptr);
}

TEST(PlayerRoster, JoinThenRespawnThenSpawn)
{
    PlayerRoster roster;

    ASSERT_EQ(roster.reserve(), 0);
    ASSERT_TRUE(roster.join(0, "Ana", LIVES));
    EXPECT_EQ(roster.get(0)->state, SlotState::RESPAWNING);
    EXPECT_EQ(roster.get(0)->respawnTimer, 0.0F);
    EXPECT_EQ(roster.get(0)->name, "Ana");
    EXPECT_EQ(roster.get(0)->lives, LIVES);
    EXPECT_EQ(roster.advanceRespawns(0.0F), std::vector<std::uint8_t>{0});
    EXPECT_EQ(roster.get(0)->state, SlotState::RESPAWNING);
    ASSERT_TRUE(roster.onShipSpawned(0, SHIP));
    EXPECT_EQ(roster.get(0)->state, SlotState::ALIVE);
    EXPECT_EQ(roster.get(0)->shipNetId, SHIP);
    EXPECT_TRUE(roster.anyAlive());
}

TEST(PlayerRoster, AdvanceRespawnsWaitsForTheTimer)
{
    PlayerRoster roster;

    reach(roster, SlotState::ALIVE);
    ASSERT_TRUE(roster.onShipKilled(0, RESPAWN_DELAY));
    EXPECT_TRUE(roster.advanceRespawns(1.0F).empty());
    EXPECT_EQ(roster.get(0)->respawnTimer, 1.0F);
    EXPECT_EQ(roster.advanceRespawns(1.5F), std::vector<std::uint8_t>{0});
    EXPECT_EQ(roster.get(0)->respawnTimer, 0.0F);
    EXPECT_EQ(roster.get(0)->state, SlotState::RESPAWNING);
}

TEST(PlayerRoster, ShipKilledUntilOut)
{
    PlayerRoster roster;

    reach(roster, SlotState::ALIVE);
    for (const std::uint8_t livesLeft : {2, 1}) {
        ASSERT_TRUE(roster.onShipKilled(0, RESPAWN_DELAY));
        EXPECT_EQ(roster.get(0)->state, SlotState::RESPAWNING);
        EXPECT_EQ(roster.get(0)->lives, livesLeft);
        EXPECT_EQ(roster.get(0)->respawnTimer, RESPAWN_DELAY);
        EXPECT_EQ(roster.get(0)->shipNetId, NO_NET_ID);
        ASSERT_TRUE(roster.onShipSpawned(0, SHIP));
    }
    ASSERT_TRUE(roster.onShipKilled(0, RESPAWN_DELAY));
    EXPECT_EQ(roster.get(0)->state, SlotState::OUT);
    EXPECT_EQ(roster.get(0)->lives, 0);
    EXPECT_EQ(roster.get(0)->shipNetId, NO_NET_ID);
    EXPECT_TRUE(roster.allOut());
}

TEST(PlayerRoster, ShipKilledWithNoLifeGoesOutWithoutWrapping)
{
    PlayerRoster roster;

    ASSERT_EQ(roster.reserve(), 0);
    ASSERT_TRUE(roster.join(0, "Ana", 0));
    ASSERT_TRUE(roster.onShipSpawned(0, SHIP));
    ASSERT_TRUE(roster.onShipKilled(0, RESPAWN_DELAY));
    EXPECT_EQ(roster.get(0)->state, SlotState::OUT);
    EXPECT_EQ(roster.get(0)->lives, 0);
}

TEST_F(PlayerRosterLog, EveryTransitionRefusesEveryWrongState)
{
    struct Transition {
        std::string name;
        SlotState from;
        std::function<bool(PlayerRoster &)> call;
    };
    const std::vector<Transition> transitions{
        {"join", SlotState::EMPTY,
            [](PlayerRoster &r) { return r.join(0, "Bob", LIVES); }},
        {"joinWaiting", SlotState::EMPTY,
            [](PlayerRoster &r) { return r.joinWaiting(0, "Bob"); }},
        {"onShipSpawned", SlotState::RESPAWNING,
            [](PlayerRoster &r) { return r.onShipSpawned(0, SHIP + 1); }},
        {"onShipKilled", SlotState::ALIVE,
            [](PlayerRoster &r) {
                return r.onShipKilled(0, RESPAWN_DELAY);
            }},
    };

    for (const auto &transition : transitions) {
        for (const auto state : {SlotState::EMPTY, SlotState::ALIVE,
                 SlotState::RESPAWNING, SlotState::OUT}) {
            if (state == transition.from)
                continue;
            SCOPED_TRACE(transition.name + " from state "
                + std::to_string(static_cast<int>(state)));
            PlayerRoster roster;

            reach(roster, state);
            const PlayerSlot before = *roster.get(0);

            _output.str("");
            EXPECT_FALSE(transition.call(roster));
            expectSameSlot(*roster.get(0), before);
            EXPECT_EQ(loggedLines(), 1U);
            EXPECT_NE(_output.str().find(transition.name), std::string::npos);
        }
    }
}

TEST_F(PlayerRosterLog, TransitionsRefuseAFreeSlotAndAnIdOutOfRange)
{
    PlayerRoster roster;

    for (const std::uint8_t id : {0, 4}) {
        SCOPED_TRACE(static_cast<int>(id));
        _output.str("");
        EXPECT_FALSE(roster.join(id, "Bob", LIVES));
        EXPECT_FALSE(roster.joinWaiting(id, "Bob"));
        EXPECT_FALSE(roster.onShipSpawned(id, SHIP));
        EXPECT_FALSE(roster.onShipKilled(id, RESPAWN_DELAY));
        EXPECT_FALSE(roster.addScore(id, 100));
        EXPECT_FALSE(roster.acceptInput(id, 1));
        EXPECT_EQ(loggedLines(), 6U);
    }
    EXPECT_EQ(roster.active(), 0U);
}

TEST(PlayerRoster, JoinWaitingThenRestart)
{
    PlayerRoster roster;

    reach(roster, SlotState::OUT);
    EXPECT_EQ(roster.get(0)->state, SlotState::OUT);
    EXPECT_EQ(roster.get(0)->lives, 0);
    ASSERT_TRUE(roster.addScore(0, 300));
    roster.onGameRestarted(LIVES);
    EXPECT_EQ(roster.get(0)->state, SlotState::RESPAWNING);
    EXPECT_EQ(roster.get(0)->lives, LIVES);
    EXPECT_EQ(roster.get(0)->score, 0U);
    EXPECT_EQ(roster.get(0)->respawnTimer, 0.0F);
}

TEST(PlayerRoster, RestartResetsJoinedSlotsAndKeepsEmptyOnes)
{
    PlayerRoster roster;

    reach(roster, SlotState::ALIVE);
    ASSERT_EQ(roster.reserve(), 1);
    roster.onGameRestarted(LIVES);
    EXPECT_EQ(roster.get(0)->state, SlotState::RESPAWNING);
    EXPECT_EQ(roster.get(0)->shipNetId, NO_NET_ID);
    EXPECT_EQ(roster.get(1)->state, SlotState::EMPTY);
    EXPECT_TRUE(roster.join(1, "Bob", LIVES));
}

TEST(PlayerRoster, AcceptInputOnlyTakesNewerSequences)
{
    PlayerRoster roster;

    ASSERT_EQ(roster.reserve(), 0);
    EXPECT_TRUE(roster.acceptInput(0, 5));
    EXPECT_FALSE(roster.acceptInput(0, 3));
    EXPECT_FALSE(roster.acceptInput(0, 5));
    EXPECT_TRUE(roster.acceptInput(0, 6));
    EXPECT_EQ(roster.get(0)->lastInputSequence, 6U);
}

TEST_F(PlayerRosterLog, AcceptInputDoesNotLogALateSequence)
{
    PlayerRoster roster;

    ASSERT_EQ(roster.reserve(), 0);
    ASSERT_TRUE(roster.acceptInput(0, 5));
    EXPECT_FALSE(roster.acceptInput(0, 3));
    EXPECT_EQ(loggedLines(), 0U);
}

TEST(PlayerRoster, AllOut)
{
    PlayerRoster none;
    EXPECT_FALSE(none.allOut());

    PlayerRoster oneEmpty;
    ASSERT_EQ(oneEmpty.reserve(), 0);
    EXPECT_FALSE(oneEmpty.allOut());

    PlayerRoster twoOut;
    ASSERT_EQ(twoOut.reserve(), 0);
    ASSERT_EQ(twoOut.reserve(), 1);
    ASSERT_TRUE(twoOut.joinWaiting(0, "Ana"));
    ASSERT_TRUE(twoOut.joinWaiting(1, "Bob"));
    EXPECT_TRUE(twoOut.allOut());

    PlayerRoster outAndRespawning;
    ASSERT_EQ(outAndRespawning.reserve(), 0);
    ASSERT_EQ(outAndRespawning.reserve(), 1);
    ASSERT_TRUE(outAndRespawning.joinWaiting(0, "Ana"));
    ASSERT_TRUE(outAndRespawning.join(1, "Bob", LIVES));
    EXPECT_FALSE(outAndRespawning.allOut());
}

TEST(PlayerRoster, AnyAliveOnlyCountsShipsOnScreen)
{
    PlayerRoster roster;

    reach(roster, SlotState::RESPAWNING);
    EXPECT_FALSE(roster.anyAlive());
    ASSERT_TRUE(roster.onShipSpawned(0, SHIP));
    EXPECT_TRUE(roster.anyAlive());
}

TEST(PlayerRoster, TeamScoreSumsTheReservedSlots)
{
    PlayerRoster roster;

    ASSERT_EQ(roster.reserve(), 0);
    ASSERT_EQ(roster.reserve(), 1);
    ASSERT_TRUE(roster.addScore(0, 100));
    ASSERT_TRUE(roster.addScore(1, 250));
    ASSERT_TRUE(roster.addScore(0, 50));
    EXPECT_EQ(roster.teamScore(), 400U);
    roster.release(1);
    EXPECT_EQ(roster.teamScore(), 150U);
}
