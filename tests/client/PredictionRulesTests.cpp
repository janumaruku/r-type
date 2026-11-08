/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** PredictionRulesTests
*/

#include <gtest/gtest.h>

#include "PredictionState.hpp"
#include "rtype/protocol/Messages.hpp"

using engine::core::Vec2f;
using rtype::client::PredictionState;
using rtype::client::predictionRules;
using rtype::game::Rules;
using rtype::protocol::Welcome;
namespace buttons = rtype::protocol::buttons;

namespace {
const Welcome FAST_WIDE{.playerSpeed = 600.0F,
    .playerHalfWidth                 = 20.0F,
    .playerHalfHeight                = 10.0F};
}

TEST(PredictionRulesTests, TheMovementRulesComeFromTheWelcome)
{
    const Rules rules = predictionRules(FAST_WIDE);

    EXPECT_FLOAT_EQ(rules.playerSpeed, 600.0F);
    EXPECT_EQ(rules.playerHalfSize, Vec2f(20.0F, 10.0F));
}

TEST(PredictionRulesTests, TheOtherRulesKeepTheirDefaults)
{
    const Rules rules = predictionRules(FAST_WIDE);

    EXPECT_EQ(rules.playfield, Rules{}.playfield);
    EXPECT_EQ(rules.playerLives, Rules{}.playerLives);
    EXPECT_EQ(rules.enemySpeed, Rules{}.enemySpeed);
}

TEST(PredictionRulesTests, AStepMovesAtTheServersSpeed)
{
    const PredictionState prediction;
    const Vec2f moved = prediction.step({100.0F, 100.0F}, buttons::RIGHT,
        predictionRules(FAST_WIDE));

    EXPECT_FLOAT_EQ(moved.x(), 110.0F);
    EXPECT_FLOAT_EQ(moved.y(), 100.0F);
}

/* The fixed 1280-wide playfield, minus the server's half width of 20. */
TEST(PredictionRulesTests, AStepStopsTheServersHitboxAtTheEdge)
{
    const PredictionState prediction;
    const Vec2f moved = prediction.step({1255.0F, 360.0F}, buttons::RIGHT,
        predictionRules(FAST_WIDE));

    EXPECT_FLOAT_EQ(moved.x(), 1260.0F);
}
