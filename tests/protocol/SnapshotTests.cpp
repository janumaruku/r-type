/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SnapshotTests
*/

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "MessageTestHelpers.hpp"
#include "rtype/protocol/Header.hpp"
#include "rtype/protocol/Messages.hpp"
#include "rtype/protocol/Snapshot.hpp"

namespace proto = rtype::protocol;
using protocol_test::encode;
using protocol_test::expectEveryPrefixTruncated;
using protocol_test::expectOutOfRange;
using protocol_test::expectRoundTrip;

namespace {

const std::string SIXTEEN_BYTES = "ABCDEFGHIJKLMNOP";

/*
** One player per state a joined player can be in, each with a 16-byte name:
** the largest roster the RFC allows.
*/
std::vector<proto::PlayerInfo> fullRoster()
{
    return {
        proto::PlayerInfo{.playerId = 0, .state = proto::PlayerState::ALIVE,
            .lives = 3,
            .score = 0xFFFFFFFF, .shipNetId = 101, .name = SIXTEEN_BYTES},
        proto::PlayerInfo{.playerId = 1,
            .state = proto::PlayerState::RESPAWNING, .lives = 2,
            .score = 1500, .name = SIXTEEN_BYTES},
        proto::PlayerInfo{.playerId = 2, .state = proto::PlayerState::OUT,
            .lives = 0, .score = 42, .name = SIXTEEN_BYTES},
        proto::PlayerInfo{.playerId = 3, .state = proto::PlayerState::ALIVE,
            .lives = 1, .score = 7, .shipNetId = 104, .name = SIXTEEN_BYTES},
    };
}

std::vector<proto::PlayerInfo> rosterWithIds(const std::vector<std::uint8_t>
        &ids)
{
    std::vector<proto::PlayerInfo> roster;

    for (const std::uint8_t id: ids)
        roster.push_back(proto::PlayerInfo{.playerId = id});
    return roster;
}

/*
** Cycles through the 4 kinds, each with a variant valid for it.
*/
proto::EntityState entityOfEveryKind(std::uint32_t netId)
{
    switch (netId % 4) {
    case 0:
        return proto::EntityState{.netId = netId,
            .kind = proto::EntityKind::PLAYER_SHIP,
            .variant = 3, .x = 1280.0F, .y = 720.0F, .vx = -1.5F, .vy = 2.5F};
    case 1:
        return proto::EntityState{.netId = netId,
            .kind = proto::EntityKind::BYDOS_SLAVE,
            .variant = proto::pattern::SINE, .x = 640.0F, .y = 360.0F,
                .vx = -120.0F};
    case 2:
        return proto::EntityState{.netId = netId,
            .kind = proto::EntityKind::PLAYER_MISSILE,
            .x = 10.0F, .y = 20.0F, .vx = 600.0F};
    default:
        return proto::EntityState{.netId = netId,
            .kind = proto::EntityKind::ENEMY_MISSILE,
            .x = 0.0F, .y = 0.0F, .vx = -300.0F, .vy = 50.0F};
    }
}

std::vector<proto::EntityState> entities(std::size_t count)
{
    std::vector<proto::EntityState> result;

    for (std::size_t index = 0; index < count; ++index)
        result.push_back(entityOfEveryKind(static_cast<std::uint32_t>(index +
            1)));
    return result;
}

proto::Snapshot fullChunk()
{
    return proto::Snapshot{.serverTick = 0xFFFFFFFF, .lastProcessedInput = 812,
        .chunkIndex = 0, .chunkCount = 1, .roster = fullRoster(),
        .entities = entities(proto::MAX_ENTITIES_PER_CHUNK)};
}

}

TEST(SnapshotTests, AFullChunkRoundTripsAndFitsInOneDatagram)
{
    const proto::Snapshot chunk = fullChunk();

    expectRoundTrip(chunk);
    EXPECT_EQ(proto::makePacket(proto::MessageType::SNAPSHOT, 1, chunk).size(),
        1190U);
}

TEST(SnapshotTests, AnEmptyLaterChunkIsTheSmallestPayload)
{
    const proto::Snapshot chunk{.serverTick = 9, .chunkIndex = 1,
        .chunkCount = 2};

    expectRoundTrip(chunk);
    EXPECT_EQ(encode(chunk).size(), 14U);
}

TEST(SnapshotTests, AnEmptySlotRoundTrips)
{
    expectRoundTrip(proto::PlayerInfo{.playerId = 2,
        .state = proto::PlayerState::EMPTY});
}

TEST(SnapshotTests, EveryPrefixOfAPayloadIsTruncated)
{
    {
        SCOPED_TRACE("Snapshot");
        expectEveryPrefixTruncated(proto::Snapshot{.roster = fullRoster(),
            .entities = entities(4)});
    }
    {
        SCOPED_TRACE("PlayerInfo");
        expectEveryPrefixTruncated(fullRoster()[0]);
    }
    {
        SCOPED_TRACE("EntityState");
        expectEveryPrefixTruncated(entityOfEveryKind(1));
    }
}

