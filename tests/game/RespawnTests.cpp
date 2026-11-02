/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RespawnTests
*/

#include <gtest/gtest.h>

#include "GameSystemsFixture.hpp"

using engine::ecs::Entity;
using rtype::game::NetId;
using rtype::game::NO_NET_ID;
using rtype::game::PlayerShip;
using rtype::game::SlotState;
using rtype::game::Transform;

class Respawn: public game_tests::GameSystems {
protected:
    void SetUp() override
    {
        GameSystems::SetUp();
        addRespawnSystem();
    }

    std::uint8_t joinPlayer()
    {
        const std::uint8_t playerId = roster().reserve().value();

        EXPECT_TRUE(roster().join(playerId, "player", 3));
        return playerId;
    }

    void expectShipAtItsLane(const std::uint8_t playerId)
    {
        const Entity ship = entityOf(slot(playerId).shipNetId);

        EXPECT_EQ(component<PlayerShip>(ship).playerId, playerId);
        EXPECT_EQ(component<Transform>(ship).position,
            rules().playerSpawn[playerId]);
        EXPECT_FLOAT_EQ(component<PlayerShip>(ship).invulnerableFor,
            rules().respawnInvulnerability);
    }
};

TEST_F(Respawn, JoinedSlotIsAliveAfterOneTick)
{
    const std::uint8_t playerId = joinPlayer();

    ASSERT_EQ(slot(playerId).state, SlotState::RESPAWNING);
    tick();
    EXPECT_EQ(slot(playerId).state, SlotState::ALIVE);
    ASSERT_NE(slot(playerId).shipNetId, NO_NET_ID);
    expectShipAtItsLane(playerId);
}

TEST_F(Respawn, EveryJoinedSlotGetsItsOwnLane)
{
    const std::uint8_t first = joinPlayer();
    const std::uint8_t second = joinPlayer();

    tick();
    EXPECT_EQ(countOf<PlayerShip>(), 2U);
    expectShipAtItsLane(first);
    expectShipAtItsLane(second);
}

TEST_F(Respawn, SlotRespawningForTwoSecondsIsAliveAfter120Ticks)
{
    const Entity old = spawnAliveShip();
    const NetId oldNetId = netIdOf(old);

    ASSERT_TRUE(roster().onShipKilled(0, 2.0F));
    _registry.killEntity(old);
    tick(119);
    EXPECT_EQ(slot(0).state, SlotState::RESPAWNING);
    EXPECT_EQ(countOf<PlayerShip>(), 0U);
    tick();
    EXPECT_EQ(slot(0).state, SlotState::ALIVE);
    EXPECT_NE(slot(0).shipNetId, oldNetId);
    EXPECT_EQ(slot(0).lives, 2);
    expectShipAtItsLane(0);
}
