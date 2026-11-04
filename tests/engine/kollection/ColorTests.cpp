/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ColorTests.cpp
*/

#include <gtest/gtest.h>

#include "kollection/core/Styles.hpp"

using kollection::core::Color;

TEST(KollectionColor, default_is_opaque_black)
{
    EXPECT_EQ(Color{}, Color::BLACK);
    EXPECT_EQ(Color{}.a, 255);
}

TEST(KollectionColor, designated_initializers_keep_the_other_defaults)
{
    const Color red{.r = 255};

    EXPECT_EQ(red, (Color{255, 0, 0, 255}));
}

TEST(KollectionColor, transparent_differs_from_black)
{
    EXPECT_NE(Color::TRANSPARENT, Color::BLACK);
    EXPECT_EQ(Color::TRANSPARENT.a, 0);
}

TEST(KollectionColor, named_constants)
{
    EXPECT_EQ(Color::WHITE, (Color{255, 255, 255, 255}));
    EXPECT_EQ(Color::BLUE, (Color{0, 0, 255, 255}));
}
