/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** EventFilterTests.cpp
*/

#include <gtest/gtest.h>

#include <cstdint>

#include "GameEventBus.hpp"

namespace rtype::client {

TEST(EventFilterTests, RepeatedIdIsRejectedUntilForgotten)
{
    EventFilter filter;

    EXPECT_TRUE(filter.firstTime(5));
    EXPECT_FALSE(filter.firstTime(5));
    EXPECT_TRUE(filter.firstTime(6));

    for (std::uint32_t id = 100; id < 230; ++id) {
        EXPECT_TRUE(filter.firstTime(id));
    }

    EXPECT_TRUE(filter.firstTime(5));
}

TEST(EventFilterTests, RemembersExactlyCapacityIds)
{
    constexpr auto last = static_cast<std::uint32_t>(EventFilter::CAPACITY);
    EventFilter filter;

    for (std::uint32_t id = 1; id <= last; ++id) {
        EXPECT_TRUE(filter.firstTime(id));
    }
    EXPECT_FALSE(filter.firstTime(last));
    EXPECT_TRUE(filter.firstTime(last + 1));
    EXPECT_TRUE(filter.firstTime(1));
}

} // namespace rtype::client