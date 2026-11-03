#include <gtest/gtest.h>

#include "SnapshotBuilder.hpp"
#include "ServerTestHelpers.hpp"
#include "rtype/game/Components.hpp"
#include "rtype/protocol/Snapshot.hpp"

namespace rtype::game {
struct GameWorldTestAccess {
    static void addEntity(GameWorld &world, NetId netId, EntityKind kind)
    {
        const auto entity = world._registry.spawnEntity();
        world._registry.addComponent(entity,
            Replicated{.netId = netId, .kind = kind});
        world._registry.addComponent(entity,
            Transform{.position = engine::core::Vec2f{}});
        world._registry.addComponent(entity,
            Velocity{.value = engine::core::Vec2f{}});
    }
};
}

namespace {

using rtype::game::EntityKind;
using rtype::game::GameWorld;
using rtype::server::SnapshotBuilder;

TEST(SnapshotBuilderTests, BuildsRosterAndEntitiesThatDeserialize)
{
    GameWorld world(rtype::game::Rules{}, 1);
    auto first = world.reserveSlot();
    auto second = world.reserveSlot();
    ASSERT_TRUE(first.has_value());
    ASSERT_TRUE(second.has_value());
    world.apply(rtype::game::JoinPlayer{*first, "one"});
    world.apply(rtype::game::JoinPlayer{*second, "two"});
    for (std::uint32_t id = 1; id <= 2; ++id)
        rtype::game::GameWorldTestAccess::addEntity(
            world, id, EntityKind::PLAYER_SHIP);
    for (std::uint32_t id = 3; id <= 5; ++id)
        rtype::game::GameWorldTestAccess::addEntity(
            world, id, EntityKind::BYDOS_SLAVE);

    SnapshotBuilder builder;
    const auto &chunks = builder.build(world, 10);

    ASSERT_EQ(chunks.size(), 1U);
    EXPECT_EQ(chunks.front().roster.size(), 4U);
    EXPECT_EQ(chunks.front().entities.size(), 5U);
    const auto packet = rtype::protocol::makePacket(
        rtype::protocol::MessageType::SNAPSHOT, 0, chunks.front());
    EXPECT_NO_THROW((void)server_test::readMessage<rtype::protocol::Snapshot>(
        {std::vector<std::uint8_t>(packet.bytes().begin(), packet.bytes().end()),
            server_test::endpointFor(1)}));
}

TEST(SnapshotBuilderTests, SplitsNinetyFourEntitiesAndKeepsRosterInFirstChunk)
{
    GameWorld world(rtype::game::Rules{}, 1);
    for (std::uint32_t id = 1; id <= 94; ++id)
        rtype::game::GameWorldTestAccess::addEntity(
            world, id, EntityKind::BYDOS_SLAVE);

    SnapshotBuilder builder;
    const auto &chunks = builder.build(world);

    ASSERT_EQ(chunks.size(), 2U);
    EXPECT_EQ(chunks[0].roster.size(), 4U);
    EXPECT_TRUE(chunks[1].roster.empty());
    EXPECT_EQ(chunks[0].entities.size(), 48U);
    EXPECT_EQ(chunks[1].entities.size(), 46U);
}

TEST(SnapshotBuilderTests, SendSnapshotsUsesEachSessionsProcessedInput)
{
    net::UdpSocket socket(0);
    socket.start();
    GameWorld world(rtype::game::Rules{}, 1);
    rtype::server::SessionManager sessions;
    rtype::server::ServerNetworkAdapter adapter(socket, world, sessions);
    asio::io_context io;
    asio::ip::udp::socket first(io, {asio::ip::address_v4::loopback(), 0});
    asio::ip::udp::socket second(io, {asio::ip::address_v4::loopback(), 0});
    auto &firstSession = sessions.add(first.local_endpoint(), 0, "one", {});
    auto &secondSession = sessions.add(second.local_endpoint(), 1, "two", {});
    firstSession.lastProcessedInput = 10;
    secondSession.lastProcessedInput = 20;

    adapter.sendSnapshots();

    const auto firstPackets = server_test::receive(first, 1);
    const auto secondPackets = server_test::receive(second, 1);
    ASSERT_EQ(firstPackets.size(), 1U);
    ASSERT_EQ(secondPackets.size(), 1U);
    EXPECT_EQ(server_test::readMessage<rtype::protocol::Snapshot>(
        firstPackets.front()).lastProcessedInput, 10U);
    EXPECT_EQ(server_test::readMessage<rtype::protocol::Snapshot>(
        secondPackets.front()).lastProcessedInput, 20U);
    socket.stop();
}

}
