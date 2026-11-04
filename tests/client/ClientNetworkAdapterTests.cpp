/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** AdapterTests
*/

#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

#include "ClientNetworkAdapter.hpp"
#include "GameEventBus.hpp"
#include "ServerLink.hpp"
#include "SpriteCatalog.hpp"
#include "engine/core/Logger.hpp"
#include "rtype/protocol/Event.hpp"
#include "rtype/protocol/Header.hpp"
#include "rtype/protocol/Messages.hpp"

using rtype::client::ClientNetworkAdapter;
using rtype::client::ClientWorld;
using rtype::client::GameEventBus;
using rtype::client::LeaveCause;
using rtype::client::ServerLink;
using rtype::client::TimePoint;
using rtype::protocol::Event;
using rtype::protocol::EventType;
using rtype::protocol::MessageType;
using rtype::protocol::Welcome;

namespace {
constexpr std::uint16_t SERVER_PORT = 4242;

const asio::ip::udp::endpoint SERVER{asio::ip::address_v4::loopback(),
    SERVER_PORT};
const asio::ip::udp::endpoint STRANGER{asio::ip::address_v4::loopback(),
    SERVER_PORT + 1};

const TimePoint START{};
const TimePoint LATER = START + std::chrono::seconds(5);

std::vector<std::uint8_t> bytesOf(const net::Packet &packet)
{
    const auto bytes = packet.bytes();

    return {bytes.begin(), bytes.end()};
}

net::Datagram datagramOf(std::vector<std::uint8_t> bytes,
    const asio::ip::udp::endpoint &from = SERVER)
{
    return {.bytes = std::move(bytes), .from = from};
}

Event eventWithId(std::uint32_t id)
{
    return {.eventId = id, .type = EventType::ENEMY_KILLED, .a = 1, .b = 2,
        .x = 3.0F, .y = 4.0F};
}

std::vector<std::uint8_t> eventBytes(std::uint32_t id)
{
    return bytesOf(rtype::protocol::makePacket(MessageType::EVENT, 1,
        eventWithId(id)));
}
}

class Adapter: public testing::Test {
protected:
    using SpriteCatalog = rtype::client::SpriteCatalog;
    void SetUp() override
    {
        engine::core::log::setOutput(_output);
        _link.server = SERVER;
    }

    void TearDown() override
    {
        engine::core::log::setOutput(std::cerr);
    }

    std::ostringstream _output;
    ServerLink _link;
    ClientWorld _world{SpriteCatalog{}};
    GameEventBus _events;
    ClientNetworkAdapter _adapter{_link, _world, _events};
};

TEST_F(Adapter, ThreeCopiesOfOneEventArePublishedOnce)
{
    std::vector<Event> received;
    auto sub = _events.subscribe([&](const Event &event) {
        received.push_back(event);
    });
    const auto bytes = eventBytes(42);

    for (int i = 0; i < 3; ++i) {
        _adapter.handle(datagramOf(bytes), START);
    }
    _events.dispatch();

    ASSERT_EQ(received.size(), 1U);
    EXPECT_EQ(received.front(), eventWithId(42));
}

TEST_F(Adapter, EventFromAnotherAddressIsDroppedAndLinkNotRefreshed)
{
    std::vector<Event> received;
    auto sub = _events.subscribe([&](const Event &event) {
        received.push_back(event);
    });

    _adapter.handle(datagramOf(eventBytes(7), STRANGER), LATER);
    _events.dispatch();

    EXPECT_TRUE(received.empty());
    EXPECT_EQ(_link.lastHeard, START);
}

TEST_F(Adapter, LateWelcomeInGameIsDroppedAndLinkNotRefreshed)
{
    const auto bytes = bytesOf(rtype::protocol::makePacket(
        MessageType::WELCOME, 3, Welcome{.playerId = 1}));

    _adapter.handle(datagramOf(bytes), LATER);

    EXPECT_EQ(_link.lastHeard, START);
}

TEST_F(Adapter, FirstLeaveCauseWinsOverLaterKick)
{
    const auto shutdown = bytesOf(rtype::protocol::makePacket(
        MessageType::SHUTDOWN, 1, rtype::protocol::Shutdown{}));
    const auto kick = bytesOf(rtype::protocol::makePacket(
        MessageType::KICK, 2, rtype::protocol::Kick{
            .reason = rtype::protocol::KickReason::RATE_LIMITED}));

    _adapter.handle(datagramOf(shutdown), START);
    _adapter.handle(datagramOf(kick), START);

    EXPECT_EQ(_adapter.leaveCause(), LeaveCause::SHUTDOWN);
}