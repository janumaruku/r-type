/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GameWorldTests
*/

#include "rtype/game/GameWorld.hpp"

#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <vector>

#include "engine/ecs/Registry.hpp"
#include "GameWorldTestAccess.hpp"
#include "rtype/game/Builders.hpp"
#include "rtype/game/Components.hpp"
#include "rtype/game/NetIdAllocator.hpp"

using engine::core::Vec2f;
using engine::ecs::Entity;
using rtype::game::EntityKind;
using rtype::game::GameEvent;
using rtype::game::GameWorld;
using rtype::game::GameWorldTestAccess;
using rtype::game::InputButtons;
using rtype::game::JoinPlayer;
using rtype::game::LeavePlayer;
using rtype::game::LeaveReason;
using rtype::game::NetId;
using rtype::game::NetIdAllocator;
using rtype::game::Phase;
using rtype::game::PlayerJoined;
using rtype::game::PlayerLeft;
using rtype::game::PlayerShip;
using rtype::game::ReplicatedEntity;
using rtype::game::Rules;
using rtype::game::SetInput;
using rtype::game::SlotState;
using rtype::game::Stage;

namespace {
constexpr float DT           = 1.0F / 60.0F;
constexpr std::uint32_t SEED = 7;
} // namespace

class World: public testing::Test {
protected:
    GameWorld _world{Rules{}, SEED};

    engine::ecs::Registry &registry()
    {
        return GameWorldTestAccess::registry(_world);
    }

    /*
    ** Reserves the lowest free slot, joins it, steps once so its ship
    ** spawns, and drops the PlayerJoined event.
    */
    std::uint8_t joinedPlayer(const std::string &name)
    {
        const std::uint8_t playerId = _world.reserveSlot().value();

        _world.apply(JoinPlayer{.playerId = playerId, .name = name});
        _world.step(DT);
        (void)_world.drainEvents();
        return playerId;
    }

    NetId shipOf(const std::uint8_t playerId)
    {
        return _world.roster().get(playerId)->shipNetId;
    }

    std::optional<Entity> find(const NetId netId)
    {
        return registry().getResource<NetIdAllocator>().find(netId, registry());
    }

    Entity missileOf(const std::uint8_t playerId, const Vec2f position)
    {
        const Entity missile = registry().spawnEntity();

        (void)rtype::game::makePlayerMissile(registry(), missile,
            shipOf(playerId), playerId, position);
        return missile;
    }

    InputButtons buttonsOf(const std::uint8_t playerId)
    {
        return registry()
            .getComponent<PlayerShip>(*find(shipOf(playerId)))
            ->buttons;
    }

    std::vector<ReplicatedEntity> replicated() const
    {
        std::vector<ReplicatedEntity> out;

        _world.collectReplicated(out);
        return out;
    }
};

TEST_F(World, NewWorldIsEmptyAtTickZero)
{
    EXPECT_EQ(_world.tick(), 0U);
    EXPECT_TRUE(replicated().empty());
    EXPECT_TRUE(_world.drainEvents().empty());
    EXPECT_EQ(_world.roster().active(), 0U);
    for (std::uint8_t playerId = 0; playerId < 4; ++playerId)
        EXPECT_EQ(_world.roster().get(playerId), nullptr);
    EXPECT_EQ(_world.phase(), Phase::PLAYING);
}

TEST_F(World, StageRunsTheFourteenSystemsInTickOrder)
{
    const std::vector<std::string> expected{"playerInput", "straightMover",
        "sineMover", "aimedShooter", "weapon", "movement", "playfield",
        "collision", "health", "lifetime", "spawnDirector", "respawn",
        "gameOver", "netIdCleanup"};

    EXPECT_EQ(registry().systemNames(Stage::SIMULATION), expected);
}

