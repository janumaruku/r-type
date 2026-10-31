/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** CollisionTests
*/

#include <gtest/gtest.h>

#include "GameSystemsFixture.hpp"

using engine::ecs::Entity;
using rtype::game::Health;
using rtype::game::LastHit;
using rtype::game::NO_NET_ID;
using rtype::game::NO_PLAYER;
using rtype::game::PlayerShip;

class Collisions: public game_tests::GameSystems {
protected:
    void SetUp() override
    {
        GameSystems::SetUp();
        addCollisionSystem();
    }

    [[nodiscard]] int healthOf(const Entity entity)
    {
        return component<Health>(entity).current;
    }
};

TEST_F(Collisions, ShipAndBydosOverlappingBothTakeDamage)
{
    const Entity ship = spawnAliveShip();
    const Entity bydos = spawnBydos({650.0F, 360.0F});

    moveTo(ship, {640.0F, 360.0F});
    tick();
    EXPECT_EQ(healthOf(ship), 0);
    EXPECT_EQ(healthOf(bydos), 0);
    EXPECT_EQ(component<LastHit>(ship).by, netIdOf(bydos));
    EXPECT_EQ(component<LastHit>(ship).byPlayerId, NO_PLAYER);
    EXPECT_EQ(component<LastHit>(bydos).by, netIdOf(ship));
    EXPECT_EQ(component<LastHit>(bydos).byPlayerId, 0);
}

TEST_F(Collisions, InvulnerableShipAndBydosAreNotDamaged)
{
    const Entity ship = spawnAliveShip();
    const Entity bydos = spawnBydos({650.0F, 360.0F});

    moveTo(ship, {640.0F, 360.0F});
    component<PlayerShip>(ship).invulnerableFor = 1.0F;
    tick();
    EXPECT_EQ(healthOf(ship), 1);
    EXPECT_EQ(healthOf(bydos), 1);
    EXPECT_FALSE(has<LastHit>(ship));
    EXPECT_FALSE(has<LastHit>(bydos));
}

TEST_F(Collisions, PlayerMissileDamagesBydosAndIsSpent)
{
    const Entity ship = spawnAliveShip();
    const Entity bydos = spawnBydos({800.0F, 360.0F});
    const Entity missile = spawnPlayerMissile(netIdOf(ship), 0,
        {800.0F, 360.0F});

    tick();
    EXPECT_EQ(healthOf(bydos), 0);
    EXPECT_EQ(healthOf(missile), 0);
    EXPECT_EQ(component<LastHit>(bydos).by, netIdOf(missile));
    EXPECT_EQ(component<LastHit>(bydos).byPlayerId, 0);
}

TEST_F(Collisions, EnemyMissileDamagesShipAndIsSpent)
{
    const Entity ship = spawnAliveShip();
    const Entity bydos = spawnBydos({1000.0F, 100.0F});
    const Entity missile = spawnEnemyMissile(netIdOf(bydos),
        {640.0F, 360.0F});

    moveTo(ship, {640.0F, 360.0F});
    tick();
    EXPECT_EQ(healthOf(ship), 0);
    EXPECT_EQ(healthOf(missile), 0);
    EXPECT_EQ(healthOf(bydos), 1);
    EXPECT_EQ(component<LastHit>(ship).by, netIdOf(missile));
    EXPECT_EQ(component<LastHit>(ship).byPlayerId, NO_PLAYER);
}

TEST_F(Collisions, SameSideAndFriendlyFireDealNoDamage)
{
    const Entity first = spawnAliveShip();
    const Entity second = spawnAliveShip();
    const Entity playerMissile = spawnPlayerMissile(netIdOf(first), 0,
        {300.0F, 200.0F});
    const Entity bydosA = spawnBydos({900.0F, 500.0F});
    const Entity bydosB = spawnBydos({905.0F, 500.0F});
    const Entity enemyMissile = spawnEnemyMissile(netIdOf(bydosA),
        {900.0F, 500.0F});

    moveTo(first, {300.0F, 200.0F});
    moveTo(second, {305.0F, 200.0F});
    tick();
    for (const Entity entity :
         {first, second, playerMissile, bydosA, bydosB, enemyMissile})
        EXPECT_EQ(healthOf(entity), 1);
    EXPECT_EQ(countOf<LastHit>(), 0U);
}

TEST_F(Collisions, MissileOverlappingTwoBydosHitsOnlyOne)
{
    const Entity bydosA = spawnBydos({800.0F, 360.0F});
    const Entity bydosB = spawnBydos({800.0F, 360.0F});
    const Entity missile = spawnPlayerMissile(NO_NET_ID, 0, {800.0F, 360.0F});

    tick();
    EXPECT_EQ(healthOf(missile), 0);
    EXPECT_EQ(healthOf(bydosA) + healthOf(bydosB), 1);
}
