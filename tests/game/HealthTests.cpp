/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** HealthTests
*/

#include <gtest/gtest.h>

#include "GameSystemsFixture.hpp"

using engine::core::Vec2f;
using engine::ecs::Entity;
using rtype::game::EnemyKilled;
using rtype::game::Health;
using rtype::game::LastHit;
using rtype::game::NetId;
using rtype::game::NO_NET_ID;
using rtype::game::PlayerKilled;
using rtype::game::SlotState;

class Healths: public game_tests::GameSystems {
protected:
    void SetUp() override
    {
        GameSystems::SetUp();
        addCollisionSystem();
        addHealthSystem();
    }
};

TEST_F(Healths, BydosKilledByADeadPlayersMissileScoresForThatPlayer)
{
    (void)spawnAliveShip();
    (void)spawnAliveShip();
    const Entity ship = spawnAliveShip();
    const NetId shipNetId = netIdOf(ship);
    const Vec2f position{900.0F, 500.0F};

    component<Health>(ship).current = 0;
    tick();
    ASSERT_FALSE(_registry.isAlive(ship));
    ASSERT_EQ(roster().get(2)->state, SlotState::RESPAWNING);
    (void)eventsOf<PlayerKilled>();

    const Entity bydos = spawnBydos(position);
    const Entity missile = spawnPlayerMissile(shipNetId, 2, position);
    const EnemyKilled expected{.killerShip = netIdOf(missile),
        .enemy = netIdOf(bydos), .position = position};

    tick();

    const auto killed = eventsOf<EnemyKilled>();

    EXPECT_FALSE(_registry.isAlive(bydos));
    EXPECT_FALSE(_registry.isAlive(missile));
    EXPECT_EQ(roster().get(2)->score, 100U);
    EXPECT_EQ(roster().get(0)->score, 0U);
    EXPECT_EQ(roster().get(1)->score, 0U);
    ASSERT_EQ(killed.size(), 1U);
    EXPECT_EQ(killed[0], expected);
}

TEST_F(Healths, ShipKilledWithThreeLivesRespawns)
{
    const Entity ship = spawnAliveShip(3);
    const Entity bydos = spawnBydos({640.0F, 360.0F});

    moveTo(ship, {640.0F, 360.0F});

    const PlayerKilled expected{.killer = netIdOf(bydos),
        .ship = netIdOf(ship), .playerId = 0, .position = {640.0F, 360.0F}};

    tick();

    const auto *slot = roster().get(0);
    const auto killed = eventsOf<PlayerKilled>();

    EXPECT_FALSE(_registry.isAlive(ship));
    ASSERT_NE(slot, nullptr);
    EXPECT_EQ(slot->state, SlotState::RESPAWNING);
    EXPECT_EQ(slot->lives, 2);
    EXPECT_FLOAT_EQ(slot->respawnTimer, 2.0F);
    EXPECT_EQ(slot->shipNetId, NO_NET_ID);
    ASSERT_EQ(killed.size(), 1U);
    EXPECT_EQ(killed[0], expected);
}

TEST_F(Healths, ShipKilledOnItsLastLifeIsOut)
{
    const Entity ship = spawnAliveShip(1);

    component<Health>(ship).current = 0;
    tick();

    const auto *slot = roster().get(0);
    const auto killed = eventsOf<PlayerKilled>();

    EXPECT_FALSE(_registry.isAlive(ship));
    ASSERT_NE(slot, nullptr);
    EXPECT_EQ(slot->state, SlotState::OUT);
    EXPECT_EQ(slot->lives, 0);
    ASSERT_EQ(killed.size(), 1U);
    EXPECT_EQ(killed[0].killer, NO_NET_ID);
    EXPECT_EQ(killed[0].playerId, 0);
}

TEST_F(Healths, MissileAtZeroHealthDiesSilently)
{
    const Entity missile = spawnPlayerMissile(NO_NET_ID, 0, {640.0F, 360.0F});

    component<Health>(missile).current = 0;
    tick();
    EXPECT_FALSE(_registry.isAlive(missile));
    EXPECT_TRUE(eventsOf<EnemyKilled>().empty());
}

TEST_F(Healths, EntityAboveZeroHealthSurvives)
{
    const Entity bydos = spawnBydos({800.0F, 360.0F});

    tick();
    EXPECT_TRUE(_registry.isAlive(bydos));
    EXPECT_FALSE(has<LastHit>(bydos));
}
