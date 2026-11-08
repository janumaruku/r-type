/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RateLimitsTests.cpp
*/

#include "RateLimits.hpp"

#include <asio.hpp>
#include <chrono>
#include <cstdint>
#include <gtest/gtest.h>

namespace rtype::server::tests {

using namespace std::chrono_literals;
using Clock     = std::chrono::steady_clock;
using TimePoint = Clock::time_point;

namespace {

constexpr std::uint32_t WINDOW_LIMIT = 10;
const TimePoint T0{};

asio::ip::udp::endpoint endpointFor(std::uint32_t index)
{
    const auto address = asio::ip::address_v4(0x0a000001U + index);
    return {address, static_cast<unsigned short>(10000U + (index % 50000U))};
}

std::uint32_t sendDatagrams(FloodTracker &tracker, TimePoint now,
    std::uint32_t amount)
{
    std::uint32_t allowed = 0;

    for (std::uint32_t i = 0; i < amount; ++i) {
        if (tracker.allow(now))
            ++allowed;
    }
    return allowed;
}

} // namespace

TEST(WindowCounterTests, AcceptsExactlyTheLimitAndResetsForANewWindow)
{
    WindowCounter counter{WINDOW_LIMIT, 1s};

    for (std::uint32_t i = 0; i < WINDOW_LIMIT; ++i)
        EXPECT_TRUE(counter.record(T0)) << "record " << (i + 1);

    EXPECT_EQ(counter.countInWindow(), WINDOW_LIMIT);
    EXPECT_FALSE(counter.record(T0));

    EXPECT_TRUE(counter.record(T0 + 1001ms));
    EXPECT_EQ(counter.countInWindow(), 1U);
}

TEST(FloodTrackerTests,
    Allows240PerSecondAndDoesNotKickAfterTwoFloodedSecondsThenCalm)
{
    FloodTracker tracker;

    EXPECT_EQ(sendDatagrams(tracker, T0, 300), 240U);
    EXPECT_FALSE(tracker.shouldKick(T0));

    EXPECT_EQ(sendDatagrams(tracker, T0 + 1s, 300), 240U);
    EXPECT_FALSE(tracker.shouldKick(T0 + 2s));

    // A calm second.
    EXPECT_EQ(sendDatagrams(tracker, T0 + 2s, 100), 100U);
    EXPECT_FALSE(tracker.shouldKick(T0 + 3s));
}

TEST(FloodTrackerTests, KicksAfterThreeConsecutiveFloodedSeconds)
{
    FloodTracker tracker;

    EXPECT_EQ(sendDatagrams(tracker, T0, 300), 240U);
    EXPECT_EQ(sendDatagrams(tracker, T0 + 1s, 300), 240U);
    EXPECT_FALSE(tracker.shouldKick(T0 + 2s));

    EXPECT_EQ(sendDatagrams(tracker, T0 + 2s, 300), 240U);
    EXPECT_TRUE(tracker.shouldKick(T0 + 3s));
}

TEST(FloodTrackerTests, QuietSecondsBreakTheStreak)
{
    FloodTracker tracker;

    EXPECT_EQ(sendDatagrams(tracker, T0, 300), 240U);
    EXPECT_EQ(sendDatagrams(tracker, T0 + 1s, 300), 240U);

    EXPECT_EQ(sendDatagrams(tracker, T0 + 12s, 300), 240U);
    EXPECT_FALSE(tracker.shouldKick(T0 + 13s))
        << "10 silent seconds sit between the 2nd and the 3rd flood";

    EXPECT_EQ(sendDatagrams(tracker, T0 + 13s, 300), 240U);
    EXPECT_EQ(sendDatagrams(tracker, T0 + 14s, 300), 240U);
    EXPECT_TRUE(tracker.shouldKick(T0 + 15s));
}

TEST(FloodTrackerTests, ACalmSecondBreaksTheStreak)
{
    FloodTracker tracker;

    EXPECT_EQ(sendDatagrams(tracker, T0, 300), 240U);
    EXPECT_EQ(sendDatagrams(tracker, T0 + 1s, 300), 240U);
    EXPECT_EQ(sendDatagrams(tracker, T0 + 2s, 100), 100U);
    EXPECT_EQ(sendDatagrams(tracker, T0 + 3s, 300), 240U);
    EXPECT_EQ(sendDatagrams(tracker, T0 + 4s, 300), 240U);
    EXPECT_FALSE(tracker.shouldKick(T0 + 5s));

    EXPECT_EQ(sendDatagrams(tracker, T0 + 5s, 300), 240U);
    EXPECT_TRUE(tracker.shouldKick(T0 + 6s));
}

TEST(FloodTrackerTests, AnOldStreakNoLongerKicks)
{
    FloodTracker tracker;

    for (const auto second: {0s, 1s, 2s})
        EXPECT_EQ(sendDatagrams(tracker, T0 + second, 300), 240U);
    ASSERT_TRUE(tracker.shouldKick(T0 + 3s));

    EXPECT_FALSE(tracker.shouldKick(T0 + 10s));
}

TEST(RejectLimiterTests, LimitsEachEndpointToOneDatagramPerSecond)
{
    RejectLimiter limiter;
    const auto endpoint = endpointFor(1);

    EXPECT_TRUE(limiter.allow(endpoint, T0));
    EXPECT_FALSE(limiter.allow(endpoint, T0 + 500ms));
    EXPECT_TRUE(limiter.allow(endpoint, T0 + 1010ms));
}

TEST(RejectLimiterTests, AllowsOnlyTwentyDifferentEndpointsPerSecond)
{
    RejectLimiter limiter;
    std::uint32_t allowed = 0;

    for (std::uint32_t i = 0; i < 25; ++i) {
        if (limiter.allow(endpointFor(i), T0))
            ++allowed;
        else
            EXPECT_GE(i, 20U) << "An endpoint among the first 20 was rejected";
    }

    EXPECT_EQ(allowed, 20U);
}

TEST(RejectLimiterTests, HandlesTenThousandUniqueEndpointsOverOneHundredSeconds)
{
    RejectLimiter limiter;
    std::uint32_t serial = 0;

    for (std::uint32_t second = 0; second < 100; ++second) {
        std::uint32_t allowedThisSecond = 0;
        const auto now                  = T0 + std::chrono::seconds(second);

        for (std::uint32_t i = 0; i < 100; ++i, ++serial) {
            if (limiter.allow(endpointFor(serial), now))
                ++allowedThisSecond;
        }

        EXPECT_EQ(allowedThisSecond, 20U) << "second " << second;
        EXPECT_LE(limiter.trackedAddresses(), 20U) << "second " << second;
    }
}

/*
** The worst case for the map: one REJECT opens a window, 19 fill it at its
** very end, and 20 more open the next one. Only the first is a second old
** by then, so 39 addresses are remembered at once.
*/
TEST(RejectLimiterTests, NeverTracksMoreThanTwiceTheLimit)
{
    RejectLimiter limiter;
    std::uint32_t serial = 0;
    const auto sendAt    = [&](TimePoint now, std::uint32_t count) {
        for (std::uint32_t i = 0; i < count; ++i, ++serial)
            EXPECT_TRUE(limiter.allow(endpointFor(serial), now))
                << "address " << serial;
    };

    sendAt(T0, 1);
    sendAt(T0 + 990ms, RejectLimiter::LIMIT - 1);
    sendAt(T0 + 1s, RejectLimiter::LIMIT);

    EXPECT_EQ(limiter.trackedAddresses(), 2 * RejectLimiter::LIMIT - 1);
    EXPECT_FALSE(limiter.allow(endpointFor(serial), T0 + 1s));
    EXPECT_LE(limiter.trackedAddresses(), 2 * RejectLimiter::LIMIT);
}

TEST(RejectLimiterTests, ForgetsAnAddressAfterOneSecond)
{
    RejectLimiter limiter;

    EXPECT_TRUE(limiter.allow(endpointFor(1), T0));
    EXPECT_EQ(limiter.trackedAddresses(), 1U);

    EXPECT_TRUE(limiter.allow(endpointFor(2), T0 + 1s));
    EXPECT_EQ(limiter.trackedAddresses(), 1U);
}

} // namespace rtype::server::tests
