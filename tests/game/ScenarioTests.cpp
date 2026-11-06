/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ScenarioTests
*/

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "engine/core/Vec2.hpp"
#include "engine/ecs/Registry.hpp"
#include "GameWorldTestAccess.hpp"
#include "rtype/game/Builders.hpp"
#include "rtype/game/Components.hpp"
#include "rtype/game/GameWorld.hpp"
#include "rtype/game/Motion.hpp"
#include "rtype/game/NetIdAllocator.hpp"
#include "rtype/game/SpawnDirectorState.hpp"
#include "ScriptedGame.hpp"

using engine::core::Vec2f;
using engine::ecs::Entity;
using game_tests::SCRIPT_DT;
using rtype::game::EnemyKilled;
using rtype::game::EntityKind;
using rtype::game::Fired;
using rtype::game::GameEvent;
using rtype::game::GameOver;
using rtype::game::GameRestarted;
using rtype::game::GameWorld;
using rtype::game::GameWorldTestAccess;
using rtype::game::InputButtons;
using rtype::game::JoinPlayer;
using rtype::game::LeavePlayer;
using rtype::game::LeaveReason;
using rtype::game::MovePattern;
using rtype::game::NetId;
using rtype::game::NetIdAllocator;
using rtype::game::NO_NET_ID;
using rtype::game::Phase;
using rtype::game::PlayerJoined;
using rtype::game::PlayerKilled;
using rtype::game::PlayerLeft;
using rtype::game::PlayerShip;
using rtype::game::Projectile;
using rtype::game::Rules;
using rtype::game::SetInput;
using rtype::game::Side;
using rtype::game::SlotState;
using rtype::game::SpawnDirectorState;
using rtype::game::Transform;

namespace {
constexpr std::uint32_t SEED = 7;
constexpr float NO_SPAWN     = 1.0e9F;
} // namespace

/*
** A GameWorld driven through its public interface, as the server drives it.
** Every drained event is kept with its tick. The spawn director is held
** back (holdSpawns) so random Bydos and their missiles can't change the
** outcome; each scenario places the entities it needs.
*/
class Scenario: public testing::Test {
protected:
    GameWorld _world{Rules{}, SEED};
    Rules _rules{};
    std::vector<std::pair<std::uint32_t, GameEvent>> _events;
    std::uint32_t _sequence = 0;

    void SetUp() override
    {
        holdSpawns();
    }

    engine::ecs::Registry &registry()
    {
        return GameWorldTestAccess::registry(_world);
    }

    void holdSpawns()
    {
        registry().getResource<SpawnDirectorState>().untilNextSpawn = NO_SPAWN;
    }

    void play(const std::uint32_t ticks = 1)
    {
        for (std::uint32_t i = 0; i < ticks; ++i) {
            _world.step(SCRIPT_DT);
            for (auto &event: _world.drainEvents())
                _events.emplace_back(_world.tick(), std::move(event));
        }
    }

    /*
    ** Steps until an event of type Event is drained, at most `limit` ticks.
    ** @return The event, or std::nullopt if it never came.
    */
    template <class Event>
    std::optional<Event> playUntil(const std::uint32_t limit)
    {
        const std::size_t seen = _events.size();

        for (std::uint32_t i = 0; i < limit; ++i) {
            play();
            for (std::size_t e = seen; e < _events.size(); ++e)
                if (const auto *event = std::get_if<Event>(&_events[e].second))
                    return *event;
        }
        return std::nullopt;
    }

    std::uint8_t join(const std::string &name)
    {
        const std::uint8_t playerId = _world.reserveSlot().value();

        _world.apply(JoinPlayer{.playerId = playerId, .name = name});
        return playerId;
    }

    void hold(const std::uint8_t playerId, const std::uint8_t bits)
    {
        _world.apply(SetInput{.playerId = playerId,
            .inputSequence              = ++_sequence,
            .buttons                    = InputButtons{.bits = bits}});
    }

    const rtype::game::PlayerSlot &slot(const std::uint8_t playerId)
    {
        return *_world.roster().get(playerId);
    }

    std::optional<Entity> find(const NetId netId)
    {
        return registry().getResource<NetIdAllocator>().find(netId, registry());
    }

    Entity shipOf(const std::uint8_t playerId)
    {
        return find(slot(playerId).shipNetId).value();
    }

    Vec2f positionOf(const Entity entity)
    {
        return registry().getComponent<Transform>(entity)->position;
    }

    /*
    ** Steps until the player's ship has lost its spawn protection.
    */
    void waitForVulnerable(const std::uint8_t playerId)
    {
        for (int i = 0; i < 300; ++i) {
            const auto *ship =
                registry().getComponent<PlayerShip>(shipOf(playerId));

            if (ship->invulnerableFor <= 0.0F)
                return;
            play();
        }
        FAIL() << "player " << int{playerId} << " stayed invulnerable";
    }

