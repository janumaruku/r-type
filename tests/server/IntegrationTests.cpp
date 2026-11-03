#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

#include "ServerApp.hpp"
#include "TestClient.hpp"

namespace {

using namespace std::chrono_literals;
using rtype::protocol::Event;
using rtype::protocol::EventType;
using rtype::protocol::Hello;
using rtype::protocol::MessageType;
using rtype::protocol::Reject;
using rtype::protocol::RejectReason;
using rtype::protocol::Snapshot;
using rtype::protocol::Welcome;

class ServerFixture {
public:
    ServerFixture()
    {
        rtype::server::ServerConfig config;
        config.port = 0;
        config.seed = 7;
        _app = std::make_unique<rtype::server::ServerApp>(config);
        _thread = std::thread([this] { _result = _app->run(); });
    }

    ~ServerFixture()
    {
        if (!_stopped)
            stop();
    }

    [[nodiscard]] std::uint16_t port() const
    {
        return _app->port();
    }

    [[nodiscard]] int result() const
    {
        return _result;
    }

    void stop()
    {
        _app->requestStop();
        _thread.join();
        _stopped = true;
    }

private:
    std::unique_ptr<rtype::server::ServerApp> _app;
    std::thread _thread;
    int _result = -1;
    bool _stopped = false;
};

std::optional<Snapshot> join(server_test::TestClient &client)
{
    client.send(MessageType::HELLO, Hello{.playerName = "player"});
    EXPECT_TRUE(client.waitFor<Welcome>(MessageType::WELCOME).has_value());
    return client.waitFor<Snapshot>(MessageType::SNAPSHOT);
}

TEST(IntegrationTests, JoinReceivesWelcomeAndOwnShip)
{
    ServerFixture server;
    server_test::TestClient client(server.port());

    const auto snapshot = join(client);

    ASSERT_TRUE(snapshot.has_value());
    ASSERT_EQ(snapshot->roster.size(), 4U);
    EXPECT_EQ(snapshot->roster[0].state, rtype::protocol::PlayerState::ALIVE);
    EXPECT_TRUE(std::any_of(snapshot->entities.begin(), snapshot->entities.end(),
        [](const auto &entity) {
            return entity.kind == rtype::protocol::EntityKind::PLAYER_SHIP;
        }));
}

TEST(IntegrationTests, TwoPlayersSeeMovement)
{
    ServerFixture server;
    server_test::TestClient first(server.port());
    server_test::TestClient second(server.port());
    ASSERT_TRUE(join(first).has_value());
    ASSERT_TRUE(join(second).has_value());

    first.send(MessageType::INPUT,
        rtype::protocol::Input{.inputSequence = 1,
            .buttons = rtype::protocol::buttons::RIGHT});
    const auto initial = second.waitFor<Snapshot>(MessageType::SNAPSHOT);
    ASSERT_TRUE(initial.has_value());
    std::vector<std::pair<std::uint32_t, float>> initialShips;
    for (const auto &entity : initial->entities) {
        if (entity.kind == rtype::protocol::EntityKind::PLAYER_SHIP)
            initialShips.emplace_back(entity.netId, entity.x);
    }
    ASSERT_FALSE(initialShips.empty());

    for (std::uint32_t sequence = 2; sequence != 62; ++sequence)
        first.send(MessageType::INPUT,
            rtype::protocol::Input{.inputSequence = sequence,
                .buttons = rtype::protocol::buttons::RIGHT});

    bool moved = false;
    const auto deadline = std::chrono::steady_clock::now() + 2s;
    while (!moved && std::chrono::steady_clock::now() < deadline) {
        const auto snapshot = second.waitFor<Snapshot>(
            MessageType::SNAPSHOT, 100ms);
        if (snapshot) {
            for (const auto &[netId, initialX] : initialShips) {
                const auto ship = std::find_if(snapshot->entities.begin(),
                    snapshot->entities.end(), [netId](const auto &entity) {
                        return entity.netId == netId;
                    });
                moved = ship != snapshot->entities.end() && ship->x > initialX;
                if (moved)
                    break;
            }
        }
    }
    EXPECT_TRUE(moved);
}

TEST(IntegrationTests, FullServerRejectsFifthClient)
{
    ServerFixture server;
    std::vector<std::unique_ptr<server_test::TestClient>> players;
    for (int i = 0; i != 4; ++i) {
        players.push_back(std::make_unique<server_test::TestClient>(
            server.port()));
        players.back()->send(MessageType::HELLO,
            Hello{.playerName = "player"});
        ASSERT_TRUE(players.back()->waitFor<Welcome>(
            MessageType::WELCOME).has_value()) << "player " << i;
    }
    server_test::TestClient fifth(server.port());

    fifth.send(MessageType::HELLO, Hello{.playerName = "fifth"});

    const auto reject = fifth.waitFor<Reject>(MessageType::REJECT);
    ASSERT_TRUE(reject.has_value());
    EXPECT_EQ(reject->reason, RejectReason::SERVER_FULL);
}

TEST(IntegrationTests, VersionMismatchIsRejected)
{
    ServerFixture server;
    server_test::TestClient client(server.port());
    net::Packet packet;
    packet << rtype::protocol::Header{
        .version = 2, .type = MessageType::HELLO}
        << Hello{.playerName = "bad"};
    client.sendRaw(packet.bytes());

    const auto reject = client.waitFor<Reject>(MessageType::REJECT);
    ASSERT_TRUE(reject.has_value());
    EXPECT_EQ(reject->reason, RejectReason::VERSION_MISMATCH);
}

TEST(IntegrationTests, QuitIsRepeatedAndRemovesTheShip)
{
    ServerFixture server;
    server_test::TestClient quitter(server.port());
    server_test::TestClient other(server.port());
    ASSERT_TRUE(join(quitter).has_value());
    ASSERT_TRUE(join(other).has_value());

    for (int copy = 0; copy != 3; ++copy)
        quitter.send(MessageType::DISCONNECT,
            rtype::protocol::Disconnect{});

    std::vector<Event> left;
    std::size_t seenEvents = 0;
    const auto deadline = std::chrono::steady_clock::now() + 2s;
    while (std::chrono::steady_clock::now() < deadline && left.size() < 3) {
        const auto events = other.eventsSoFar();
        for (std::size_t index = seenEvents; index < events.size(); ++index) {
            const auto &event = events[index];
            if (event.type == EventType::PLAYER_LEFT &&
                event.b == static_cast<std::uint32_t>(
                    rtype::protocol::LeaveReason::QUIT))
                left.push_back(event);
        }
        seenEvents = events.size();
        std::this_thread::yield();
    }
    ASSERT_EQ(left.size(), 3U);
    EXPECT_EQ(left[0].eventId, left[1].eventId);
    EXPECT_EQ(left[1].eventId, left[2].eventId);
}

TEST(IntegrationTests, SlowClientTimesOut)
{
    ServerFixture server;
    server_test::TestClient silent(server.port());
    server_test::TestClient other(server.port());
    ASSERT_TRUE(join(silent).has_value());
    ASSERT_TRUE(join(other).has_value());

    int left = 0;
    const auto deadline = std::chrono::steady_clock::now() + 7s;
    while (std::chrono::steady_clock::now() < deadline && left == 0) {
        const auto event = other.waitFor<Event>(MessageType::EVENT, 200ms);
        if (event && event->type == EventType::PLAYER_LEFT)
            left = 1;
    }
    EXPECT_EQ(left, 1);
}

TEST(IntegrationTests, MalformedTrafficKicksOnlyTheSender)
{
    ServerFixture server;
    server_test::TestClient bad(server.port());
    server_test::TestClient other(server.port());
    ASSERT_TRUE(join(bad).has_value());
    ASSERT_TRUE(join(other).has_value());

    const std::array<std::uint8_t, 1> garbage{0};
    for (int copy = 0; copy != 20; ++copy)
        bad.sendRaw(garbage);

    const auto kick = bad.waitFor<rtype::protocol::Kick>(MessageType::KICK);
    ASSERT_TRUE(kick.has_value());
    EXPECT_TRUE(other.waitFor<Event>(MessageType::EVENT, 500ms).has_value());
}

TEST(IntegrationTests, WrongDirectionIsDropped)
{
    ServerFixture server;
    server_test::TestClient client(server.port());
    ASSERT_TRUE(join(client).has_value());

    client.send(MessageType::SHUTDOWN, rtype::protocol::Shutdown{});
    EXPECT_FALSE(client.waitFor<Welcome>(MessageType::WELCOME, 100ms)
        .has_value());
}

TEST(IntegrationTests, ShutdownStopsServer)
{
    ServerFixture server;
    server_test::TestClient first(server.port());
    server_test::TestClient second(server.port());
    ASSERT_TRUE(join(first).has_value());
    ASSERT_TRUE(join(second).has_value());

    server.stop();
    EXPECT_TRUE(first.waitFor<rtype::protocol::Shutdown>(
        MessageType::SHUTDOWN).has_value());
    EXPECT_TRUE(second.waitFor<rtype::protocol::Shutdown>(
        MessageType::SHUTDOWN).has_value());
    EXPECT_EQ(server.result(), 0);
}

}
