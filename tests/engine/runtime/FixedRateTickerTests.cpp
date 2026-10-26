/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** FixedRateTickerTests
*/

#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "engine/core/Errors.hpp"
#include "engine/runtime/Clock.hpp"
#include "engine/runtime/FixedRateTicker.hpp"

using engine::runtime::Clock;
using engine::runtime::FixedRateTicker;
using namespace std::chrono_literals;

/*
** Timing tests only check lower bounds and counts. The one upper bound,
** in WorkBetweenTicksDoesNotSlowTheRate, is kept loose on purpose.
*/
namespace {
constexpr std::chrono::milliseconds PERIOD{10};
}

TEST(FixedRateTickerTests, StartsAtTickZero)
{
    const FixedRateTicker ticker(PERIOD);

    EXPECT_EQ(ticker.tick(), 0U);
    EXPECT_EQ(ticker.overruns(), 0U);
}

TEST(FixedRateTickerTests, TwentyTicksTakeAtLeastNineteenPeriods)
{
    const auto start = Clock::now();
    FixedRateTicker ticker(PERIOD);

    for (int i = 0; i < 20; ++i)
        ticker.waitNext();
    EXPECT_EQ(ticker.tick(), 20U);
    EXPECT_GE(Clock::now() - start, 19 * PERIOD);
    EXPECT_EQ(ticker.overruns(), 0U);
}

TEST(FixedRateTickerTests, WorkBetweenTicksDoesNotSlowTheRate)
{
    const auto start = Clock::now();
    FixedRateTicker ticker(PERIOD);

    for (int i = 0; i < 20; ++i) {
        std::this_thread::sleep_for(5ms);
        ticker.waitNext();
    }
    EXPECT_LT(Clock::now() - start, 20 * 15ms);
}

TEST(FixedRateTickerTests, ABlockedCallerOverrunsAndReturnsAtOnce)
{
    FixedRateTicker ticker(PERIOD);

    std::this_thread::sleep_for(300ms);
    const auto before = Clock::now();
    ticker.waitNext();

    EXPECT_LT(Clock::now() - before, 100ms);
    EXPECT_EQ(ticker.overruns(), 1U);
    EXPECT_EQ(ticker.tick(), 1U);
}

TEST(FixedRateTickerTests, TheTicksAfterAnOverrunComeWithoutABurst)
{
    FixedRateTicker ticker(PERIOD);

    std::this_thread::sleep_for(300ms);
    ticker.waitNext();
    const auto start = Clock::now();
    for (int i = 0; i < 5; ++i)
        ticker.waitNext();

    EXPECT_GE(Clock::now() - start, 4 * PERIOD);
    EXPECT_EQ(ticker.overruns(), 1U);
}

TEST(FixedRateTickerTests, ANonPositivePeriodIsRejected)
{
    EXPECT_THROW(FixedRateTicker(0ns), engine::core::EngineError);
    EXPECT_THROW(FixedRateTicker(-PERIOD), engine::core::EngineError);
}