    /*
    ** An enemy missile 40 units in front of the player's ship, flying at
    ** it: it hits a few ticks later.
    */
    void fireAt(const std::uint8_t playerId)
    {
        const Entity missile = registry().spawnEntity();

        (void)rtype::game::makeEnemyMissile(registry(), missile, NO_NET_ID,
            positionOf(shipOf(playerId)) + Vec2f{40.0F, 0.0F},
            Vec2f{-1.0F, 0.0F});
    }

    /*
    ** Waits for the ship, lets its protection run out, and kills it.
    */
    std::optional<PlayerKilled> killPlayer(const std::uint8_t playerId)
    {
        for (int i = 0; i < 300 && slot(playerId).state != SlotState::ALIVE;
            ++i)
            play();
        waitForVulnerable(playerId);
        fireAt(playerId);
        return playUntil<PlayerKilled>(60);
    }

    template <class Event>
    std::size_t countOf() const
    {
        std::size_t count = 0;

        for (const auto &[tick, event]: _events)
            count += std::holds_alternative<Event>(event) ? 1 : 0;
        return count;
    }

    template <class Event>
    std::uint32_t tickOf() const
    {
        for (const auto &[tick, event]: _events)
            if (std::holds_alternative<Event>(event))
                return tick;
        ADD_FAILURE() << "event never drained";
        return 0;
    }

    std::size_t countOwnedBy(const std::uint8_t playerId)
    {
        std::size_t count = 0;

        for (auto &&[entity, projectile]:
            registry().indexedView<const Projectile>())
            count += projectile.ownerPlayerId == playerId ? 1 : 0;
        return count;
    }
};

TEST_F(Scenario, JoinedShipAppearsOneTickLaterAndMovesLikeMovePlayer)
{
    const std::uint8_t player = join("alice");

    std::vector<rtype::game::ReplicatedEntity> replicated;

    _world.collectReplicated(replicated);
    EXPECT_TRUE(replicated.empty());
    play();
    _world.collectReplicated(replicated);
    ASSERT_EQ(replicated.size(), 1U);
    EXPECT_EQ(replicated[0].kind, EntityKind::PLAYER_SHIP);

    const Entity ship = shipOf(player);
    Vec2f predicted   = _rules.playerSpawn[player];

    EXPECT_EQ(positionOf(ship), predicted);
    hold(player, InputButtons::RIGHT);
    for (int i = 0; i < 60; ++i) {
        play();
        predicted = movePlayer(predicted, InputButtons{InputButtons::RIGHT},
            SCRIPT_DT, _rules);
        ASSERT_EQ(positionOf(ship), predicted) << "tick " << i + 1;
    }
    EXPECT_GT(predicted.x(), _rules.playerSpawn[player].x());
}

TEST_F(Scenario, ShootingABydosOnTheLaneScoresTheKill)
{
    const std::uint8_t player = join("alice");

    play();

    const NetId ship       = slot(player).shipNetId;
    const Entity bydos     = registry().spawnEntity();
    const NetId bydosNetId = rtype::game::makeBydosSlave(registry(), bydos,
        _rules.playerSpawn[player] + Vec2f{480.0F, 0.0F},
        MovePattern::STRAIGHT);

    hold(player, InputButtons::FIRE);

    const auto killed = playUntil<EnemyKilled>(120);

    ASSERT_TRUE(killed.has_value());
    EXPECT_LT(tickOf<Fired>(), tickOf<EnemyKilled>());

    const auto &firstShot = std::get<Fired>(std::find_if(_events.begin(),
        _events.end(), [](const auto &entry) {
            return std::holds_alternative<Fired>(entry.second);
        })->second);

    EXPECT_EQ(firstShot.shooter, ship);
    EXPECT_EQ(firstShot.side, Side::PLAYER);
    EXPECT_EQ(killed->killerShip, firstShot.missile);
    EXPECT_EQ(killed->enemy, bydosNetId);
    EXPECT_EQ(slot(player).score, _rules.killScore);
    EXPECT_FALSE(registry().isAlive(bydos));
}

TEST_F(Scenario, KilledShipComesBackTwoSecondsLaterProtected)
{
    const std::uint8_t player = join("alice");
    const auto killed         = killPlayer(player);

    ASSERT_TRUE(killed.has_value());
    EXPECT_EQ(killed->playerId, player);
    EXPECT_EQ(slot(player).lives, 2);
    EXPECT_EQ(slot(player).state, SlotState::RESPAWNING);

    const std::uint32_t death = _world.tick();

    play(118);
    EXPECT_EQ(slot(player).state, SlotState::RESPAWNING);
    play();
    ASSERT_EQ(slot(player).state, SlotState::ALIVE);
    EXPECT_EQ(_world.tick() - death, 119U);
    EXPECT_NE(slot(player).shipNetId, killed->ship);
    EXPECT_EQ(positionOf(shipOf(player)), _rules.playerSpawn[player]);
    EXPECT_FLOAT_EQ(registry()
                        .getComponent<PlayerShip>(shipOf(player))
                        ->invulnerableFor,
        _rules.respawnInvulnerability);
}

