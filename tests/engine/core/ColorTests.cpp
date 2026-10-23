/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ColorTests.cpp
*/


#include <gtest/gtest.h>
#include "engine/core/Color.hpp"

TEST(Color, constuct)
{
    engine::core::Color c;
    EXPECT_EQ(c.a, 255);
}

TEST(Color, constuct_with_values)
{
    engine::core::Color c(255, 0, 0, 255);
    EXPECT_EQ(c.r, 255);
    EXPECT_EQ(c.g, 0);
    EXPECT_EQ(c.b, 0);
    EXPECT_EQ(c.a, 255);
}

TEST(Color, withAlpha)
{
    engine::core::Color c(255, 0, 0, 255);
    engine::core::Color c2 = c.withAlpha(128);
    EXPECT_EQ(c2.r, 255);
    EXPECT_EQ(c2.g, 0);
    EXPECT_EQ(c2.b, 0);
    EXPECT_EQ(c2.a, 128);
}

TEST(Color, static_colors)
{
    EXPECT_EQ(engine::core::Color::WHITE.r, 255);
    EXPECT_EQ(engine::core::Color::WHITE.g, 255);
    EXPECT_EQ(engine::core::Color::WHITE.b, 255);
    EXPECT_EQ(engine::core::Color::WHITE.a, 255);

    EXPECT_EQ(engine::core::Color::BLACK.r, 0);
    EXPECT_EQ(engine::core::Color::BLACK.g, 0);
    EXPECT_EQ(engine::core::Color::BLACK.b, 0);
    EXPECT_EQ(engine::core::Color::BLACK.a, 255);

    EXPECT_EQ(engine::core::Color::RED.r, 255);
    EXPECT_EQ(engine::core::Color::RED.g, 0);
    EXPECT_EQ(engine::core::Color::RED.b, 0);
    EXPECT_EQ(engine::core::Color::RED.a, 255);

    EXPECT_EQ(engine::core::Color::GREEN.r, 0);
    EXPECT_EQ(engine::core::Color::GREEN.g, 255);
    EXPECT_EQ(engine::core::Color::GREEN.b, 0);
    EXPECT_EQ(engine::core::Color::GREEN.a, 255);

    EXPECT_EQ(engine::core::Color::BLUE.r, 0);
    EXPECT_EQ(engine::core::Color::BLUE.g, 0);
    EXPECT_EQ(engine::core::Color::BLUE.b, 255);
    EXPECT_EQ(engine::core::Color::BLUE.a, 255);

    EXPECT_EQ(engine::core::Color::CLEAR.r, 0);
    EXPECT_EQ(engine::core::Color::CLEAR.g, 0);
    EXPECT_EQ(engine::core::Color::CLEAR.b, 0);
    EXPECT_EQ(engine::core::Color::CLEAR.a, 0);
}

