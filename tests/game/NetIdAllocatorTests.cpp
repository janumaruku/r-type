/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** NetIdAllocatorTests
*/

#include <gtest/gtest.h>

#include <vector>

#include "engine/ecs/Registry.hpp"
#include "rtype/game/NetIdAllocator.hpp"

using engine::ecs::Entity;
using engine::ecs::Registry;
using rtype::game::NetIdAllocator;

TEST(NetIdAllocator, AllocatesFromOneUpwards)
{
    Registry registry;
    NetIdAllocator allocator;

    EXPECT_EQ(allocator.allocate(registry.spawnEntity()), 1U);
    EXPECT_EQ(allocator.allocate(registry.spawnEntity()), 2U);
    EXPECT_EQ(allocator.allocate(registry.spawnEntity()), 3U);
    EXPECT_EQ(allocator.size(), 3U);
}

TEST(NetIdAllocator, FindsALiveEntity)
{
    Registry registry;
    NetIdAllocator allocator;
    const Entity entity = registry.spawnEntity();
    const auto netId = allocator.allocate(entity);

    EXPECT_EQ(allocator.find(netId, registry), entity);
}

TEST(NetIdAllocator, FindIgnoresAKilledEntity)
{
    Registry registry;
    NetIdAllocator allocator;
    const Entity entity = registry.spawnEntity();
    const auto netId = allocator.allocate(entity);

    registry.killEntity(entity);
    EXPECT_EQ(allocator.find(netId, registry), std::nullopt);
}

TEST(NetIdAllocator, FindIgnoresAKilledEntityWhoseIndexWasReused)
{
    Registry registry;
    NetIdAllocator allocator;
    const Entity entity = registry.spawnEntity();
    const auto netId = allocator.allocate(entity);

    registry.killEntity(entity);
    const Entity reused = registry.spawnEntity();

    ASSERT_EQ(reused.index(), entity.index());
    EXPECT_EQ(allocator.find(netId, registry), std::nullopt);
}

TEST(NetIdAllocator, FindIgnoresAnUnknownNetId)
{
    const Registry registry;
    const NetIdAllocator allocator;

    EXPECT_EQ(allocator.find(42, registry), std::nullopt);
}

TEST(NetIdAllocator, ReleaseDeadForgetsOnlyTheKilledEntities)
{
    Registry registry;
    NetIdAllocator allocator;
    std::vector<Entity> entities;

    for (int i = 0; i < 5; ++i) {
        entities.push_back(registry.spawnEntity());
        (void)allocator.allocate(entities.back());
    }
    ASSERT_EQ(allocator.size(), 5U);
    registry.killEntity(entities[1]);
    registry.killEntity(entities[3]);
    allocator.releaseDead(registry);
    EXPECT_EQ(allocator.size(), 3U);
    EXPECT_EQ(allocator.find(1, registry), entities[0]);
    EXPECT_EQ(allocator.find(3, registry), entities[2]);
    EXPECT_EQ(allocator.find(5, registry), entities[4]);
}
