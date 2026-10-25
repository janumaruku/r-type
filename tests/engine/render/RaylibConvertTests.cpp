/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RaylibConvertTests
*/

#include <gtest/gtest.h>

#include "RaylibConvert.hpp"

using engine::core::FloatRect;
using engine::core::IntRect;
using engine::core::Vec2f;
using engine::core::Vec2i;
using engine::render::detail::toRaylib;

TEST(RaylibConvertTests, AColorKeepsEveryChannel)
{
    const ::Color color = toRaylib(engine::core::Color{10, 20, 30, 40});

    EXPECT_EQ(color.r, 10);
    EXPECT_EQ(color.g, 20);
    EXPECT_EQ(color.b, 30);
    EXPECT_EQ(color.a, 40);
}

TEST(RaylibConvertTests, AVectorKeepsBothCoordinates)
{
    const Vector2 vector = toRaylib(Vec2f{-1.5F, 2.25F});

    EXPECT_FLOAT_EQ(vector.x, -1.5F);
    EXPECT_FLOAT_EQ(vector.y, 2.25F);
}

TEST(RaylibConvertTests, AFloatRectKeepsOriginAndSize)
{
    const Rectangle rect = toRaylib(FloatRect{Vec2f{1.0F, 2.0F}, 3.5F, 4.5F});

    EXPECT_FLOAT_EQ(rect.x, 1.0F);
    EXPECT_FLOAT_EQ(rect.y, 2.0F);
    EXPECT_FLOAT_EQ(rect.width, 3.5F);
    EXPECT_FLOAT_EQ(rect.height, 4.5F);
}

TEST(RaylibConvertTests, AnIntRectKeepsANegativeWidthForFlipping)
{
    const Rectangle rect = toRaylib(IntRect{Vec2i{32, 0}, -32, 16});

    EXPECT_FLOAT_EQ(rect.x, 32.0F);
    EXPECT_FLOAT_EQ(rect.y, 0.0F);
    EXPECT_FLOAT_EQ(rect.width, -32.0F);
    EXPECT_FLOAT_EQ(rect.height, 16.0F);
}
