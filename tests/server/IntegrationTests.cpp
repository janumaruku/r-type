#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <optional>
#include <thread>
#include <vector>

#include "ServerApp.hpp"
#include "TestClient.hpp"

namespace {

using namespace std::chrono_literals;
using rtype::protocol::Event;
using rtype::protocol::EventType;
using rtype::protocol::Hello;
using rtype::protocol::Input;
using rtype::protocol::Kick;
using rtype::protocol::KickReason;
using rtype::protocol::LeaveReason;
using rtype::protocol::MessageType;
using rtype::protocol::PlayerInfo;
using rtype::protocol::PlayerState;
using rtype::protocol::Reject;
using rtype::protocol::RejectReason;
using rtype::protocol::Snapshot;
using rtype::protocol::Welcome;

constexpr int OFFENCES_PER_SECOND = 10;

/*
** The server reads its rules from a file named after the current test, so
** a test can pick how many players start the game: one by default, so the
** first join starts it.
*/
class ServerFixture {
public:
    explicit ServerFixture(int playersToStart = 1):
        _rulesPath(std::filesystem::temp_directory_path() /
            ("rtype_integration_" +
                std::string(testing::UnitTest::GetInstance()
                        ->current_test_info()
                        ->name()) +
                ".json"))
    {
        std::ofstream(_rulesPath)
            << R"({"playersToStart": )" << playersToStart << "}";
        rtype::server::ServerConfig config;
        config.port      = 0;
        config.seed      = 7;
        config.rulesPath = _rulesPath;
        _app =
            std::make_unique<rtype::server::ServerApp>(config, _stopRequested);
        _thread = std::thread([this] { _result = _app->run(); });
    }

    ~ServerFixture()
    {
        if (!_stopped)
            stop();
        std::filesystem::remove(_rulesPath);
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
        _stopRequested = true;
        _thread.join();
        _stopped = true;
    }

private:
    std::filesystem::path _rulesPath;
    std::atomic<bool> _stopRequested{false};
    std::unique_ptr<rtype::server::ServerApp> _app;
    std::thread _thread;
    int _result   = -1;
    bool _stopped = false;
};

struct Joined {
    Welcome welcome;
    Snapshot snapshot;
};

std::optional<Joined> join(server_test::TestClient &client)
{
    client.send(MessageType::HELLO, Hello{.playerName = "player"});
    const auto welcome = client.waitFor<Welcome>(MessageType::WELCOME);
    EXPECT_TRUE(welcome.has_value());
    const auto snapshot = client.waitFor<Snapshot>(MessageType::SNAPSHOT);
    EXPECT_TRUE(snapshot.has_value());
    EXPECT_TRUE(client.unexpected().empty())
        << "first unexpected message: " << client.unexpected().front();
    EXPECT_EQ(client.malformed(), 0U);
    if (!welcome || !snapshot)
        return std::nullopt;
    return Joined{*welcome, *snapshot};
}

const PlayerInfo *slotOf(const Snapshot &snapshot, std::uint8_t playerId)
{
    const auto slot = std::find_if(snapshot.roster.begin(),
        snapshot.roster.end(),
        [playerId](const auto &player) { return player.playerId == playerId; });
    return slot == snapshot.roster.end() ? nullptr : &*slot;
}

bool hasEntity(const Snapshot &snapshot, std::uint32_t netId)
{
    return std::any_of(snapshot.entities.begin(), snapshot.entities.end(),
        [netId](const auto &entity) { return entity.netId == netId; });
}

/**
 * @brief Reads snapshots until one carries the roster and satisfies
 * `matches`, at most `timeout` long.
 *
 * Snapshots queued before the change being waited for are read first, so
 * a test can't just check the next one.
 */
template <typename Predicate>
std::optional<Snapshot> waitForSnapshot(server_test::TestClient &client,
    Predicate matches, std::chrono::milliseconds timeout = 1s)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;

    while (std::chrono::steady_clock::now() < deadline) {
        const auto snapshot =
            client.waitFor<Snapshot>(MessageType::SNAPSHOT, 100ms);
        if (snapshot && !snapshot->roster.empty() && matches(*snapshot))
            return snapshot;
    }
    return std::nullopt;
}

std::optional<Event> waitForPlayerLeft(server_test::TestClient &client,
    std::chrono::milliseconds timeout = 1s)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;

    while (std::chrono::steady_clock::now() < deadline) {
        const auto event = client.waitFor<Event>(MessageType::EVENT, 100ms);
        if (event && event->type == EventType::PLAYER_LEFT)
            return event;
    }
    return std::nullopt;
}

