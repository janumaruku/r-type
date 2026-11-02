/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SpawnDirectorTests
*/

#include <gtest/gtest.h>

#include <array>
#include <utility>

#include "GameSystemsFixture.hpp"

using engine::ecs::Entity;
using game_tests::DT;
using rtype::game::MovePattern;
using rtype::game::Replicated;
using rtype::game::Transform;

class SpawnDirector: public game_tests::GameSystems {
protected:
    void SetUp() override
    {
        GameSystems::SetUp();
        addSpawnDirectorSystem();
    }
};

TEST_F(SpawnDirector, NoPlayerAliveForTenSecondsSpawnsNothing)
{
    tick(600);
    EXPECT_TRUE(bydos().empty());
    EXPECT_FLOAT_EQ(director().elapsed, 0.0F);
}

TEST_F(SpawnDirector, OnePlayerAliveForOnePointSixSecondsSpawnsOneBydos)
{
    (void)spawnAliveShip();
    tick(96);

    const auto spawned = bydos();

    ASSERT_EQ(spawned.size(), 1U);

    const auto position = component<Transform>(spawned[0]).position;

    EXPECT_FLOAT_EQ(position.x(), 1312.0F);
    EXPECT_GE(position.y(), 40.0F);
    EXPECT_LT(position.y(), 680.0F);
}

TEST_F(SpawnDirector, IntervalShrinksWithElapsedTime)
{
    constexpr std::array<std::pair<float, float>, 4> expected{{
        {0.0F, 1.6F}, {10.0F, 1.55F}, {100.0F, 1.1F}, {300.0F, 0.5F},
    }};

    (void)spawnAliveShip();
    for (const auto &[elapsed, interval]: expected) {
        SCOPED_TRACE(elapsed);
        director().elapsed = elapsed;
        director().untilNextSpawn = 0.0F;
        tick();
        EXPECT_NEAR(director().untilNextSpawn, interval, 1e-5F);
    }
}

TEST_F(SpawnDirector, ThirtyBydosAliveBlockTheThirtyFirst)
{
    (void)spawnAliveShip();
    for (int i = 0; i < 30; ++i)
        (void)spawnBydos({600.0F, 40.0F + 20.0F * static_cast<float>(i)});
    tick();
    EXPECT_EQ(bydos().size(), 30U);
    EXPECT_NEAR(director().untilNextSpawn, 1.6F, 1e-5F);
}

TEST_F(SpawnDirector, AboutHalfOfAThousandSpawnsAreSine)
{
    int sine = 0;

    (void)spawnAliveShip();
    for (int i = 0; i < 1000; ++i) {
        director().untilNextSpawn = 0.0F;
        tick();
        for (const Entity entity: bydos()) {
            if (component<Replicated>(entity).variant
                == static_cast<std::uint8_t>(MovePattern::SINE))
                ++sine;
            _registry.killEntity(entity);
        }
    }
    EXPECT_GE(sine, 450);
    EXPECT_LE(sine, 550);
}

TEST_F(SpawnDirector, ClockPausesWhileNoShipIsAlive)
{
    (void)spawnAliveShip();
    tick(60);
    ASSERT_NEAR(director().elapsed, 1.0F, 1e-4F);

    const float untilNextSpawn = director().untilNextSpawn;

    ASSERT_TRUE(roster().onShipKilled(0, 2.0F));
    tick(60);
    EXPECT_NEAR(director().elapsed, 1.0F, 1e-4F);
    EXPECT_FLOAT_EQ(director().untilNextSpawn, untilNextSpawn);
    EXPECT_EQ(bydos().size(), 1U);
}

TEST_F(SpawnDirector, NothingSpawnsDuringAGameOver)
{
    (void)spawnAliveShip();
    ASSERT_TRUE(phase().enterGameOver(5.0F));
    tick(120);
    EXPECT_TRUE(bydos().empty());
    EXPECT_FLOAT_EQ(director().elapsed, 0.0F);
}

TEST_F(SpawnDirector, FirstBydosAppearsOnTheFirstTickWithAShipAlive)
{
    (void)spawnAliveShip();
    tick();
    EXPECT_EQ(bydos().size(), 1U);
    EXPECT_NEAR(director().elapsed, DT, 1e-7F);
}
