/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SplitSnapshotTests
*/

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "MessageTestHelpers.hpp"
#include "engine/core/Errors.hpp"
#include "rtype/protocol/Snapshot.hpp"

namespace proto = rtype::protocol;
using protocol_test::expectRoundTrip;

namespace {

/*
** chunkIndex and chunkCount are set to nonsense, since splitSnapshot must
** ignore them.
*/
proto::Snapshot wholeTick(std::size_t entityCount)
{
    proto::Snapshot full{.serverTick = 1234, .lastProcessedInput = 812,
        .chunkIndex = 5, .chunkCount = 3};

    for (std::uint8_t slot = 0; slot < 4; ++slot)
        full.roster.push_back(proto::PlayerInfo{.playerId = slot,
            .state = proto::PlayerState::ALIVE, .lives = 3,
                .shipNetId = 900U + slot,
            .name = "Player " + std::to_string(slot + 1)});
    for (std::size_t index = 0; index < entityCount; ++index)
        full.entities.push_back(proto::EntityState{
            .netId = static_cast<std::uint32_t>(index + 1),
            .kind = proto::EntityKind::BYDOS_SLAVE,
                .x = static_cast<float>(index)});
    return full;
}

}

TEST(SplitSnapshotTests, ATickIsSplitIntoChunksThatEachDecodeAlone)
{
    const std::pair<std::size_t, std::size_t> cases[]{
        {0, 1}, {1, 1}, {48, 1}, {49, 2}, {94, 2}, {768, 16},
    };

    for (const auto &[entityCount, expectedChunks]: cases) {
        SCOPED_TRACE(std::to_string(entityCount) + " entities");
        const proto::Snapshot full = wholeTick(entityCount);
        const std::vector<proto::Snapshot> chunks = proto::splitSnapshot(full);
        std::vector<proto::EntityState> reassembled;

        ASSERT_EQ(chunks.size(), expectedChunks);
        for (std::size_t index = 0; index < chunks.size(); ++index) {
            const proto::Snapshot &chunk = chunks[index];

            EXPECT_EQ(chunk.serverTick, full.serverTick);
            EXPECT_EQ(chunk.lastProcessedInput, full.lastProcessedInput);
            EXPECT_EQ(chunk.chunkIndex, index);
            EXPECT_EQ(chunk.chunkCount, expectedChunks);
            if (index == 0)
                EXPECT_EQ(chunk.roster, full.roster);
            else
                EXPECT_TRUE(chunk.roster.empty());
            EXPECT_LE(chunk.entities.size(), proto::MAX_ENTITIES_PER_CHUNK);
            expectRoundTrip(chunk);
            reassembled.insert(reassembled.end(), chunk.entities.begin(),
                chunk.entities.end());
        }
        EXPECT_EQ(reassembled, full.entities);
    }
}

TEST(SplitSnapshotTests, MoreThanSixteenChunksOfEntitiesIsAServerBug)
{
    EXPECT_THROW(static_cast<void>(proto::splitSnapshot(wholeTick(769))),
        engine::core::EngineError);
}
