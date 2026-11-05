#include <gtest/gtest.h>

#include "SnapshotAssembler.hpp"

namespace rtype::client {
namespace {

protocol::Snapshot chunk(std::uint32_t tick, std::uint8_t index,
    std::uint8_t count = 1)
{
    protocol::Snapshot result;
    result.serverTick = tick;
    result.chunkIndex = index;
    result.chunkCount = count;
    return result;
}

TEST(SnapshotAssembler, CompletesSingleChunkTick)
{
    SnapshotAssembler assembler;
    EXPECT_EQ(assembler.add(chunk(10, 0)), 10U);
}

TEST(SnapshotAssembler, CompletesOnlyAfterEachChunkArrivesOnce)
{
    SnapshotAssembler assembler;
    EXPECT_EQ(assembler.add(chunk(20, 1, 2)), std::nullopt);
    EXPECT_EQ(assembler.add(chunk(20, 0, 2)), 20U);
}

TEST(SnapshotAssembler, DuplicateChunkDoesNotCompleteTick)
{
    SnapshotAssembler assembler;
    EXPECT_EQ(assembler.add(chunk(20, 0, 2)), std::nullopt);
    EXPECT_EQ(assembler.add(chunk(20, 0, 2)), std::nullopt);
    EXPECT_EQ(assembler.add(chunk(20, 1, 2)), 20U);
}

TEST(SnapshotAssembler, NewerCompleteTickMakesOlderTickStale)
{
    SnapshotAssembler assembler;
    EXPECT_EQ(assembler.add(chunk(30, 0, 2)), std::nullopt);
    EXPECT_EQ(assembler.add(chunk(32, 0)), 32U);
    EXPECT_TRUE(assembler.isStale(31));
    EXPECT_EQ(assembler.add(chunk(30, 1, 2)), std::nullopt);
}

TEST(SnapshotAssembler, TracksAtMostEightIncompleteTicks)
{
    SnapshotAssembler assembler;
    for (std::uint32_t tick = 1; tick <= 10; ++tick)
        EXPECT_EQ(assembler.add(chunk(tick, 0, 2)), std::nullopt);
    EXPECT_EQ(assembler.add(chunk(1, 1, 2)), std::nullopt);
    EXPECT_EQ(assembler.add(chunk(10, 1, 2)), 10U);
}

} // namespace
} // namespace rtype::client
