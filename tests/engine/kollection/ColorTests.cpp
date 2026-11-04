/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ColorTests.cpp
*/

#include <gtest/gtest.h>

#include "kollection/core/Color.hpp"

using kollection::core::Color;

TEST(KollectionColor, default_is_opaque_black)
{
    EXPECT_EQ(Color{}, Color::Black);
    EXPECT_EQ(Color{}.a, 255);
}

TEST(KollectionColor, designated_initializers_keep_the_other_defaults)
{
    const Color red{.r = 255};

    EXPECT_EQ(red, (Color{255, 0, 0, 255}));
}

TEST(KollectionColor, transparent_differs_from_black)
{
    EXPECT_NE(Color::Transparent, Color::Black);
    EXPECT_EQ(Color::Transparent.a, 0);
}

TEST(KollectionColor, named_constants)
{
    EXPECT_EQ(Color::White, (Color{255, 255, 255, 255}));
    EXPECT_EQ(Color::Blue, (Color{0, 0, 255, 255}));
}
