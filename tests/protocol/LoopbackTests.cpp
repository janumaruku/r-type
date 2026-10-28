/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** LoopbackTests
*/

/*
** The week 1 gate: a test client gets WELCOME from a test server on loopback.
** The test server is only handleOne, which follows the dispatch order of RFC
** section 10.3 for one datagram. It keeps no sessions and doesn't apply the
** one-REJECT-per-second rule, which belong to the server application.
*/

#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "network/Packet.hpp"
#include "network/UdpSocket.hpp"
#include "rtype/protocol/Decode.hpp"
#include "rtype/protocol/Header.hpp"
#include "rtype/protocol/Messages.hpp"
#include "rtype/protocol/MessageType.hpp"
#include "rtype/protocol/ProtocolError.hpp"

using namespace rtype::protocol;
using namespace std::chrono_literals;

namespace {

using Bytes = std::vector<std::uint8_t>;

constexpr std::chrono::milliseconds DEADLINE = 2s;
constexpr std::chrono::milliseconds SILENCE = 200ms;

asio::ip::udp::endpoint loopbackOf(const net::UdpSocket &socket)
{
    return {asio::ip::address_v4::loopback(), socket.localEndpoint().port()};
}

/*
** Polls instead of sleeping a fixed time, so the test is fast on a fast
** machine and still passes on a slow one.
*/
std::optional<net::Datagram> waitForDatagram(net::UdpSocket &socket,
    std::chrono::milliseconds timeout)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;

    while (std::chrono::steady_clock::now() < deadline) {
        std::vector<net::Datagram> received = socket.receiveAll(1);

        if (!received.empty())
            return std::move(received.front());
        std::this_thread::sleep_for(1ms);
    }
    return std::nullopt;
}

struct TestServer {
    net::UdpSocket socket{0};
    std::uint32_t sequence = 0;

    template <class Message>
    void reply(const asio::ip::udp::endpoint &to, const Message &message)
    {
        socket.sendTo(to, makePacket(Message::TYPE, ++sequence, message));
    }
};

/*
** A test helper, not a server: handles one datagram in the dispatch order of
** RFC section 10.3 and answers it, or reports why it was dropped without an
** answer.
*/
std::optional<DecodeError> handleOne(TestServer &server)
{
    const std::optional<net::Datagram> datagram = waitForDatagram(server.socket,
        DEADLINE);

    if (!datagram) {
        ADD_FAILURE() << "the test server received nothing";
        return std::nullopt;
    }
    try {
        checkDatagramSize(datagram->bytes.size());
        net::Packet packet{datagram->bytes};
        const auto header = packet.read<Header>();

        if (header.version != Header::VERSION) {
            if (header.type == MessageType::HELLO)
                server.reply(datagram->from,
                    Reject{.reason = RejectReason::VERSION_MISMATCH});
            return std::nullopt;
        }
        checkType(header.type, Direction::CLIENT_TO_SERVER);
        if (header.type != MessageType::HELLO) {
            ADD_FAILURE() << "handleOne only handles HELLO, not "
                << header.type;
            return std::nullopt;
        }
        const auto hello = decode<Hello>(packet);

        if (hello.protocolVersion != Header::VERSION) {
            server.reply(datagram->from,
                Reject{.reason = RejectReason::VERSION_MISMATCH});
            return std::nullopt;
        }
        server.reply(datagram->from, Welcome{});
    } catch (const net::PacketError &error) {
        return classify(error);
    }
    return std::nullopt;
}

/*
** Waits for the server's answer, checks its header, and decodes its payload.
*/
template <class Message>
std::optional<Message> receiveReply(net::UdpSocket &client,
    std::uint32_t sequence)
{
    const std::optional<net::Datagram> datagram = waitForDatagram(client,
        DEADLINE);

    if (!datagram) {
        ADD_FAILURE() << "the client received no " << Message::TYPE;
        return std::nullopt;
    }
    net::Packet packet{datagram->bytes};
    const auto header = packet.read<Header>();

    EXPECT_EQ(header.magic, Header::MAGIC);
    EXPECT_EQ(header.version, Header::VERSION);
    EXPECT_EQ(header.type, Message::TYPE);
    EXPECT_EQ(header.sequence, sequence);
    if (header.type != Message::TYPE)
        return std::nullopt;
    return decode<Message>(packet);
}

