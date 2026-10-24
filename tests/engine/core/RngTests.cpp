/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RngTests.cpp
*/

#include <gtest/gtest.h>

#include "engine/core/Rng.hpp"

TEST(Rng, SameSeedProducesSameSequence)
{
    engine::core::Rng first(123456U);
    engine::core::Rng second(123456U);

    for (int index = 0; index < 1000; ++index)
        EXPECT_EQ(first.uniformInt(-1000000, 1000000),
            second.uniformInt(-1000000, 1000000));
}

TEST(Rng, DifferentSeedsProduceDifferentSequences)
{
    engine::core::Rng first(123456U);
    engine::core::Rng second(654321U);
    bool sequenceDiffers = false;

    for (int index = 0; index < 1000; ++index)
        sequenceDiffers = sequenceDiffers
            || first.uniformInt(-1000000, 1000000)
                != second.uniformInt(-1000000, 1000000);

    EXPECT_TRUE(sequenceDiffers);
}

TEST(Rng, UniformIntRespectsBoundsAndHitsEndpoints)
{
    engine::core::Rng rng(123456U);
    bool hitLowerBound = false;
    bool hitUpperBound = false;

    for (int index = 0; index < 10000; ++index) {
        const int value = rng.uniformInt(-3, 3);
        EXPECT_GE(value, -3);
        EXPECT_LE(value, 3);
        hitLowerBound = hitLowerBound || value == -3;
        hitUpperBound = hitUpperBound || value == 3;
    }

    EXPECT_TRUE(hitLowerBound);
    EXPECT_TRUE(hitUpperBound);
}

TEST(Rng, UniformFloatNeverReturnsOne)
{
    engine::core::Rng rng(123456U);

    for (int index = 0; index < 10000; ++index) {
        const float value = rng.uniformFloat(0.0F, 1.0F);
        EXPECT_GE(value, 0.0F);
        EXPECT_LT(value, 1.0F);
    }
}

TEST(Rng, ChanceRespectsExtremeProbabilities)
{
    engine::core::Rng rng(123456U);

    for (int index = 0; index < 10000; ++index) {
        EXPECT_FALSE(rng.chance(0.0F));
        EXPECT_TRUE(rng.chance(1.0F));
    }
}