/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** MotionTests
*/

#include <gtest/gtest.h>

#include <numbers>

#include "engine/core/Vec2.hpp"
#include "rtype/game/Motion.hpp"
#include "rtype/game/Rules.hpp"

using engine::core::Vec2f;
using rtype::game::confineToPlayfield;
using rtype::game::InputButtons;
using rtype::game::inputDirection;
using rtype::game::movePlayer;
using rtype::game::Rules;

namespace {

void expectVec(const Vec2f actual, const Vec2f expected)
{
    EXPECT_FLOAT_EQ(actual.x(), expected.x());
    EXPECT_FLOAT_EQ(actual.y(), expected.y());
}

}

TEST(InputDirection, OppositeButtonsAndNothingGiveZero)
{
    expectVec(inputDirection(InputButtons{InputButtons::UP
                  | InputButtons::DOWN}),
        {0.0F, 0.0F});
    expectVec(inputDirection(InputButtons{InputButtons::LEFT
                  | InputButtons::RIGHT}),
        {0.0F, 0.0F});
    expectVec(inputDirection(InputButtons{}), {0.0F, 0.0F});
}

TEST(InputDirection, DiagonalIsNormalised)
{
    const float half = std::numbers::sqrt2_v<float> / 2.0F;

    expectVec(inputDirection(InputButtons{InputButtons::UP
                  | InputButtons::RIGHT}),
        {half, -half});
}

TEST(InputDirection, EachButtonPointsItsWay)
{
    expectVec(inputDirection(InputButtons{InputButtons::UP}), {0.0F, -1.0F});
    expectVec(inputDirection(InputButtons{InputButtons::DOWN}), {0.0F, 1.0F});
    expectVec(inputDirection(InputButtons{InputButtons::LEFT}), {-1.0F, 0.0F});
    expectVec(inputDirection(InputButtons{InputButtons::RIGHT}), {1.0F, 0.0F});
    expectVec(inputDirection(InputButtons{InputButtons::FIRE}), {0.0F, 0.0F});
}

TEST(ConfineToPlayfield, KeepsTheWholeBoxInside)
{
    const Rules rules;
    const Vec2f half{16.0F, 7.0F};

    expectVec(confineToPlayfield({640.0F, 360.0F}, half, rules),
        {640.0F, 360.0F});
    expectVec(confineToPlayfield({-50.0F, -50.0F}, half, rules),
        {16.0F, 7.0F});
    expectVec(confineToPlayfield({2000.0F, 2000.0F}, half, rules),
        {1264.0F, 713.0F});
}

TEST(MovePlayer, MovesRightAtThePlayerSpeed)
{
    const Rules rules;

    expectVec(movePlayer({120.0F, 144.0F}, InputButtons{InputButtons::RIGHT},
                  1.0F, rules),
        {440.0F, 144.0F});
}

TEST(MovePlayer, StopsAtTheLeftEdge)
{
    const Rules rules;

    expectVec(movePlayer({120.0F, 144.0F}, InputButtons{InputButtons::LEFT},
                  10.0F, rules),
        {16.0F, 144.0F});
}

TEST(MovePlayer, StopsAtTheBottomEdge)
{
    const Rules rules;

    expectVec(movePlayer({120.0F, 144.0F}, InputButtons{InputButtons::DOWN},
                  10.0F, rules),
        {120.0F, 713.0F});
}