TEST(SnapshotTests, APlayerIdOfFourIsRejected)
{
    expectOutOfRange(proto::PlayerInfo{.playerId = 4});
}

TEST(SnapshotTests, AnUnknownPlayerStateIsRejected)
{
    expectOutOfRange(proto::PlayerInfo{
        .state = static_cast<proto::PlayerState>(4)});
}

TEST(SnapshotTests, MoreThanThreeLivesIsRejected)
{
    expectOutOfRange(proto::PlayerInfo{.state = proto::PlayerState::OUT,
        .lives = 4});
}

TEST(SnapshotTests, ANameLongerThanSixteenBytesIsRejected)
{
    expectOutOfRange(proto::PlayerInfo{.state = proto::PlayerState::ALIVE,
        .name = SIXTEEN_BYTES + "Q"});
}

TEST(SnapshotTests, AnEmptySlotWithANameIsRejected)
{
    expectOutOfRange(proto::PlayerInfo{.state = proto::PlayerState::EMPTY,
        .name = "Ghost"});
}

TEST(SnapshotTests, AShipNetIdOnAPlayerThatIsNotAliveIsRejected)
{
    for (const proto::PlayerState state: {proto::PlayerState::EMPTY,
             proto::PlayerState::RESPAWNING, proto::PlayerState::OUT}) {
        SCOPED_TRACE("state " + std::to_string(static_cast<int>(state)));
        expectOutOfRange(proto::PlayerInfo{.state = state, .shipNetId = 101});
    }
}

TEST(SnapshotTests, ANetIdOfZeroIsRejected)
{
    expectOutOfRange(proto::EntityState{.netId = 0});
}

TEST(SnapshotTests, AnUnknownEntityKindIsRejected)
{
    expectOutOfRange(proto::EntityState{.netId = 1,
        .kind = static_cast<proto::EntityKind>(0)});
    expectOutOfRange(proto::EntityState{.netId = 1,
        .kind = static_cast<proto::EntityKind>(5)});
}

TEST(SnapshotTests, APlayerShipOfAFifthPlayerIsRejected)
{
    expectOutOfRange(proto::EntityState{.netId = 1,
        .kind = proto::EntityKind::PLAYER_SHIP, .variant = 4});
}

TEST(SnapshotTests, AnUnknownBydosPatternIsRejected)
{
    expectOutOfRange(proto::EntityState{.netId = 1,
        .kind = proto::EntityKind::BYDOS_SLAVE, .variant = 2});
}

TEST(SnapshotTests, AMissileWithAVariantIsRejected)
{
    expectOutOfRange(proto::EntityState{.netId = 1,
        .kind = proto::EntityKind::PLAYER_MISSILE, .variant = 1});
    expectOutOfRange(proto::EntityState{.netId = 1,
        .kind = proto::EntityKind::ENEMY_MISSILE, .variant = 1});
}

TEST(SnapshotTests, AChunkCountOfZeroOrAboveSixteenIsRejected)
{
    expectOutOfRange(proto::Snapshot{.chunkCount = 0, .roster = fullRoster()});
    expectOutOfRange(proto::Snapshot{.chunkCount = 17, .roster = fullRoster()});
}

TEST(SnapshotTests, AChunkIndexPastTheCountIsRejected)
{
    expectOutOfRange(proto::Snapshot{.chunkIndex = 2, .chunkCount = 2});
}

TEST(SnapshotTests, ARosterOfThreeIsRejected)
{
    expectOutOfRange(proto::Snapshot{.roster = rosterWithIds({0, 1, 2})});
}

TEST(SnapshotTests, AFirstChunkWithoutARosterIsRejected)
{
    expectOutOfRange(proto::Snapshot{.chunkIndex = 0, .chunkCount = 2});
}

TEST(SnapshotTests, ALaterChunkWithARosterIsRejected)
{
    expectOutOfRange(proto::Snapshot{.chunkIndex = 1, .chunkCount = 2,
        .roster = fullRoster()});
}

/*
** The roster has empty names so that 49 entities still fit in 1200 bytes, and
** reach the decoder instead of being refused by Packet when they are written.
*/
TEST(SnapshotTests, MoreThanFortyEightEntitiesIsRejected)
{
    expectOutOfRange(proto::Snapshot{.roster = rosterWithIds({0, 1, 2, 3}),
        .entities = entities(proto::MAX_ENTITIES_PER_CHUNK + 1)});
}

TEST(SnapshotTests, ARosterOutOfSlotOrderIsRejected)
{
    expectOutOfRange(proto::Snapshot{.roster = rosterWithIds({1, 0, 2, 3})});
    expectOutOfRange(proto::Snapshot{.roster = rosterWithIds({0, 0, 1, 3})});
}
