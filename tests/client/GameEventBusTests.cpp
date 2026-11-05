/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GameEventBusTests.cpp
*/

#include <gtest/gtest.h>

#include <optional>
#include <utility>
#include <vector>

#include "GameEventBus.hpp"

namespace rtype::client {

TEST(GameEventBusTests, DispatchCallsEachSubscriberOnceInOrder)
{
    GameEventBus bus;
    std::vector<int> calls;
    auto first = bus.subscribe([&](const protocol::Event &) { calls.push_back(1); });
    auto second = bus.subscribe([&](const protocol::Event &) { calls.push_back(2); });

    bus.publish(protocol::Event{});
    bus.dispatch();

    EXPECT_EQ(calls, (std::vector<int>{1, 2}));
}

TEST(GameEventBusTests, PublishWithoutDispatchCallsNoHandler)
{
    GameEventBus bus;
    int calls = 0;
    auto sub = bus.subscribe([&](const protocol::Event &) { ++calls; });

    bus.publish(protocol::Event{});

    EXPECT_EQ(calls, 0);
}

TEST(GameEventBusTests, DestroyedSubscriptionIsNotCalledAndIsRemoved)
{
    GameEventBus bus;
    int keptCalls = 0;
    int removedCalls = 0;
    auto kept = bus.subscribe([&](const protocol::Event &) { ++keptCalls; });
    {
        auto removed = bus.subscribe([&](const protocol::Event &) { ++removedCalls; });
        EXPECT_EQ(bus.subscribers(), 2u);
    }

    EXPECT_EQ(bus.subscribers(), 1u);
    bus.publish(protocol::Event{});
    bus.dispatch();

    EXPECT_EQ(keptCalls, 1);
    EXPECT_EQ(removedCalls, 0);
}

TEST(GameEventBusTests, MovedSubscriptionStaysActiveWhenSourceIsDestroyed)
{
    GameEventBus bus;
    int calls = 0;
    GameEventBus::Subscription kept;
    {
        auto original = bus.subscribe([&](const protocol::Event &) { ++calls; });
        kept = std::move(original);
    }

    EXPECT_EQ(bus.subscribers(), 1u);
    bus.publish(protocol::Event{});
    bus.dispatch();

    EXPECT_EQ(calls, 1);
}

TEST(GameEventBusTests, EventPublishedDuringDispatchIsDeliveredOnNextDispatch)
{
    GameEventBus bus;
    int calls = 0;
    auto sub = bus.subscribe([&](const protocol::Event &) {
        ++calls;
        if (calls == 1) {
            bus.publish(protocol::Event{});
        }
    });
    bus.publish(protocol::Event{});

    bus.dispatch();
    EXPECT_EQ(calls, 1);

    bus.dispatch();
    EXPECT_EQ(calls, 2);
}

TEST(GameEventBusTests, HandlerUnsubscribingAnotherDoesNotCrashOrCallIt)
{
    GameEventBus bus;
    std::optional<GameEventBus::Subscription> victim;
    int victimCalls = 0;
    auto remover = bus.subscribe([&](const protocol::Event &) { victim.reset(); });
    victim.emplace(bus.subscribe([&](const protocol::Event &) { ++victimCalls; }));
    bus.publish(protocol::Event{});

    bus.dispatch();

    EXPECT_EQ(victimCalls, 0);
    EXPECT_EQ(bus.subscribers(), 1u);
}

} // namespace rtype::client