TEST_F(Scenario, LosingTheLastLifeEndsTheGameThenRestartsIt)
{
    const std::uint8_t player = join("alice");

    play();

    const Entity bydos = registry().spawnEntity();

    (void)rtype::game::makeBydosSlave(registry(), bydos,
        _rules.playerSpawn[player] + Vec2f{480.0F, 0.0F},
        MovePattern::STRAIGHT);
    hold(player, InputButtons::FIRE);
    ASSERT_TRUE(playUntil<EnemyKilled>(120).has_value());
    hold(player, 0);
    for (int life = 0; life < 3; ++life)
        ASSERT_TRUE(killPlayer(player).has_value()) << "life " << life;
    play();

    EXPECT_EQ(countOf<GameOver>(), 1U);
    EXPECT_EQ(std::get<GameOver>(std::find_if(_events.begin(), _events.end(),
                                     [](const auto &entry) {
                                         return std::holds_alternative<
                                             GameOver>(entry.second);
                                     })
                                     ->second)
                  .teamScore,
        _rules.killScore);
    EXPECT_EQ(_world.phase(), Phase::GAME_OVER);
    EXPECT_EQ(slot(player).state, SlotState::SLOT_OUT);

    ASSERT_TRUE(playUntil<GameRestarted>(400).has_value());
    EXPECT_EQ(tickOf<GameRestarted>() - tickOf<GameOver>(), 300U);
    EXPECT_EQ(countOf<GameOver>(), 1U);
    EXPECT_EQ(_world.phase(), Phase::PLAYING);
    EXPECT_EQ(slot(player).state, SlotState::RESPAWNING);
    play();
    EXPECT_EQ(slot(player).state, SlotState::ALIVE);
    EXPECT_EQ(slot(player).lives, 3);
    EXPECT_EQ(slot(player).score, 0U);
}

TEST_F(Scenario, PlayerJoiningDuringAGameOverPlaysAfterTheRestart)
{
    const std::uint8_t first = join("alice");

    for (int life = 0; life < 3; ++life)
        ASSERT_TRUE(killPlayer(first).has_value()) << "life " << life;
    play();
    ASSERT_EQ(_world.phase(), Phase::GAME_OVER);

    const std::size_t seen  = _events.size();
    const std::uint8_t late = join("bob");

    play();
    ASSERT_GT(_events.size(), seen);
    EXPECT_EQ(_events[seen].second, GameEvent{PlayerJoined{.playerId = late}});
    EXPECT_EQ(slot(late).state, SlotState::SLOT_OUT);
    EXPECT_EQ(slot(late).lives, 0);
    EXPECT_EQ(_world.phase(), Phase::GAME_OVER);

    ASSERT_TRUE(playUntil<GameRestarted>(400).has_value());
    play();
    EXPECT_EQ(slot(first).state, SlotState::ALIVE);
    EXPECT_EQ(slot(late).state, SlotState::ALIVE);
    EXPECT_EQ(slot(late).lives, _rules.playerLives);
}

TEST_F(Scenario, PlayerLeavingMidGameLeavesTheOtherOneUntouched)
{
    const std::uint8_t stayer = join("alice");
    const std::uint8_t leaver = join("bob");

    play();
    hold(stayer, InputButtons::FIRE);
    hold(leaver, InputButtons::FIRE);
    play(30);
    ASSERT_GT(countOwnedBy(leaver), 0U);
    ASSERT_GT(countOwnedBy(stayer), 0U);

    const NetId stayerShip           = slot(stayer).shipNetId;
    const NetId leaverShip           = slot(leaver).shipNetId;
    const std::size_t stayerMissiles = countOwnedBy(stayer);
    const std::size_t seen           = _events.size();

    _world.apply(LeavePlayer{.playerId = leaver, .reason = LeaveReason::QUIT});
    EXPECT_EQ(countOwnedBy(leaver), 0U);
    EXPECT_FALSE(find(leaverShip).has_value());
    EXPECT_EQ(countOwnedBy(stayer), stayerMissiles);
    EXPECT_EQ(_world.roster().get(leaver), nullptr);

    play();
    ASSERT_GT(_events.size(), seen);

    const GameEvent left =
        PlayerLeft{.playerId = leaver, .reason = LeaveReason::QUIT};

    EXPECT_EQ(_events[seen].second, left);
    EXPECT_EQ(slot(stayer).state, SlotState::ALIVE);
    EXPECT_EQ(slot(stayer).shipNetId, stayerShip);
    EXPECT_EQ(_world.phase(), Phase::PLAYING);

    std::vector<rtype::game::ReplicatedEntity> replicated;

    _world.collectReplicated(replicated);
    for (const auto &entity: replicated)
        EXPECT_NE(entity.netId, leaverShip);
}
