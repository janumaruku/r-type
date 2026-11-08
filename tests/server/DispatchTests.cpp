#include <algorithm>
#include <gtest/gtest.h>

#include "ServerTestHelpers.hpp"

namespace {

namespace game  = rtype::game;
namespace proto = rtype::protocol;
using server_test::datagram;
using server_test::datagramWithVersion;
using server_test::endpointFor;
using server_test::Fixture;
using server_test::readMessage;
using namespace std::chrono_literals;

namespace {
constexpr std::uint8_t WRONG_VERSION = proto::Header::VERSION + 1;
}

/*
** A kick is queued, not sent: each flushGoodbyes (one per tick) sends one
** copy of the same KICK, 3 ticks in a row, then nothing.
*/
void expectKickOnThreeTicks(Fixture &fixture, proto::KickReason reason)
{
    EXPECT_TRUE(server_test::receive(fixture.client, 1, 50ms).empty());
    std::uint32_t sequence = 0;

    for (int tick = 0; tick != 3; ++tick) {
        fixture.adapter.flushGoodbyes();

        const auto kicks = server_test::receive(fixture.client, 1);

        ASSERT_EQ(kicks.size(), 1U) << "tick " << tick;
        EXPECT_EQ(readMessage<proto::Kick>(kicks[0]).reason, reason);
        if (tick == 0)
            sequence = server_test::readHeader(kicks[0]).sequence;
        EXPECT_EQ(server_test::readHeader(kicks[0]).sequence, sequence);
    }
    fixture.adapter.flushGoodbyes();
    EXPECT_TRUE(server_test::receive(fixture.client, 1, 50ms).empty());
}

TEST(DispatchTests, HelloCreatesSessionSendsWelcomeAndJoinsWorld)
{
    Fixture fixture;
    const auto from = fixture.client.local_endpoint();
    fixture.send(datagram(from, proto::MessageType::HELLO,
        proto::Hello{.playerName = "Alice"}));

    ASSERT_EQ(fixture.sessions.size(), 1U);
    ASSERT_NE(fixture.sessions.find(from), nullptr);
    EXPECT_EQ(fixture.world.roster().get(0)->state,
        game::SlotState::RESPAWNING);
    const auto responses = server_test::receive(fixture.client, 1);
    ASSERT_EQ(responses.size(), 1U);
    const auto welcome = readMessage<proto::Welcome>(responses.front());
    EXPECT_EQ(welcome.playerId, 0);
    EXPECT_EQ(welcome.tickRate, 60);
    EXPECT_EQ(welcome.snapshotRate, 30);
    EXPECT_EQ(welcome.maxPlayers, 4);
}

TEST(DispatchTests, SameHelloRepairsWelcomeWithoutCreatingSession)
{
    Fixture fixture;
    const auto from  = fixture.client.local_endpoint();
    const auto hello = datagram(from, proto::MessageType::HELLO,
        proto::Hello{.playerName = "Alice"});
    fixture.send(hello);
    fixture.send(hello);
    EXPECT_EQ(fixture.sessions.size(), 1U);
    EXPECT_EQ(server_test::receive(fixture.client, 2).size(), 2U);
}

TEST(DispatchTests, SessionDatagramsAreNumberedFromOne)
{
    Fixture fixture;
    const auto from  = fixture.client.local_endpoint();
    const auto hello = datagram(from, proto::MessageType::HELLO,
        proto::Hello{.playerName = "Alice"});

    fixture.send(hello);
    fixture.send(hello);

    const auto welcomes = server_test::receive(fixture.client, 2);

    ASSERT_EQ(welcomes.size(), 2U);
    EXPECT_EQ(server_test::readHeader(welcomes[0]).sequence, 1U);
    EXPECT_EQ(server_test::readHeader(welcomes[1]).sequence, 2U);
}

TEST(DispatchTests, RejectToAnAddressWithoutSessionIsSequenceOne)
{
    Fixture fixture;
    const auto from = fixture.client.local_endpoint();

    fixture.send(datagram(from, proto::MessageType::HELLO,
        proto::Hello{.protocolVersion = WRONG_VERSION, .playerName = "Alice"}));

    const auto rejects = server_test::receive(fixture.client, 1);

    ASSERT_EQ(rejects.size(), 1U);
    EXPECT_EQ(server_test::readHeader(rejects[0]).type,
        proto::MessageType::REJECT);
    EXPECT_EQ(server_test::readHeader(rejects[0]).sequence, 1U);
}

TEST(DispatchTests, ShutdownGoesToEverySessionThreeTimesTwentyMsApart)
{
    Fixture fixture;
    const auto from = fixture.client.local_endpoint();

    fixture.sessions.add(from, 0, "Alice", fixture.now);
    fixture.sessions.add(endpointFor(30), 1, "Bob", fixture.now);

    const auto start   = std::chrono::steady_clock::now();
    const auto sent    = fixture.adapter.broadcastShutdown();
    const auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_EQ(sent, 6U);
    EXPECT_GE(elapsed, 40ms);

    const auto shutdowns = server_test::receive(fixture.client, 3);

    ASSERT_EQ(shutdowns.size(), 3U);
    for (std::uint32_t copy = 0; copy != 3; ++copy) {
        EXPECT_EQ(server_test::readHeader(shutdowns[copy]).type,
            proto::MessageType::SHUTDOWN);
        EXPECT_EQ(server_test::readHeader(shutdowns[copy]).sequence, copy + 1);
    }
    EXPECT_TRUE(server_test::receive(fixture.client, 1, 50ms).empty());
}

/*
** A HELLO header with no body: TRUNCATED, which is logged even from an
** address with no session.
*/
net::Datagram truncatedHello(const asio::ip::udp::endpoint &from)
{
    auto hello = datagram(from, proto::MessageType::HELLO,
        proto::Hello{.playerName = "x"});

    hello.bytes.resize(proto::Header::SIZE + 1);
    return hello;
}

asio::ip::udp::endpoint stranger(unsigned index)
{
    return {asio::ip::address_v4(0x0A000000U + index), 4242};
}

TEST(DispatchTests, LogLimiterStaysBoundedUnderAFloodOfAddresses)
{
    using Adapter = rtype::server::ServerNetworkAdapter;
    Fixture fixture;

    for (unsigned i = 0; i != Adapter::MAX_LOG_ENTRIES + 500; ++i)
        fixture.send(truncatedHello(stranger(i)));
    EXPECT_EQ(fixture.adapter.loggedAddresses(), Adapter::MAX_LOG_ENTRIES);
    EXPECT_EQ(fixture.adapter.stats().totalTruncated,
        Adapter::MAX_LOG_ENTRIES + 500);

    fixture.now += 2s;
    fixture.send(truncatedHello(stranger(100000)));
    EXPECT_EQ(fixture.adapter.loggedAddresses(), 1U);
}

TEST(DispatchTests, VersionMismatchesAreRejected)
{
    Fixture fixture;
    const auto from = fixture.client.local_endpoint();
    fixture.send(datagramWithVersion(from, proto::MessageType::HELLO,
        proto::Hello{.playerName = "Alice"}, WRONG_VERSION));
    fixture.send(datagram(from, proto::MessageType::HELLO,
        proto::Hello{.protocolVersion = WRONG_VERSION, .playerName = "Alice"}));
    EXPECT_EQ(fixture.sessions.size(), 0U);
    ASSERT_EQ(server_test::receive(fixture.client, 1).size(), 1U);
    fixture.now += 1s;
    fixture.send(datagram(from, proto::MessageType::HELLO,
        proto::Hello{.protocolVersion = WRONG_VERSION, .playerName = "Alice"}));
    const auto responses = server_test::receive(fixture.client, 1);
    ASSERT_EQ(responses.size(), 1U);
    EXPECT_EQ(readMessage<proto::Reject>(responses.front()).reason,
        proto::RejectReason::VERSION_MISMATCH);
}

TEST(DispatchTests, FifthHelloIsRejected)
{
    Fixture fixture;
    const auto fifth = fixture.client.local_endpoint();

    for (unsigned i = 0; i != 4; ++i)
        fixture.send(datagram(endpointFor(i), proto::MessageType::HELLO,
            proto::Hello{.playerName = "Player"}));
    fixture.send(datagram(fifth, proto::MessageType::HELLO,
        proto::Hello{.playerName = "Player"}));
    EXPECT_EQ(fixture.sessions.size(), 4U);
    EXPECT_EQ(fixture.sessions.find(fifth), nullptr);

    const auto replies = server_test::receive(fixture.client, 2, 100ms);

    ASSERT_EQ(replies.size(), 1U);
    EXPECT_EQ(server_test::readHeader(replies[0]).type,
        proto::MessageType::REJECT);
    EXPECT_EQ(readMessage<proto::Reject>(replies[0]).reason,
        proto::RejectReason::SERVER_FULL);
}

TEST(DispatchTests, UnknownInputIsDropped)
{
    Fixture fixture;
    fixture.send(datagram(fixture.client.local_endpoint(),
        proto::MessageType::INPUT,
        proto::Input{.inputSequence = 1, .buttons = proto::buttons::RIGHT}));
    EXPECT_EQ(fixture.sessions.size(), 0U);
    EXPECT_TRUE(server_test::receive(fixture.client, 1, 50ms).empty());
    EXPECT_EQ(fixture.adapter.stats().totalKicked, 0U);
}

TEST(DispatchTests, ShutdownFromSessionCountsWrongDirection)
{
    Fixture fixture;
    const auto from = endpointFor(6);
    fixture.send(datagram(from, proto::MessageType::HELLO,
        proto::Hello{.playerName = "Player"}));
    fixture.send(datagram(from, proto::MessageType::SHUTDOWN,
        proto::Shutdown{}));
    EXPECT_EQ(fixture.adapter.stats().totalWrongDirection, 1U);
    EXPECT_EQ(fixture.sessions.size(), 1U);
}

TEST(DispatchTests, ElevenMalformedDatagramsKickTheSession)
{
    Fixture fixture;
    const auto from = fixture.client.local_endpoint();
    fixture.send(datagram(from, proto::MessageType::HELLO,
        proto::Hello{.playerName = "Player"}));
    ASSERT_EQ(server_test::receive(fixture.client, 1).size(), 1U);
    const auto malformed =
        datagram(from, proto::MessageType::INPUT, proto::Input{});
    for (int i = 0; i != 11; ++i)
        fixture.send({std::vector<std::uint8_t>(malformed.bytes.begin(),
                          malformed.bytes.end() - 1),
            from});
    EXPECT_EQ(fixture.sessions.find(from), nullptr);
    EXPECT_EQ(fixture.adapter.stats().totalTruncated, 11U)
        << "each bad datagram is counted once, not twice";
    const auto events = fixture.world.drainEvents();
    EXPECT_TRUE(std::any_of(events.begin(), events.end(),
        [](const game::GameEvent &event) {
            const auto *left = std::get_if<game::PlayerLeft>(&event);
            return left != nullptr && left->playerId == 0 &&
                left->reason == game::LeaveReason::KICKED;
        }));
    expectKickOnThreeTicks(fixture, proto::KickReason::PROTOCOL_ERROR);
}

TEST(DispatchTests, DisconnectCopiesAreIdempotent)
{
    Fixture fixture;
    const auto from = endpointFor(8);
    fixture.send(datagram(from, proto::MessageType::HELLO,
        proto::Hello{.playerName = "Player"}));
    const auto disconnect =
        datagram(from, proto::MessageType::DISCONNECT, proto::Disconnect{});
    fixture.send(disconnect);
    fixture.send(disconnect);
    fixture.send(disconnect);
    EXPECT_EQ(fixture.sessions.size(), 0U);
}

TEST(DispatchTests, ThreeFloodedSecondsKickTheSession)
{
    Fixture fixture;
    const auto from = fixture.client.local_endpoint();
    fixture.send(datagram(from, proto::MessageType::HELLO,
        proto::Hello{.playerName = "Player"}));
    ASSERT_EQ(server_test::receive(fixture.client, 1).size(), 1U);
    std::uint32_t sequence = 0;

    for (int second = 0; second != 3; ++second) {
        fixture.now = rtype::server::TimePoint{} + std::chrono::seconds(second);
        for (int packet = 0; packet != 300; ++packet)
            fixture.send(datagram(from, proto::MessageType::INPUT,
                proto::Input{.inputSequence = ++sequence}));
        if (second == 0) {
            const auto *const session = fixture.sessions.find(from);

            ASSERT_NE(session, nullptr);
            EXPECT_EQ(fixture.world.roster()
                          .get(session->playerId)
                          ->lastInputSequence,
                240U)
                << "the 60 INPUTs over 240 in one second are dropped";
        }
        if (second == 1) {
            EXPECT_NE(fixture.sessions.find(from), nullptr)
                << "2 flooded seconds aren't enough to kick";
        }
    }
    EXPECT_EQ(fixture.sessions.find(from), nullptr);
    EXPECT_EQ(fixture.adapter.stats().totalKicked, 1U);
    expectKickOnThreeTicks(fixture, proto::KickReason::RATE_LIMITED);
}

} // namespace
