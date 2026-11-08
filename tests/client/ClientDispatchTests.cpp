/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ClientDispatchTests
*/

#include "ClientDispatch.hpp"

#include <chrono>
#include <cstdint>
#include <gtest/gtest.h>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "engine/core/Errors.hpp"
#include "engine/core/Logger.hpp"
#include "LoopbackHelpers.hpp"
#include "rtype/protocol/Decode.hpp"
#include "rtype/protocol/Header.hpp"
#include "rtype/protocol/Messages.hpp"
#include "ServerLink.hpp"

using client_tests::occurrences;
using client_tests::rawPacket;
using rtype::client::ClientLogLimiter;
using rtype::client::ReceivedMessage;
using rtype::client::ServerLink;
using rtype::client::TimePoint;
using rtype::protocol::DecodeError;
using rtype::protocol::Hello;
using rtype::protocol::MessageType;
using rtype::protocol::Welcome;

namespace {
constexpr std::uint16_t SERVER_PORT = 4242;
constexpr std::size_t VERSION_BYTE  = 2;
constexpr std::size_t TYPE_BYTE     = 3;
const std::string DROPPED           = "Dropped a datagram";

const asio::ip::udp::endpoint SERVER{
    asio::ip::address_v4::loopback(), SERVER_PORT};
const asio::ip::udp::endpoint STRANGER{
    asio::ip::address_v4::loopback(), SERVER_PORT + 1};

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

std::vector<std::uint8_t> welcomeBytes()
{
    return bytesOf(rtype::protocol::makePacket(MessageType::WELCOME, 7,
        Welcome{.playerId = 2}));
}
} // namespace

/*
** The log output is process-wide: each test captures it and gives
** std::cerr back when it ends.
*/
class Dispatch: public testing::Test {
protected:
    void SetUp() override
    {
        engine::core::log::setOutput(_output);
        _link.server = SERVER;
    }

    void TearDown() override
    {
        engine::core::log::setOutput(std::cerr);
    }

    std::optional<ReceivedMessage> check(const net::Datagram &datagram,
        TimePoint now = TimePoint{})
    {
        return rtype::client::checkFromServer(datagram, _link, _logs, now);
    }

    [[nodiscard]] std::size_t droppedLines() const
    {
        return occurrences(_output.str(), DROPPED);
    }

    std::ostringstream _output;
    ServerLink _link;
    ClientLogLimiter _logs;
};

TEST_F(Dispatch, WelcomeFromTheServerComesBackReadyToDecode)
{
    auto message = check(datagramOf(welcomeBytes()));

    ASSERT_TRUE(message.has_value());
    EXPECT_EQ(message->header.type, MessageType::WELCOME);
    EXPECT_EQ(message->header.sequence, 7U);
    EXPECT_EQ(rtype::protocol::decode<Welcome>(message->packet),
        Welcome{.playerId = 2});
    EXPECT_EQ(droppedLines(), 0U);
}

TEST_F(Dispatch, DatagramFromAnotherAddressIsDroppedWithoutALog)
{
    EXPECT_FALSE(check(datagramOf(welcomeBytes(), STRANGER)).has_value());
    EXPECT_EQ(droppedLines(), 0U);
}

TEST_F(Dispatch, AnotherVersionIsDroppedWithoutALog)
{
    auto bytes = welcomeBytes();

    bytes[VERSION_BYTE] = rtype::protocol::Header::VERSION + 1;
    EXPECT_FALSE(check(datagramOf(bytes)).has_value());
    EXPECT_EQ(droppedLines(), 0U);
}

TEST_F(Dispatch, EachKindOfBadDatagramIsDroppedAndLogged)
{
    auto badMagic    = welcomeBytes();
    auto unknownType = welcomeBytes();
    const auto hello = bytesOf(rtype::protocol::makePacket(MessageType::HELLO,
        1, Hello{.playerName = "alice"}));

    badMagic[0] ^= 0xFFU;
    unknownType[TYPE_BYTE] = 0x7F;
    EXPECT_FALSE(check(datagramOf({1, 2, 3})).has_value());
    EXPECT_FALSE(check(datagramOf(badMagic)).has_value());
    EXPECT_FALSE(check(datagramOf(unknownType)).has_value());
    EXPECT_FALSE(check(datagramOf(hello)).has_value());
    EXPECT_EQ(droppedLines(), 4U);
}

TEST_F(Dispatch, TwentyBadDatagramsInOneSecondLogOneLinePerCode)
{
    const auto hello = bytesOf(rtype::protocol::makePacket(MessageType::HELLO,
        1, Hello{.playerName = "alice"}));
    const TimePoint start{};

    for (int i = 0; i < 20; ++i) {
        const TimePoint now = start + std::chrono::milliseconds(50 * i);

        EXPECT_FALSE(check(datagramOf({1, 2, 3}), now).has_value());
        EXPECT_FALSE(check(datagramOf(hello), now).has_value());
    }
    EXPECT_EQ(droppedLines(), 2U);
    EXPECT_FALSE(check(datagramOf({1, 2, 3}), start + std::chrono::seconds(1))
            .has_value());
    EXPECT_EQ(droppedLines(), 3U);
}

TEST(ClientLogLimiter, AllowsOneLinePerSecondForEachCode)
{
    ClientLogLimiter limiter;
    const TimePoint start{};

    EXPECT_TRUE(limiter.shouldLog(DecodeError::BAD_SIZE, start));
    EXPECT_FALSE(limiter.shouldLog(DecodeError::BAD_SIZE,
        start + std::chrono::milliseconds(999)));
    EXPECT_TRUE(limiter.shouldLog(DecodeError::BAD_MAGIC,
        start + std::chrono::milliseconds(999)));
    EXPECT_TRUE(limiter.shouldLog(DecodeError::BAD_SIZE,
        start + std::chrono::seconds(1)));
    EXPECT_FALSE(limiter.shouldLog(DecodeError::BAD_SIZE,
        start + std::chrono::milliseconds(1500)));
}

TEST(ResolveServer, NumericAddressGivesThatEndpoint)
{
    const auto endpoint = rtype::client::resolveServer("127.0.0.1", 4242);

    EXPECT_EQ(endpoint, SERVER);
}

TEST(ResolveServer, LocalhostGivesAnIpv4Address)
{
    const auto endpoint = rtype::client::resolveServer("localhost", 4242);

    EXPECT_TRUE(endpoint.address().is_v4());
    EXPECT_TRUE(endpoint.address().is_loopback());
    EXPECT_EQ(endpoint.port(), 4242);
}

TEST(ResolveServer, UnknownHostThrowsNetworkError)
{
    EXPECT_THROW((void)rtype::client::resolveServer("no.such.host.invalid",
                     4242),
        engine::core::NetworkError);
}
