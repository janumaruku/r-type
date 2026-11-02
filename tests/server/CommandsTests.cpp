#include <gtest/gtest.h>

#include "ServerTestHelpers.hpp"

namespace {

namespace game = rtype::game;
namespace proto = rtype::protocol;
using server_test::Fixture;
using server_test::datagram;
using server_test::endpointFor;

class ThrowingCommand final: public rtype::server::IServerCommand {
public:
    static std::unique_ptr<rtype::server::IServerCommand> create()
    {
        return std::make_unique<ThrowingCommand>();
    }

    bool execute(rtype::server::ServerNetworkAdapter &,
        rtype::server::IncomingMessage &) override
    {
        throw std::runtime_error("test command failure");
    }

    bool operator()(rtype::server::ServerNetworkAdapter &adapter,
        rtype::server::IncomingMessage &message) override
    {
        return execute(adapter, message);
    }
};

TEST(CommandsTests, InputIsAppliedToAJoinedPlayer)
{
    Fixture fixture;
    const auto from = endpointFor(20);
    fixture.send(datagram(from, proto::MessageType::HELLO,
        proto::Hello{.playerName = "Player"}));
    fixture.world.step(0.016F);
    std::vector<game::ReplicatedEntity> before;
    fixture.world.collectReplicated(before);
    ASSERT_FALSE(before.empty());
    fixture.send(datagram(from, proto::MessageType::INPUT,
        proto::Input{.inputSequence = 1, .buttons = proto::buttons::RIGHT}));
    fixture.world.step(0.016F);
    std::vector<game::ReplicatedEntity> entities;
    fixture.world.collectReplicated(entities);
    ASSERT_FALSE(entities.empty());
    EXPECT_GT(entities.front().position.x(), before.front().position.x());
}

TEST(CommandsTests, UnknownDisconnectIsIgnored)
{
    Fixture fixture;
    fixture.send(datagram(endpointFor(21), proto::MessageType::DISCONNECT,
        proto::Disconnect{}));
    EXPECT_EQ(fixture.sessions.size(), 0U);
}

TEST(CommandsTests, UnexpectedCommandExceptionKicksSessionWithoutEscaping)
{
    net::UdpSocket socket(0);
    socket.start();
    game::GameWorld world(game::Rules{}, 1);
    rtype::server::SessionManager sessions;
    rtype::server::ServerCommandFactory commands;
    commands.registerCreator<ThrowingCommand>(proto::MessageType::INPUT);
    rtype::server::ServerNetworkAdapter adapter(socket, world, sessions,
        &commands);
    const auto from = endpointFor(22);
    ASSERT_EQ(world.reserveSlot(), 0);
    ASSERT_TRUE(world.roster().get(0) != nullptr);
    sessions.add(from, 0, "Player", {});

    adapter.handle(datagram(from, proto::MessageType::INPUT,
        proto::Input{}), {});

    EXPECT_EQ(sessions.find(from), nullptr);
    EXPECT_EQ(adapter.stats().totalKicked, 1U);
    socket.stop();
}

}