TEST(IntegrationTests, JoinReceivesWelcomeAndOwnShip)
{
    ServerFixture server;
    server_test::TestClient client(server.port());

    const auto joined = join(client);

    ASSERT_TRUE(joined.has_value());
    EXPECT_EQ(joined->welcome.playerId, 0U);
    const auto &snapshot = joined->snapshot;
    ASSERT_EQ(snapshot.roster.size(), 4U);
    const auto *const mine = slotOf(snapshot, joined->welcome.playerId);
    ASSERT_NE(mine, nullptr);
    EXPECT_EQ(mine->state, PlayerState::ALIVE);
    ASSERT_NE(mine->shipNetId, 0U);
    const auto ship = std::find_if(snapshot.entities.begin(),
        snapshot.entities.end(),
        [mine](const auto &entity) { return entity.netId == mine->shipNetId; });
    ASSERT_NE(ship, snapshot.entities.end());
    EXPECT_EQ(ship->kind, rtype::protocol::EntityKind::PLAYER_SHIP);
}

TEST(IntegrationTests, TheGameWaitsForEnoughPlayersBeforeStarting)
{
    ServerFixture server(2);
    server_test::TestClient first(server.port());
    server_test::TestClient second(server.port());

    const auto joined = join(first);

    ASSERT_TRUE(joined.has_value());
    EXPECT_EQ(joined->welcome.playersToStart, 2U);
    EXPECT_EQ(joined->snapshot.phase, rtype::protocol::GamePhase::WAITING);
    const auto *const mine = slotOf(joined->snapshot, 0);
    ASSERT_NE(mine, nullptr);
    EXPECT_EQ(mine->shipNetId, 0U) << "no ship before the game starts";
    ASSERT_TRUE(join(second).has_value());
    const auto started = waitForSnapshot(first, [](const Snapshot &snapshot) {
        const auto *const slot = slotOf(snapshot, 0);
        return snapshot.phase == rtype::protocol::GamePhase::PLAYING &&
            slot != nullptr && slot->shipNetId != 0;
    });
    EXPECT_TRUE(started.has_value());
}

