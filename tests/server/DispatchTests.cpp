#include <gtest/gtest.h>

#include <algorithm>
#include "ServerTestHelpers.hpp"

namespace {

namespace game = rtype::game;
namespace proto = rtype::protocol;
using server_test::Fixture;
using server_test::datagram;
using server_test::datagramWithVersion;
using server_test::endpointFor;
using server_test::readMessage;
using namespace std::chrono_literals;

TEST(DispatchTests, HelloCreatesSessionSendsWelcomeAndJoinsWorld)
{
    Fixture fixture;
    const auto from = fixture.client.local_endpoint();
    fixture.send(datagram(from, proto::MessageType::HELLO,
        proto::Hello{.playerName = "Alice"}));

    ASSERT_EQ(fixture.sessions.size(), 1U);
    ASSERT_NE(fixture.sessions.find(from), nullptr);
    EXPECT_EQ(fixture.world.roster().get(0)->state, game::SlotState::RESPAWNING);
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
    const auto from = fixture.client.local_endpoint();
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
    const auto from = fixture.client.local_endpoint();
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
        proto::Hello{.protocolVersion = 2, .playerName = "Alice"}));

    const auto rejects = server_test::receive(fixture.client, 1);

    ASSERT_EQ(rejects.size(), 1U);
    EXPECT_EQ(server_test::readHeader(rejects[0]).type,
        proto::MessageType::REJECT);
    EXPECT_EQ(server_test::readHeader(rejects[0]).sequence, 1U);
}

TEST(DispatchTests, VersionMismatchesAreRejected)
{
    Fixture fixture;
    const auto from = fixture.client.local_endpoint();
    fixture.send(datagramWithVersion(from, proto::MessageType::HELLO,
        proto::Hello{.playerName = "Alice"}, 2));
    fixture.send(datagram(from, proto::MessageType::HELLO,
        proto::Hello{.protocolVersion = 2, .playerName = "Alice"}));
    EXPECT_EQ(fixture.sessions.size(), 0U);
    ASSERT_EQ(server_test::receive(fixture.client, 1).size(), 1U);
    fixture.now += 1s;
    fixture.send(datagram(from, proto::MessageType::HELLO,
        proto::Hello{.protocolVersion = 2, .playerName = "Alice"}));
    const auto responses = server_test::receive(fixture.client, 1);
    ASSERT_EQ(responses.size(), 1U);
    EXPECT_EQ(readMessage<proto::Reject>(responses.front()).reason,
        proto::RejectReason::VERSION_MISMATCH);
}

TEST(DispatchTests, FifthHelloIsRejected)
{
    Fixture fixture;
    for (unsigned i = 0; i != 4; ++i)
        fixture.send(datagram(endpointFor(i), proto::MessageType::HELLO,
            proto::Hello{.playerName = "Player"}));
    fixture.send(datagram(endpointFor(4), proto::MessageType::HELLO,
        proto::Hello{.playerName = "Player"}));
    EXPECT_EQ(fixture.sessions.size(), 4U);
}

TEST(DispatchTests, UnknownInputIsDropped)
{
    Fixture fixture;
    fixture.send(datagram(endpointFor(5), proto::MessageType::INPUT,
        proto::Input{.inputSequence = 1, .buttons = proto::buttons::RIGHT}));
    EXPECT_EQ(fixture.sessions.size(), 0U);
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
    const auto malformed = datagram(from, proto::MessageType::INPUT,
        proto::Input{});
    for (int i = 0; i != 11; ++i)
        fixture.send({std::vector<std::uint8_t>(malformed.bytes.begin(),
            malformed.bytes.end() - 1), from});
    EXPECT_EQ(fixture.sessions.find(from), nullptr);
    const auto events = fixture.world.drainEvents();
    EXPECT_TRUE(std::any_of(events.begin(), events.end(),
        [](const game::GameEvent &event) {
            const auto *left = std::get_if<game::PlayerLeft>(&event);
            return left != nullptr && left->playerId == 0 &&
                left->reason == game::LeaveReason::KICKED;
        }));
    const auto kicks = server_test::receive(fixture.client, 3);
    ASSERT_EQ(kicks.size(), 3U);
    for (const auto &kick : kicks)
        EXPECT_EQ(readMessage<proto::Kick>(kick).reason,
            proto::KickReason::PROTOCOL_ERROR);
}

TEST(DispatchTests, DisconnectCopiesAreIdempotent)
{
    Fixture fixture;
    const auto from = endpointFor(8);
    fixture.send(datagram(from, proto::MessageType::HELLO,
        proto::Hello{.playerName = "Player"}));
    const auto disconnect = datagram(from, proto::MessageType::DISCONNECT,
        proto::Disconnect{});
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
    const auto input = datagram(from, proto::MessageType::INPUT,
        proto::Input{});
    for (int second = 0; second != 3; ++second) {
        fixture.now = rtype::server::TimePoint{} +
            std::chrono::seconds(second);
        for (int packet = 0; packet != 300; ++packet)
            fixture.send(input);
    }
    EXPECT_EQ(fixture.sessions.find(from), nullptr);
    EXPECT_EQ(fixture.adapter.stats().totalKicked, 1U);
    const auto kicks = server_test::receive(fixture.client, 3);
    ASSERT_EQ(kicks.size(), 3U);
    for (const auto &kick : kicks)
        EXPECT_EQ(readMessage<proto::Kick>(kick).reason,
            proto::KickReason::RATE_LIMITED);
}

}
