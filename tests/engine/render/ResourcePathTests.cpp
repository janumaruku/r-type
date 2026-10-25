/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ResourcePathTests
*/

#include <gtest/gtest.h>

#include <filesystem>

#include "ResourcePath.hpp"

using engine::render::cacheKey;

TEST(ResourcePathTests, ADotSegmentGivesTheSameKey)
{
    EXPECT_EQ(cacheKey("assets/./ship.png"), cacheKey("assets/ship.png"));
}

TEST(ResourcePathTests, AParentSegmentIsFolded)
{
    EXPECT_EQ(cacheKey("assets/fonts/../ship.png"), cacheKey("assets/ship.png"));
}

TEST(ResourcePathTests, RepeatedSeparatorsGiveTheSameKey)
{
    EXPECT_EQ(cacheKey("assets//ship.png"), cacheKey("assets/ship.png"));
}

TEST(ResourcePathTests, DifferentFilesKeepDifferentKeys)
{
    EXPECT_NE(cacheKey("assets/ship.png"), cacheKey("assets/enemy.png"));
}