TEST_F(World, JoinedPlayerGetsAShipAtItsLaneOnTheNextStep)
{
    const std::uint8_t playerId = _world.reserveSlot().value();

    _world.apply(JoinPlayer{.playerId = playerId, .name = "alice"});
    EXPECT_EQ(_world.drainEvents(),
        std::vector<GameEvent>{PlayerJoined{.playerId = playerId}});
    EXPECT_TRUE(replicated().empty());
    _world.step(DT);

    const auto entities = replicated();

    ASSERT_EQ(entities.size(), 1U);
    EXPECT_EQ(entities[0].kind, EntityKind::PLAYER_SHIP);
    EXPECT_EQ(entities[0].netId, shipOf(playerId));
    EXPECT_EQ(entities[0].position, Rules{}.playerSpawn[playerId]);
    EXPECT_EQ(_world.roster().get(playerId)->state, SlotState::ALIVE);
}

TEST_F(World, JoinWithoutAReservedSlotDoesNothing)
{
    _world.apply(JoinPlayer{.playerId = 0, .name = "bob"});
    _world.step(DT);
    EXPECT_TRUE(_world.drainEvents().empty());
    EXPECT_TRUE(replicated().empty());
    EXPECT_EQ(_world.roster().get(0), nullptr);
}

TEST_F(World, OlderInputSequenceIsIgnored)
{
    const std::uint8_t playerId = joinedPlayer("alice");
    const InputButtons up{.bits = InputButtons::UP};
    const InputButtons down{.bits = InputButtons::DOWN};
    const InputButtons fire{.bits = InputButtons::FIRE};

    _world.apply(SetInput{
        .playerId = playerId, .inputSequence = 5, .buttons = up});
    EXPECT_EQ(buttonsOf(playerId), up);
    _world.apply(SetInput{
        .playerId = playerId, .inputSequence = 3, .buttons = down});
    EXPECT_EQ(buttonsOf(playerId), up);
    _world.apply(SetInput{
        .playerId = playerId, .inputSequence = 6, .buttons = fire});
    EXPECT_EQ(buttonsOf(playerId), fire);
}

TEST_F(World, LeaveRemovesTheShipAndOnlyThatPlayersMissiles)
{
    const std::uint8_t leaver = joinedPlayer("alice");
    const std::uint8_t stayer = joinedPlayer("bob");
    const Entity ship         = *find(shipOf(leaver));
    const Entity first        = missileOf(leaver, {400.0F, 200.0F});
    const Entity second       = missileOf(leaver, {500.0F, 200.0F});
    const Entity other        = missileOf(stayer, {400.0F, 400.0F});

    _world.apply(LeavePlayer{.playerId = leaver, .reason = LeaveReason::QUIT});
    EXPECT_FALSE(registry().isAlive(ship));
    EXPECT_FALSE(registry().isAlive(first));
    EXPECT_FALSE(registry().isAlive(second));
    EXPECT_TRUE(registry().isAlive(other));
    EXPECT_TRUE(find(shipOf(stayer)).has_value());
    EXPECT_EQ(_world.roster().get(leaver), nullptr);

    const std::vector<GameEvent> expected{
        PlayerLeft{.playerId = leaver, .reason = LeaveReason::QUIT}};

    EXPECT_EQ(_world.drainEvents(), expected);
}

TEST_F(World, CollectingTwiceIntoTheSameVectorKeepsItsCapacity)
{
    (void)joinedPlayer("alice");
    (void)joinedPlayer("bob");

    std::vector<ReplicatedEntity> out;

    _world.collectReplicated(out);

    const std::vector<ReplicatedEntity> first = out;
    const std::size_t capacity                = out.capacity();

    ASSERT_FALSE(first.empty());
    _world.collectReplicated(out);
    EXPECT_EQ(out, first);
    EXPECT_EQ(out.capacity(), capacity);
}

TEST_F(World, SixtyStepsCountSixtyTicks)
{
    for (int i = 0; i < 60; ++i)
        _world.step(DT);
    EXPECT_EQ(_world.tick(), 60U);
}
