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

using namespace rtype::protocol;
using namespace protocol_test;

namespace {

const std::string SIXTEEN_BYTES = "ABCDEFGHIJKLMNOP";

/*
** One player per state a joined player can be in, each with a 16-byte name:
** the largest roster the RFC allows.
*/
std::vector<PlayerInfo> fullRoster()
{
    return {
        PlayerInfo{.playerId = 0, .state = PlayerState::ALIVE, .lives = 3,
            .score = 0xFFFFFFFF, .shipNetId = 101, .name = SIXTEEN_BYTES},
        PlayerInfo{.playerId = 1, .state = PlayerState::RESPAWNING, .lives = 2,
            .score = 1500, .name = SIXTEEN_BYTES},
        PlayerInfo{.playerId = 2, .state = PlayerState::OUT, .lives = 0,
            .score = 42, .name = SIXTEEN_BYTES},
        PlayerInfo{.playerId = 3, .state = PlayerState::ALIVE, .lives = 1,
            .score = 7, .shipNetId = 104, .name = SIXTEEN_BYTES},
    };
}

std::vector<PlayerInfo> rosterWithIds(const std::vector<std::uint8_t> &ids)
{
    std::vector<PlayerInfo> roster;

    for (const std::uint8_t id: ids)
        roster.push_back(PlayerInfo{.playerId = id});
    return roster;
}

/*
** Cycles through the 4 kinds, each with a variant valid for it.
*/
EntityState entityOfEveryKind(std::uint32_t netId)
{
    switch (netId % 4) {
    case 0:
        return EntityState{.netId = netId, .kind = EntityKind::PLAYER_SHIP,
            .variant = 3, .x = 1280.0F, .y = 720.0F, .vx = -1.5F, .vy = 2.5F};
    case 1:
        return EntityState{.netId = netId, .kind = EntityKind::BYDOS_SLAVE,
            .variant = pattern::SINE, .x = 640.0F, .y = 360.0F, .vx = -120.0F};
    case 2:
        return EntityState{.netId = netId, .kind = EntityKind::PLAYER_MISSILE,
            .x = 10.0F, .y = 20.0F, .vx = 600.0F};
    default:
        return EntityState{.netId = netId, .kind = EntityKind::ENEMY_MISSILE,
            .x = 0.0F, .y = 0.0F, .vx = -300.0F, .vy = 50.0F};
    }
}

std::vector<EntityState> entities(std::size_t count)
{
    std::vector<EntityState> result;

    for (std::size_t index = 0; index < count; ++index)
        result.push_back(entityOfEveryKind(static_cast<std::uint32_t>(index + 1)));
    return result;
}

Snapshot fullChunk()
{
    return Snapshot{.serverTick = 0xFFFFFFFF, .lastProcessedInput = 812,
        .chunkIndex = 0, .chunkCount = 1, .roster = fullRoster(),
        .entities = entities(MAX_ENTITIES_PER_CHUNK)};
}

}

TEST(SnapshotTests, AFullChunkRoundTripsAndFitsInOneDatagram)
{
    const Snapshot chunk = fullChunk();

    expectRoundTrip(chunk);
    EXPECT_EQ(makePacket(MessageType::SNAPSHOT, 1, chunk).size(), 1190U);
}

TEST(SnapshotTests, AnEmptyLaterChunkIsTheSmallestPayload)
{
    const Snapshot chunk{.serverTick = 9, .chunkIndex = 1, .chunkCount = 2};

    expectRoundTrip(chunk);
    EXPECT_EQ(encode(chunk).size(), 14U);
}

TEST(SnapshotTests, AnEmptySlotRoundTrips)
{
    expectRoundTrip(PlayerInfo{.playerId = 2, .state = PlayerState::EMPTY});
}

