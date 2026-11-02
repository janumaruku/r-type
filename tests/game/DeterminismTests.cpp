/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** DeterminismTests
*/

#include <gtest/gtest.h>

#include <cstdint>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "ScriptedGame.hpp"
#include "engine/core/Rng.hpp"
#include "engine/core/Vec2.hpp"
#include "rtype/game/Commands.hpp"
#include "rtype/game/Events.hpp"
#include "rtype/game/GameWorld.hpp"

using engine::core::Vec2f;
using game_tests::runScript;
using game_tests::ScriptedResult;
using game_tests::ScriptedStep;
using rtype::game::EnemyKilled;
using rtype::game::EntityKind;
using rtype::game::Fired;
using rtype::game::InputButtons;
using rtype::game::JoinPlayer;
using rtype::game::LeavePlayer;
using rtype::game::LeaveReason;
using rtype::game::NetId;
using rtype::game::SetInput;

namespace {
constexpr std::uint32_t TICKS = 3600;
constexpr std::uint32_t INPUT_SEED = 1234;
constexpr std::uint32_t INPUT_EVERY = 15;
constexpr std::uint32_t LEAVE_TICK = 2400;
constexpr std::uint8_t PLAYERS = 4;
constexpr std::uint8_t LEAVER = 2;

/*
** Four players join on the first ticks, then every 15 ticks each player
** still in the game sends random buttons, drawn from a generator with its
** own fixed seed. Player 2 leaves at tick 2400.
*/
std::vector<ScriptedStep> fourPlayerScript()
{
    std::vector<ScriptedStep> script;
    engine::core::Rng inputs(INPUT_SEED);
    std::uint32_t sequence = 0;

    for (std::uint8_t id = 0; id < PLAYERS; ++id)
        script.push_back({.tick = id,
            .command = JoinPlayer{.playerId = id,
                .name = "player" + std::to_string(id)}});
    for (std::uint32_t tick = PLAYERS; tick < TICKS; tick += INPUT_EVERY) {
        if (tick >= LEAVE_TICK && tick < LEAVE_TICK + INPUT_EVERY)
            script.push_back({.tick = tick,
                .command = LeavePlayer{.playerId = LEAVER,
                    .reason = LeaveReason::QUIT}});
        ++sequence;
        for (std::uint8_t id = 0; id < PLAYERS; ++id) {
            if (id == LEAVER && tick >= LEAVE_TICK)
                continue;
            script.push_back({.tick = tick,
                .command = SetInput{.playerId = id,
                    .inputSequence = sequence,
                    .buttons = InputButtons{.bits = static_cast<std::uint8_t>(
                        inputs.uniformInt(0, 31))}}});
        }
    }
    return script;
}

template <class Event>
std::size_t countOf(const ScriptedResult &result)
{
    std::size_t count = 0;

    for (const auto &[tick, event]: result.events)
        count += std::holds_alternative<Event>(event) ? 1 : 0;
    return count;
}

/*
** The position of each Bydos the first time it's replicated, in order of
** appearance: where the spawn director put it.
*/
std::vector<Vec2f> bydosSpawns(const ScriptedResult &result)
{
    std::vector<Vec2f> spawns;
    std::set<NetId> seen;

    for (const auto &tick: result.replicated)
        for (const auto &entity: tick)
            if (entity.kind == EntityKind::BYDOS_SLAVE
                && seen.insert(entity.netId).second)
                spawns.push_back(entity.position);
    return spawns;
}
}

TEST(Determinism, SameSeedAndScriptGiveTheSameGameBitForBit)
{
    const auto script = fourPlayerScript();
    const ScriptedResult first = runScript(42, script, TICKS, true);
    const ScriptedResult second = runScript(42, script, TICKS, true);

    ASSERT_GT(countOf<Fired>(first), 0U);
    ASSERT_GT(countOf<EnemyKilled>(first), 0U);
    ASSERT_EQ(first.replicated.size(), TICKS);
    EXPECT_EQ(first.events, second.events);
    for (std::uint32_t tick = 0; tick < TICKS; ++tick)
        ASSERT_EQ(first.replicated[tick], second.replicated[tick])
            << "tick " << tick + 1;
}

TEST(Determinism, TheSeedChangesWhereEnemiesSpawn)
{
    const auto script = fourPlayerScript();
    const auto first = bydosSpawns(runScript(1, script, TICKS, true));
    const auto second = bydosSpawns(runScript(2, script, TICKS, true));

    ASSERT_FALSE(first.empty());
    ASSERT_FALSE(second.empty());
    EXPECT_NE(first, second);
}
