/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GamePhaseTests
*/

#include <gtest/gtest.h>

#include "rtype/game/GamePhase.hpp"

using rtype::game::GamePhase;
using rtype::game::Phase;

TEST(GamePhase, StartsPlaying)
{
    GamePhase phase;

    EXPECT_EQ(phase.phase(), Phase::PLAYING);
    EXPECT_FALSE(phase.isOver());
    EXPECT_FALSE(phase.update(1.0F));
    EXPECT_EQ(phase.phase(), Phase::PLAYING);
}

TEST(GamePhase, EntersGameOverOnceAndRestartsOnceAfterTheDelay)
{
    GamePhase phase;
    int restarts = 0;
    int restartCall = 0;

    EXPECT_TRUE(phase.enterGameOver(5.0F));
    EXPECT_FALSE(phase.enterGameOver(5.0F));
    EXPECT_TRUE(phase.isOver());
    for (int call = 1; call <= 300; ++call) {
        if (phase.update(1.0F / 60.0F)) {
            ++restarts;
            restartCall = call;
        }
    }
    EXPECT_EQ(restarts, 1);
    EXPECT_EQ(restartCall, 300);
    EXPECT_EQ(phase.phase(), Phase::PLAYING);
    EXPECT_FALSE(phase.update(1.0F / 60.0F));
}