TEST(SnapshotTests, EveryPrefixOfAPayloadIsTruncated)
{
    {
        SCOPED_TRACE("Snapshot");
        expectEveryPrefixTruncated(Snapshot{.roster = fullRoster(),
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
    expectOutOfRange(PlayerInfo{.playerId = 4});
}

TEST(SnapshotTests, AnUnknownPlayerStateIsRejected)
{
    expectOutOfRange(PlayerInfo{.state = static_cast<PlayerState>(4)});
}

TEST(SnapshotTests, MoreThanThreeLivesIsRejected)
{
    expectOutOfRange(PlayerInfo{.state = PlayerState::OUT, .lives = 4});
}

TEST(SnapshotTests, ANameLongerThanSixteenBytesIsRejected)
{
    expectOutOfRange(PlayerInfo{.state = PlayerState::ALIVE,
        .name = SIXTEEN_BYTES + "Q"});
}

TEST(SnapshotTests, AnEmptySlotWithANameIsRejected)
{
    expectOutOfRange(PlayerInfo{.state = PlayerState::EMPTY, .name = "Ghost"});
}

TEST(SnapshotTests, AShipNetIdOnAPlayerThatIsNotAliveIsRejected)
{
    for (const PlayerState state: {PlayerState::EMPTY, PlayerState::RESPAWNING,
             PlayerState::OUT}) {
        SCOPED_TRACE("state " + std::to_string(static_cast<int>(state)));
        expectOutOfRange(PlayerInfo{.state = state, .shipNetId = 101});
    }
}

TEST(SnapshotTests, ANetIdOfZeroIsRejected)
{
    expectOutOfRange(EntityState{.netId = 0});
}

TEST(SnapshotTests, AnUnknownEntityKindIsRejected)
{
    expectOutOfRange(EntityState{.netId = 1, .kind = static_cast<EntityKind>(0)});
    expectOutOfRange(EntityState{.netId = 1, .kind = static_cast<EntityKind>(5)});
}

TEST(SnapshotTests, APlayerShipOfAFifthPlayerIsRejected)
{
    expectOutOfRange(EntityState{.netId = 1, .kind = EntityKind::PLAYER_SHIP,
        .variant = 4});
}

TEST(SnapshotTests, AnUnknownBydosPatternIsRejected)
{
    expectOutOfRange(EntityState{.netId = 1, .kind = EntityKind::BYDOS_SLAVE,
        .variant = 2});
}

TEST(SnapshotTests, AMissileWithAVariantIsRejected)
{
    expectOutOfRange(EntityState{.netId = 1, .kind = EntityKind::PLAYER_MISSILE,
        .variant = 1});
    expectOutOfRange(EntityState{.netId = 1, .kind = EntityKind::ENEMY_MISSILE,
        .variant = 1});
}

TEST(SnapshotTests, AChunkCountOfZeroOrAboveSixteenIsRejected)
{
    expectOutOfRange(Snapshot{.chunkCount = 0, .roster = fullRoster()});
    expectOutOfRange(Snapshot{.chunkCount = 17, .roster = fullRoster()});
}

TEST(SnapshotTests, AChunkIndexPastTheCountIsRejected)
{
    expectOutOfRange(Snapshot{.chunkIndex = 2, .chunkCount = 2});
}

TEST(SnapshotTests, ARosterOfThreeIsRejected)
{
    expectOutOfRange(Snapshot{.roster = rosterWithIds({0, 1, 2})});
}

TEST(SnapshotTests, AFirstChunkWithoutARosterIsRejected)
{
    expectOutOfRange(Snapshot{.chunkIndex = 0, .chunkCount = 2});
}

TEST(SnapshotTests, ALaterChunkWithARosterIsRejected)
{
    expectOutOfRange(Snapshot{.chunkIndex = 1, .chunkCount = 2,
        .roster = fullRoster()});
}

/*
** The roster has empty names so that 49 entities still fit in 1200 bytes, and
** reach the decoder instead of being refused by Packet when they are written.
*/
TEST(SnapshotTests, MoreThanFortyEightEntitiesIsRejected)
{
    expectOutOfRange(Snapshot{.roster = rosterWithIds({0, 1, 2, 3}),
        .entities = entities(MAX_ENTITIES_PER_CHUNK + 1)});
}

TEST(SnapshotTests, ARosterOutOfSlotOrderIsRejected)
{
    expectOutOfRange(Snapshot{.roster = rosterWithIds({1, 0, 2, 3})});
    expectOutOfRange(Snapshot{.roster = rosterWithIds({0, 0, 1, 3})});
}
