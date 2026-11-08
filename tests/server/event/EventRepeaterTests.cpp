#include "EventRepeater.hpp"

#include <algorithm>
#include <cstddef>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <string>

#include "engine/core/Logger.hpp"

namespace {

using rtype::server::EventRepeater;

rtype::protocol::Event event(std::uint32_t value)
{
    return {.a = value};
}

bool contains(const std::vector<rtype::protocol::Event> &events,
    std::uint32_t value)
{
    return std::ranges::any_of(events,
        [value](const auto &item) { return item.a == value; });
}

TEST(EventRepeaterTests, RepeatsEventsAndKeepsEventId)
{
    EventRepeater repeater;
    repeater.add(event(1), 3);
    repeater.add(event(2), 1);

    const auto first  = repeater.takeDue();
    const auto second = repeater.takeDue();
    const auto third  = repeater.takeDue();
    const auto fourth = repeater.takeDue();

    ASSERT_EQ(first.size(), 2U);
    ASSERT_EQ(second.size(), 1U);
    ASSERT_EQ(third.size(), 1U);
    EXPECT_TRUE(fourth.empty());
    EXPECT_EQ(first[0].eventId, second[0].eventId);
    EXPECT_EQ(second[0].eventId, third[0].eventId);
    EXPECT_EQ(repeater.pending(), 0U);
}

TEST(EventRepeaterTests, EventWithNoCopyIsIgnoredAndGetsNoId)
{
    EventRepeater repeater;

    repeater.add(event(1), 0);
    repeater.add(event(2), 1);

    const auto due = repeater.takeDue();

    ASSERT_EQ(due.size(), 1U);
    EXPECT_EQ(due[0].a, 2U);
    EXPECT_EQ(due[0].eventId, 1U);
}

TEST(EventRepeaterTests, FullQueueDropsTheOldestOneCopyEventFirst)
{
    EventRepeater repeater;

    for (std::uint32_t id = 0; id != EventRepeater::MAX_PENDING; ++id)
        repeater.add(event(id), 1);
    repeater.add(event(1000), 3);

    const auto due = repeater.takeDue();

    EXPECT_EQ(due.size(), EventRepeater::MAX_PENDING);
    EXPECT_TRUE(contains(due, 1000));
    EXPECT_FALSE(contains(due, 0));
    EXPECT_TRUE(contains(due, 1));
}

TEST(EventRepeaterTests, WithoutOneCopyEventsTheOldestIsDropped)
{
    EventRepeater repeater;

    for (std::uint32_t id = 0; id != EventRepeater::MAX_PENDING + 1; ++id)
        repeater.add(event(id), 3);

    const auto due = repeater.takeDue();

    EXPECT_EQ(due.size(), EventRepeater::MAX_PENDING);
    EXPECT_FALSE(contains(due, 0));
    EXPECT_TRUE(contains(due, EventRepeater::MAX_PENDING));
}

/*
** The log output is process-wide: the test captures it and gives std::cerr
** back when it ends.
*/
class EventRepeaterLog: public testing::Test {
protected:
    void SetUp() override
    {
        engine::core::log::setOutput(_output);
    }

    void TearDown() override
    {
        engine::core::log::setOutput(std::cerr);
    }

    [[nodiscard]] std::size_t warnings() const
    {
        const std::string text   = _output.str();
        const std::string needle = "Event queue full";
        std::size_t count        = 0;

        for (auto at = text.find(needle); at != std::string::npos;
            at       = text.find(needle, at + needle.size()))
            ++count;
        return count;
    }

    std::ostringstream _output;
};

TEST_F(EventRepeaterLog, ThreeHundredEventsInOneTickKeepTheRepeatedOnes)
{
    EventRepeater repeater;

    for (std::uint32_t id = 0; id != 300; ++id)
        repeater.add(event(id), id % 3 == 0 ? 3 : 1);
    EXPECT_EQ(repeater.pending(), EventRepeater::MAX_PENDING);

    const auto due = repeater.takeDue();

    EXPECT_EQ(due.size(), EventRepeater::MAX_PENDING);
    for (std::uint32_t id = 0; id != 300; id += 3)
        EXPECT_TRUE(contains(due, id)) << "repeated event " << id;
    EXPECT_EQ(warnings(), 1U);
    EXPECT_NE(_output.str().find("44 event(s) dropped"), std::string::npos);
    (void)repeater.takeDue();
    EXPECT_EQ(warnings(), 1U);
}

} // namespace