void expectNothingReceived(net::UdpSocket &client)
{
    EXPECT_FALSE(waitForDatagram(client, SILENCE).has_value())
        << "the client received an answer to a dropped datagram";
}

template <class Message>
Bytes bytesOf(MessageType type, const Message &message)
{
    const net::Packet packet = makePacket(type, 1, message);
    const auto bytes = packet.bytes();

    return {bytes.begin(), bytes.end()};
}

void sendBytes(net::UdpSocket &client, const TestServer &server,
    const Bytes &bytes)
{
    net::Packet packet;

    packet.writeRaw(bytes.data(), bytes.size());
    client.sendTo(loopbackOf(server.socket), packet);
}

class LoopbackTests: public ::testing::Test {
protected:
    TestServer server;
    net::UdpSocket client{0};

    void SetUp() override
    {
        server.socket.start();
        client.start();
    }
};

}

TEST_F(LoopbackTests, AHelloGetsAWelcome)
{
    client.sendTo(loopbackOf(server.socket),
        makePacket(MessageType::HELLO, 1, Hello{.playerName = "Alice"}));

    EXPECT_EQ(handleOne(server), std::nullopt);
    const std::optional<Welcome> welcome = receiveReply<Welcome>(client, 1);
    ASSERT_TRUE(welcome.has_value());
    EXPECT_EQ(*welcome, (Welcome{.playerId = 0, .tickRate = 60,
        .snapshotRate = 30, .maxPlayers = 4}));
}

TEST_F(LoopbackTests, AnotherProtocolVersionInTheHelloGetsAReject)
{
    client.sendTo(loopbackOf(server.socket), makePacket(MessageType::HELLO, 1,
        Hello{.protocolVersion = 2, .playerName = "Bob"}));

    EXPECT_EQ(handleOne(server), std::nullopt);
    const std::optional<Reject> reject = receiveReply<Reject>(client, 1);
    ASSERT_TRUE(reject.has_value());
    EXPECT_EQ(reject->reason, RejectReason::VERSION_MISMATCH);
}

TEST_F(LoopbackTests, AnotherHeaderVersionGetsAReject)
{
    Bytes bytes = bytesOf(MessageType::HELLO, Hello{.playerName = "Bob"});

    bytes[2] = 2;
    sendBytes(client, server, bytes);

    EXPECT_EQ(handleOne(server), std::nullopt);
    const std::optional<Reject> reject = receiveReply<Reject>(client, 1);
    ASSERT_TRUE(reject.has_value());
    EXPECT_EQ(reject->reason, RejectReason::VERSION_MISMATCH);
}

/*
** A client resends HELLO every 0.5 s until it is answered, and RFC section
** 6.2 answers every one of them, so a lost WELCOME is repaired by the next.
*/
TEST_F(LoopbackTests, EveryHelloGetsItsOwnWelcome)
{
    for (std::uint32_t sequence = 1; sequence <= 3; ++sequence)
        client.sendTo(loopbackOf(server.socket), makePacket(MessageType::HELLO,
            sequence, Hello{.playerName = "Alice"}));

    for (std::uint32_t sequence = 1; sequence <= 3; ++sequence) {
        SCOPED_TRACE("HELLO " + std::to_string(sequence));
        EXPECT_EQ(handleOne(server), std::nullopt);
        EXPECT_TRUE(receiveReply<Welcome>(client, sequence).has_value());
    }
}

TEST_F(LoopbackTests, AShutdownSentToTheServerIsDroppedWithoutAnswer)
{
    client.sendTo(loopbackOf(server.socket),
        makePacket(MessageType::SHUTDOWN, 1, Shutdown{}));

    EXPECT_EQ(handleOne(server), DecodeError::WRONG_DIRECTION);
    expectNothingReceived(client);
}

TEST_F(LoopbackTests, ABadMagicOrAnExtraByteIsDroppedWithoutAnswer)
{
    Bytes badMagic = bytesOf(MessageType::HELLO, Hello{.playerName = "Eve"});
    Bytes extraByte = badMagic;

    badMagic[0] = 0;
    extraByte.push_back(0);

    sendBytes(client, server, badMagic);
    EXPECT_EQ(handleOne(server), DecodeError::BAD_MAGIC);
    sendBytes(client, server, extraByte);
    EXPECT_EQ(handleOne(server), DecodeError::TRAILING_BYTES);
    expectNothingReceived(client);
}
