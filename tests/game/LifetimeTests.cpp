/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** LifetimeTests
*/

#include <gtest/gtest.h>

#include "GameSystemsFixture.hpp"

using engine::ecs::Entity;
using game_tests::DT;
using rtype::game::Lifetime;
using rtype::game::NO_NET_ID;

class Lifetimes: public game_tests::GameSystems {
protected:
    void SetUp() override
    {
        GameSystems::SetUp();
        addLifetimeSystem();
    }
};

TEST_F(Lifetimes, TwoSecondMissileDiesOnTick120)
{
    const Entity missile = spawnPlayerMissile(NO_NET_ID, 0, {640.0F, 360.0F});

    tick(119);
    EXPECT_TRUE(_registry.isAlive(missile));
    tick();
    EXPECT_FALSE(_registry.isAlive(missile));
}

TEST_F(Lifetimes, FourSecondEnemyMissileDiesOnTick240)
{
    const Entity missile = spawnEnemyMissile(NO_NET_ID, {640.0F, 360.0F});

    tick(239);
    EXPECT_TRUE(_registry.isAlive(missile));
    tick();
    EXPECT_FALSE(_registry.isAlive(missile));
}

TEST_F(Lifetimes, RemainingCountsDownByDt)
{
    const Entity missile = spawnPlayerMissile(NO_NET_ID, 0, {640.0F, 360.0F});

    tick();
    EXPECT_FLOAT_EQ(component<Lifetime>(missile).remaining, 2.0F - DT);
}
