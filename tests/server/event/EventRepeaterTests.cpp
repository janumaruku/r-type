#include <algorithm>
#include <gtest/gtest.h>

#include "EventRepeater.hpp"

namespace {

rtype::protocol::Event event(std::uint32_t value)
{
    return {.a = value};
}

TEST(EventRepeaterTests, RepeatsEventsAndKeepsEventId)
{
    rtype::server::EventRepeater repeater;
    repeater.add(event(1), 3);
    repeater.add(event(2), 1);

    const auto first = repeater.takeDue();
    const auto second = repeater.takeDue();
    const auto third = repeater.takeDue();
    const auto fourth = repeater.takeDue();

    ASSERT_EQ(first.size(), 2U);
    ASSERT_EQ(second.size(), 1U);
    ASSERT_EQ(third.size(), 1U);
    EXPECT_TRUE(fourth.empty());
    EXPECT_EQ(first[0].eventId, second[0].eventId);
    EXPECT_EQ(second[0].eventId, third[0].eventId);
}

TEST(EventRepeaterTests, BoundsPendingEventsAndDropsOneCopyEventsFirst)
{
    rtype::server::EventRepeater repeater;
    for (std::uint32_t id = 0; id != 256; ++id)
        repeater.add(event(id), 1);
    repeater.add(event(1000), 3);

    const auto due = repeater.takeDue();

    EXPECT_EQ(due.size(), 256U);
    EXPECT_EQ(std::count_if(due.begin(), due.end(),
        [](const auto &item) { return item.a == 1000; }), 1);
}

}
