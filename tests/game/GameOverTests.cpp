/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GameOverTests
*/

#include <gtest/gtest.h>

#include <vector>

#include "GameSystemsFixture.hpp"

using engine::ecs::Entity;
using rtype::game::GameEvent;
using rtype::game::GameOver;
using rtype::game::GameRestarted;
using rtype::game::NO_NET_ID;
using rtype::game::Phase;
using rtype::game::PlayerShip;
using rtype::game::SlotState;

struct Leftovers {
    Entity bydos;
    Entity missile;
};

class GameOverFlow: public game_tests::GameSystems {
protected:
    void SetUp() override
    {
        GameSystems::SetUp();
        addSpawnDirectorSystem();
        addRespawnSystem();
        addGameOverSystem();
    }

    /*
    ** Two players on their last life, with 100 and 250 points, both just
    ** killed, and a Bydos and an enemy missile left on screen.
    */
    Leftovers loseTheGame()
    {
        const Entity first = spawnAliveShip(1);
        const Entity second = spawnAliveShip(1);

        EXPECT_TRUE(roster().addScore(0, 100));
        EXPECT_TRUE(roster().addScore(1, 250));
        EXPECT_TRUE(roster().onShipKilled(0, 2.0F));
        EXPECT_TRUE(roster().onShipKilled(1, 2.0F));
        _registry.killEntity(first);
        _registry.killEntity(second);

        const Entity bydos = spawnBydos({900.0F, 300.0F});

        return {.bydos = bydos,
            .missile = spawnEnemyMissile(netIdOf(bydos), {700.0F, 300.0F})};
    }

    void expectRestartedSlot(const std::uint8_t playerId)
    {
        EXPECT_EQ(slot(playerId).state, SlotState::RESPAWNING);
        EXPECT_EQ(slot(playerId).lives, 3);
        EXPECT_EQ(slot(playerId).score, 0U);
    }
};

TEST_F(GameOverFlow, AllSlotsOutEndsTheGameThenRestartsItAfterFiveSeconds)
{
    const Leftovers leftovers = loseTheGame();

    tick();

    const auto over = drainEvents();

    ASSERT_EQ(countOf<GameOver>(over), 1U);
    EXPECT_EQ(std::get<GameOver>(over[0]).teamScore, 350U);
    EXPECT_EQ(phase().phase(), Phase::GAME_OVER);

    std::vector<GameEvent> waiting;

    for (int i = 0; i < 299; ++i) {
        tick();
        for (auto &event: drainEvents())
            waiting.push_back(std::move(event));
    }
    EXPECT_TRUE(waiting.empty());
    EXPECT_TRUE(_registry.isAlive(leftovers.bydos));
    EXPECT_EQ(slot(0).state, SlotState::OUT);

    director().elapsed = 42.0F;
    tick();

    const auto restart = drainEvents();

    EXPECT_EQ(countOf<GameRestarted>(restart), 1U);
    EXPECT_EQ(countOf<GameOver>(restart), 0U);
    EXPECT_EQ(phase().phase(), Phase::PLAYING);
    EXPECT_FALSE(_registry.isAlive(leftovers.bydos));
    EXPECT_FALSE(_registry.isAlive(leftovers.missile));
    EXPECT_FLOAT_EQ(director().elapsed, 0.0F);
    expectRestartedSlot(0);
    expectRestartedSlot(1);
    EXPECT_EQ(countOf<PlayerShip>(), 0U);

    tick();
    EXPECT_EQ(slot(0).state, SlotState::ALIVE);
    EXPECT_EQ(slot(1).state, SlotState::ALIVE);
    EXPECT_NE(slot(0).shipNetId, NO_NET_ID);
    EXPECT_NE(slot(1).shipNetId, NO_NET_ID);
    EXPECT_EQ(countOf<PlayerShip>(), 2U);
}

TEST_F(GameOverFlow, NoPlayerAtAllNeverEndsTheGameNorSpawns)
{
    tick(6000);
    EXPECT_EQ(countOf<GameOver>(drainEvents()), 0U);
    EXPECT_EQ(phase().phase(), Phase::PLAYING);
    EXPECT_TRUE(bydos().empty());
}

TEST_F(GameOverFlow, OnePlayerStillPlayingKeepsTheGameGoing)
{
    (void)spawnAliveShip();
    (void)spawnAliveShip(1);
    ASSERT_TRUE(roster().onShipKilled(1, 2.0F));
    tick();
    EXPECT_EQ(countOf<GameOver>(drainEvents()), 0U);
    EXPECT_EQ(phase().phase(), Phase::PLAYING);
}
