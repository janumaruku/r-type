/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** FixedTimestepTests
*/

#include <gtest/gtest.h>

#include <random>

#include "engine/core/Errors.hpp"
#include "engine/runtime/FixedTimestep.hpp"

using engine::runtime::FixedTimestep;

/*
** 1/64 is exact in binary, so step counts can be compared exactly.
*/
namespace {
constexpr double STEP = 1.0 / 64;
constexpr int CAP = 5;
}

TEST(FixedTimestepTests, ExactlyOneStepRunsOnceWithNoRemainder)
{
    FixedTimestep timestep(STEP, CAP);

    EXPECT_EQ(timestep.advance(STEP), 1);
    EXPECT_DOUBLE_EQ(timestep.alpha(), 0.0);
}

TEST(FixedTimestepTests, TwoHalfStepsRunOnceOnTheSecondFrame)
{
    FixedTimestep timestep(STEP, CAP);

    EXPECT_EQ(timestep.advance(STEP / 2), 0);
    EXPECT_EQ(timestep.advance(STEP / 2), 1);
}

TEST(FixedTimestepTests, TwoAndAHalfStepsRunTwiceAndKeepHalf)
{
    FixedTimestep timestep(STEP, CAP);

    EXPECT_EQ(timestep.advance(STEP * 2.5), 2);
    EXPECT_DOUBLE_EQ(timestep.alpha(), 0.5);
}

TEST(FixedTimestepTests, ALongFrameHitsTheCapAndDropsTheBacklog)
{
    FixedTimestep timestep(STEP, CAP);

    EXPECT_EQ(timestep.advance(1.0), CAP);
    EXPECT_GE(timestep.alpha(), 0.0);
    EXPECT_LT(timestep.alpha(), 1.0);
    EXPECT_LE(timestep.advance(STEP / 2), 1);
}

TEST(FixedTimestepTests, ANegativeFrameRunsNothingAndKeepsTheAccumulator)
{
    FixedTimestep timestep(STEP, CAP);

    timestep.advance(STEP / 4);
    const double before = timestep.alpha();

    EXPECT_EQ(timestep.advance(-1.0), 0);
    EXPECT_DOUBLE_EQ(timestep.alpha(), before);
}

TEST(FixedTimestepTests, RandomFramesKeepAlphaInRangeAndTheStepCountExact)
{
    FixedTimestep timestep(STEP, 1000);
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> frames(0.0, 0.1);
    double elapsed = 0.0;
    long totalSteps = 0;

    for (int i = 0; i < 1000; ++i) {
        const double frame = frames(rng);

        elapsed += frame;
        totalSteps += timestep.advance(frame);
        ASSERT_GE(timestep.alpha(), 0.0);
        ASSERT_LT(timestep.alpha(), 1.0);
    }
    EXPECT_NEAR(static_cast<double>(totalSteps), elapsed / STEP, 1.0);
}

TEST(FixedTimestepTests, ANonPositiveStepIsRejected)
{
    EXPECT_THROW(FixedTimestep(0.0, CAP), engine::core::EngineError);
    EXPECT_THROW(FixedTimestep(-STEP, CAP), engine::core::EngineError);
}

TEST(FixedTimestepTests, ACapBelowOneIsRejected)
{
    EXPECT_THROW(FixedTimestep(STEP, 0), engine::core::EngineError);
}

TEST(FixedTimestepTests, StepReturnsTheConstructorValue)
{
    EXPECT_DOUBLE_EQ(FixedTimestep(STEP, CAP).step(), STEP);
}
