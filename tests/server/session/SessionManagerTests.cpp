/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SessionManagerTests.cpp
*/

#include <gtest/gtest.h>

#include <asio.hpp>
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

#include "Session.hpp"

namespace rtype::server::tests {

using namespace std::chrono_literals;

namespace {

const TimePoint T0{};

asio::ip::udp::endpoint endpointFor(std::uint32_t index)
{
    const auto address = asio::ip::address_v4(0x0a000001U + index);
    return {address, static_cast<unsigned short>(10000U + index)};
}

} // namespace

TEST(SessionManagerTests, AddsFindsAndRemovesFourSessions)
{
    SessionManager manager;
    std::vector<asio::ip::udp::endpoint> endpoints;

    for (std::uint32_t i = 0; i < SessionManager::MAX_SESSIONS; ++i) {
        const auto endpoint = endpointFor(i);
        endpoints.push_back(endpoint);

        auto &session = manager.add(
            endpoint,
            static_cast<std::uint8_t>(i + 1),
            "Player" + std::to_string(i + 1),
            T0
        );

        EXPECT_EQ(session.endpoint, endpoint);
        EXPECT_EQ(session.playerId, i + 1);
        EXPECT_EQ(session.name, "Player" + std::to_string(i + 1));
    }

    EXPECT_EQ(manager.size(), SessionManager::MAX_SESSIONS);

    for (std::uint32_t i = 0; i < endpoints.size(); ++i) {
        auto *session = manager.find(endpoints[i]);

        ASSERT_NE(session, nullptr);
        EXPECT_EQ(session->endpoint, endpoints[i]);
        EXPECT_EQ(session->playerId, i + 1);
    }

    const auto removedEndpoint = endpoints[1];
    manager.remove(removedEndpoint);

    EXPECT_EQ(manager.size(), SessionManager::MAX_SESSIONS - 1);
    EXPECT_EQ(manager.find(removedEndpoint), nullptr);

    for (std::size_t i = 0; i < endpoints.size(); ++i) {
        if (endpoints[i] == removedEndpoint)
            continue;

        EXPECT_NE(manager.find(endpoints[i]), nullptr);
    }
}

TEST(SessionManagerTests, ReportsSessionsTimedOutAfterFiveSeconds)
{
    SessionManager manager;
    const auto endpoint = endpointFor(42);

    manager.add(endpoint, 7, "TimeoutTest", T0);

    EXPECT_TRUE(manager.timedOut(T0 + 4900ms).empty());

    const auto expired = manager.timedOut(T0 + 5100ms);

    ASSERT_EQ(expired.size(), 1U);
    EXPECT_EQ(expired.front(), endpoint);
}

} // namespace rtype::server::tests