TEST(IntegrationTests, TwoPlayersSeeMovement)
{
    ServerFixture server;
    server_test::TestClient first(server.port());
    server_test::TestClient second(server.port());
    ASSERT_TRUE(join(first).has_value());
    ASSERT_TRUE(join(second).has_value());

    first.send(MessageType::INPUT,
        rtype::protocol::Input{
            .inputSequence = 1, .buttons = rtype::protocol::buttons::RIGHT});
    const auto initial = second.waitFor<Snapshot>(MessageType::SNAPSHOT);
    ASSERT_TRUE(initial.has_value());
    std::vector<std::pair<std::uint32_t, float>> initialShips;
    for (const auto &entity: initial->entities) {
        if (entity.kind == rtype::protocol::EntityKind::PLAYER_SHIP)
            initialShips.emplace_back(entity.netId, entity.x);
    }
    ASSERT_FALSE(initialShips.empty());

    for (std::uint32_t sequence = 2; sequence != 62; ++sequence)
        first.send(MessageType::INPUT,
            rtype::protocol::Input{.inputSequence = sequence,
                .buttons = rtype::protocol::buttons::RIGHT});

    bool moved          = false;
    const auto deadline = std::chrono::steady_clock::now() + 2s;
    while (!moved && std::chrono::steady_clock::now() < deadline) {
        const auto snapshot =
            second.waitFor<Snapshot>(MessageType::SNAPSHOT, 100ms);
        if (snapshot) {
            for (const auto &[netId, initialX]: initialShips) {
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
        players.push_back(std::make_unique<server_test::TestClient>(server
                .port()));
        players.back()->send(MessageType::HELLO, Hello{.playerName = "player"});
        ASSERT_TRUE(players.back()
                ->waitFor<Welcome>(MessageType::WELCOME)
                .has_value())
            << "player " << i;
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
                  .version = rtype::protocol::Header::VERSION + 1,
                  .type    = MessageType::HELLO}
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
    const auto joined = join(quitter);
    ASSERT_TRUE(joined.has_value());
    ASSERT_TRUE(join(other).has_value());
    const auto quitterId   = joined->welcome.playerId;
    const auto *const slot = slotOf(joined->snapshot, quitterId);
    ASSERT_NE(slot, nullptr);
    const auto ship = slot->shipNetId;
    ASSERT_NE(ship, 0U);

    for (int copy = 0; copy != 3; ++copy)
        quitter.send(MessageType::DISCONNECT, rtype::protocol::Disconnect{});

    std::vector<Event> left;
    for (int copy = 0; copy != 3; ++copy) {
        const auto event = waitForPlayerLeft(other);
        ASSERT_TRUE(event.has_value()) << "copy " << copy;
        left.push_back(*event);
    }
    for (const auto &event: left) {
        EXPECT_EQ(event.eventId, left.front().eventId);
        EXPECT_EQ(event.a, quitterId);
        EXPECT_EQ(event.b, static_cast<std::uint32_t>(LeaveReason::QUIT));
    }
    const auto after = waitForSnapshot(other, [&](const Snapshot &snapshot) {
        const auto *const quitterSlot = slotOf(snapshot, quitterId);
        return quitterSlot != nullptr &&
            quitterSlot->state == PlayerState::EMPTY;
    });
    ASSERT_TRUE(after.has_value()) << "the slot never became EMPTY";
    EXPECT_FALSE(hasEntity(*after, ship));
}

TEST(IntegrationTests, SlowClientTimesOut)
{
    ServerFixture server;
    server_test::TestClient silent(server.port());
    server_test::TestClient other(server.port());
    silent.send(MessageType::HELLO, Hello{.playerName = "silent"});
    const auto welcome = silent.waitFor<Welcome>(MessageType::WELCOME);
    ASSERT_TRUE(welcome.has_value());
    ASSERT_TRUE(join(other).has_value());
    std::uint32_t inputSequence = 0;
    const auto keepAlive        = [&other, &inputSequence] {
        other.send(MessageType::INPUT,
            Input{.inputSequence = ++inputSequence, .buttons = 0});
    };

    std::optional<Event> left;
    const auto deadline = std::chrono::steady_clock::now() + 7s;
    while (!left && std::chrono::steady_clock::now() < deadline) {
        keepAlive();
        const auto event = other.waitFor<Event>(MessageType::EVENT, 100ms);
        if (event && event->type == EventType::PLAYER_LEFT)
            left = event;
    }

    ASSERT_TRUE(left.has_value());
    EXPECT_EQ(left->a, welcome->playerId);
    EXPECT_EQ(left->b, static_cast<std::uint32_t>(LeaveReason::TIMEOUT));
    std::optional<Snapshot> snapshot;
    for (int attempt = 0;
        attempt != 10 && (!snapshot || snapshot->roster.empty()); ++attempt) {
        keepAlive();
        snapshot = other.waitFor<Snapshot>(MessageType::SNAPSHOT, 200ms);
    }
    ASSERT_TRUE(snapshot.has_value());
    ASSERT_FALSE(snapshot->roster.empty());
    for (const auto &player: snapshot->roster) {
        if (player.playerId == welcome->playerId) {
            EXPECT_EQ(player.state, PlayerState::EMPTY);
        } else if (player.name == "player") {
            EXPECT_NE(player.state, PlayerState::EMPTY);
        }
    }
}

TEST(IntegrationTests, MalformedTrafficKicksOnlyTheSender)
{
    ServerFixture server;
    server_test::TestClient bad(server.port());
    server_test::TestClient other(server.port());
    const auto joined = join(bad);
    ASSERT_TRUE(joined.has_value());
    ASSERT_TRUE(join(other).has_value());

    const std::array<std::uint8_t, 1> garbage{0};
    for (int copy = 0; copy != 20; ++copy)
        bad.sendRaw(garbage);

    const auto kick = bad.waitFor<Kick>(MessageType::KICK);
    ASSERT_TRUE(kick.has_value());
    EXPECT_EQ(kick->reason, KickReason::PROTOCOL_ERROR);
    const auto left = waitForPlayerLeft(other);
    ASSERT_TRUE(left.has_value());
    EXPECT_EQ(left->a, joined->welcome.playerId);
    EXPECT_EQ(left->b, static_cast<std::uint32_t>(LeaveReason::KICKED));
    server_test::TestClient newcomer(server.port());
    EXPECT_TRUE(join(newcomer).has_value()) << "the server stopped answering";
    EXPECT_TRUE(other.waitFor<Snapshot>(MessageType::SNAPSHOT).has_value());
}

/*
** A server-to-client message sent by a client is dropped and counted as one
** offence of its session: 10 per second are tolerated, the 11th kicks
** (Session::offences). Reading the adapter's stats would race with run().
*/
TEST(IntegrationTests, WrongDirectionIsDropped)
{
    ServerFixture server;
    server_test::TestClient client(server.port());
    ASSERT_TRUE(join(client).has_value());

    for (int copy = 0; copy != OFFENCES_PER_SECOND; ++copy)
        client.send(MessageType::SHUTDOWN, rtype::protocol::Shutdown{});

    EXPECT_FALSE(client.waitFor<Kick>(MessageType::KICK, 200ms).has_value());
    EXPECT_TRUE(client.waitFor<Snapshot>(MessageType::SNAPSHOT).has_value());
    client.send(MessageType::SHUTDOWN, rtype::protocol::Shutdown{});
    const auto kick = client.waitFor<Kick>(MessageType::KICK);
    ASSERT_TRUE(kick.has_value()) << "the wrong-direction messages were not "
                                     "counted as offences";
    EXPECT_EQ(kick->reason, KickReason::PROTOCOL_ERROR);
}

TEST(IntegrationTests, ShutdownStopsServer)
{
    ServerFixture server;
    server_test::TestClient first(server.port());
    server_test::TestClient second(server.port());
    ASSERT_TRUE(join(first).has_value());
    ASSERT_TRUE(join(second).has_value());

    server.stop();
    EXPECT_TRUE(first.waitFor<rtype::protocol::Shutdown>(MessageType::SHUTDOWN)
            .has_value());
    EXPECT_TRUE(second.waitFor<rtype::protocol::Shutdown>(MessageType::SHUTDOWN)
            .has_value());
    EXPECT_EQ(server.result(), 0);
}

} // namespace
