#include <gtest/gtest.h>

#include "SnapshotBuilder.hpp"
#include "ServerTestHelpers.hpp"
#include "rtype/game/Components.hpp"
#include "rtype/protocol/Snapshot.hpp"

namespace rtype::game {
struct GameWorldTestAccess {
    static void addEntity(GameWorld &world, NetId netId, EntityKind kind,
        std::uint8_t variant = 0)
    {
        const auto entity = world._registry.spawnEntity();
        world._registry.addComponent(entity,
            Replicated{.netId = netId, .kind = kind, .variant = variant});
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
    const auto &chunks = builder.build(world);

    ASSERT_EQ(chunks.size(), 1U);
    EXPECT_EQ(chunks.front().roster.size(), 4U);
    EXPECT_EQ(chunks.front().entities.size(), 5U);
    const auto packet = rtype::protocol::makePacket(
        rtype::protocol::MessageType::SNAPSHOT, 0, chunks.front());
    EXPECT_NO_THROW((void)server_test::readMessage<rtype::protocol::Snapshot>(
        {std::vector<std::uint8_t>(packet.bytes().begin(), packet.bytes().end()),
            server_test::endpointFor(1)}));
}

TEST(SnapshotBuilderTests, KeepsEachEntitysVariant)
{
    GameWorld world(rtype::game::Rules{}, 1);

    for (std::uint8_t variant = 0; variant != 4; ++variant)
        rtype::game::GameWorldTestAccess::addEntity(world, variant + 1U,
            EntityKind::PLAYER_SHIP, variant);
    rtype::game::GameWorldTestAccess::addEntity(world, 5,
        EntityKind::BYDOS_SLAVE, 1);

    SnapshotBuilder builder;
    const auto &chunks = builder.build(world);

    ASSERT_EQ(chunks.size(), 1U);
    ASSERT_EQ(chunks.front().entities.size(), 5U);
    for (const auto &entity : chunks.front().entities) {
        const std::uint8_t expected = entity.netId == 5
            ? 1
            : static_cast<std::uint8_t>(entity.netId - 1);

        EXPECT_EQ(entity.variant, expected) << "netId " << entity.netId;
    }
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
    for (std::uint32_t id = 1; id <= 60; ++id)
        rtype::game::GameWorldTestAccess::addEntity(
            world, id, EntityKind::BYDOS_SLAVE);

    adapter.sendSnapshots();

    const auto firstPackets = server_test::receive(first, 2);
    const auto secondPackets = server_test::receive(second, 2);
    ASSERT_EQ(firstPackets.size(), 2U);
    ASSERT_EQ(secondPackets.size(), 2U);
    for (std::size_t chunk = 0; chunk != 2; ++chunk) {
        const auto mine = server_test::readMessage<rtype::protocol::Snapshot>(
            firstPackets[chunk]);
        const auto theirs = server_test::readMessage<
            rtype::protocol::Snapshot>(secondPackets[chunk]);

        EXPECT_EQ(mine.lastProcessedInput, 10U) << "chunk " << chunk;
        EXPECT_EQ(theirs.lastProcessedInput, 20U) << "chunk " << chunk;
        EXPECT_EQ(mine.serverTick, theirs.serverTick);
        EXPECT_EQ(mine.entities, theirs.entities);
    }
    socket.stop();
}

}
