/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ScriptedGame
*/

#ifndef R_TYPE_SCRIPTEDGAME_HPP
#define R_TYPE_SCRIPTEDGAME_HPP

#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <utility>
#include <variant>
#include <vector>

#include "rtype/game/Commands.hpp"
#include "rtype/game/Events.hpp"
#include "rtype/game/GameWorld.hpp"
#include "rtype/game/Rules.hpp"

namespace game_tests {

/**
 * @brief The default rules, except that one player is enough to start: the
 * tests that drive a whole GameWorld join one or two players.
 */
inline rtype::game::Rules soloRules()
{
    rtype::game::Rules rules;

    rules.playersToStart = 1;
    return rules;
}

inline constexpr float SCRIPT_DT = 1.0F / 60.0F;

/**
 * @brief A command and the tick before which it's applied: tick 0 is applied
 * before the first step.
 */
struct ScriptedStep {
    std::uint32_t tick;
    rtype::game::PlayerCommand command;
};

/**
 * @brief Every event with the tick of the step that drained it, and the
 * replicated entities after each step when they were recorded.
 */
struct ScriptedResult {
    std::vector<std::pair<std::uint32_t, rtype::game::GameEvent>> events;
    std::vector<std::vector<rtype::game::ReplicatedEntity>> replicated;
};

/**
 * @brief Plays `script` on a new `GameWorld{soloRules(), seed}` for `ticks`
 * steps of 1/60 s, as the server would: before each step, the commands of
 * that tick are applied in script order. A @ref rtype::game::JoinPlayer
 * first reserves the lowest free slot, as the server does when a client
 * connects, so a script joins its players in slot order.
 * @param script Steps sorted by tick.
 * @param recordReplicated `true` to keep the replicated entities after
 * every step.
 */
inline ScriptedResult runScript(const std::uint32_t seed,
    const std::vector<ScriptedStep> &script, const std::uint32_t ticks,
    const bool recordReplicated = false)
{
    rtype::game::GameWorld world{soloRules(), seed};
    ScriptedResult result;
    std::size_t next = 0;

    for (std::uint32_t tick = 0; tick < ticks; ++tick) {
        for (; next < script.size() && script[next].tick == tick; ++next) {
            const auto *join =
                std::get_if<rtype::game::JoinPlayer>(&script[next].command);

            if (join != nullptr) {
                EXPECT_EQ(world.reserveSlot(), join->playerId);
            }
            world.apply(script[next].command);
        }
        world.step(SCRIPT_DT);
        for (auto &event: world.drainEvents())
            result.events.emplace_back(world.tick(), std::move(event));
        if (recordReplicated)
            world.collectReplicated(result.replicated.emplace_back());
    }
    return result;
}

} // namespace game_tests

#endif // R_TYPE_SCRIPTEDGAME_HPP
