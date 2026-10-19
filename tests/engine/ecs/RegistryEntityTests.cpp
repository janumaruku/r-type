/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RegistryEntityTests
*/

#include <gtest/gtest.h>

#include <cstdint>

#include "engine/ecs/Registry.hpp"

using engine::ecs::Entity;
using engine::ecs::Registry;
using engine::ecs::detail::EntityAccess;
using engine::ecs::detail::typeId;

namespace {
struct Position {
    float x;
    float y;
};

struct Health {
    int points;
};
} // namespace

TEST(RegistryEntity, FreshEntitiesGetIndicesZeroOneTwoWithGenerationZero)
{
    Registry registry;

    const Entity first = registry.spawnEntity();
    const Entity second = registry.spawnEntity();
    const Entity third = registry.spawnEntity();

    EXPECT_EQ(first, EntityAccess::make(0, 0));
    EXPECT_EQ(second, EntityAccess::make(1, 0));
    EXPECT_EQ(third, EntityAccess::make(2, 0));
}

TEST(RegistryEntity, ASpawnReusesTheLastFreedIndexWithTheNextGeneration)
{
    Registry registry;
    const Entity first = registry.spawnEntity();
    registry.spawnEntity();
    const Entity third = registry.spawnEntity();

    registry.killEntity(first);
    registry.killEntity(third);

    EXPECT_EQ(registry.spawnEntity(), EntityAccess::make(2, 1));
    EXPECT_EQ(registry.spawnEntity(), EntityAccess::make(0, 1));
    EXPECT_EQ(registry.spawnEntity(), EntityAccess::make(3, 0));
}

TEST(RegistryEntity, AKilledHandleStaysDeadAfterItsIndexIsReused)
{
    Registry registry;
    const Entity old = registry.spawnEntity();

    registry.killEntity(old);
    EXPECT_FALSE(registry.isAlive(old));

    const Entity reused = registry.spawnEntity();
    ASSERT_EQ(reused.index(), old.index());
    EXPECT_FALSE(registry.isAlive(old));
    EXPECT_TRUE(registry.isAlive(reused));
}

TEST(RegistryEntity, KillingTwiceDoesNothingTheSecondTime)
{
    Registry registry;
    const Entity entity = registry.spawnEntity();

    registry.killEntity(entity);
    registry.killEntity(entity);

    const Entity first = registry.spawnEntity();
    const Entity second = registry.spawnEntity();
    EXPECT_NE(first.index(), second.index());
    EXPECT_EQ(registry.aliveCount(), 2U);
}

TEST(RegistryEntity, NoneAndFabricatedHandlesAreNeverAliveAndCantBeKilled)
{
    Registry registry;
    const Entity entity = registry.spawnEntity();
    registry.registerComponent<Position>().insertAt(entity, Position{1.0F,
        2.0F});
    const Entity futureGeneration = EntityAccess::make(entity.index(), 5);
    const Entity outOfRange = EntityAccess::make(99, 0);

    EXPECT_FALSE(registry.isAlive(Entity::NONE));
    EXPECT_FALSE(registry.isAlive(futureGeneration));
    EXPECT_FALSE(registry.isAlive(outOfRange));

    registry.killEntity(Entity::NONE);
    registry.killEntity(futureGeneration);
    registry.killEntity(outOfRange);

    EXPECT_TRUE(registry.isAlive(entity));
    EXPECT_TRUE(registry.getComponents<Position>().contains(entity));
    EXPECT_EQ(registry.aliveCount(), 1U);
}

TEST(RegistryEntity, AKillRemovesEveryComponentOfThatEntityOnly)
{
    Registry registry;
    auto &positions = registry.registerComponent<Position>();
    auto &healths = registry.registerComponent<Health>();
    const Entity victim = registry.spawnEntity();
    const Entity survivor = registry.spawnEntity();
    positions.insertAt(victim, Position{});
    healths.insertAt(victim, Health{3});
    positions.insertAt(survivor, Position{5.0F, 6.0F});
    healths.insertAt(survivor, Health{7});

    registry.killEntity(victim);

    EXPECT_FALSE(positions.contains(victim));
    EXPECT_FALSE(healths.contains(victim));
    EXPECT_FLOAT_EQ(positions.get(survivor).x, 5.0F);
    EXPECT_EQ(healths.get(survivor).points, 7);
}

TEST(RegistryEntity, ARespawnedIndexHasNoGhostComponents)
{
    Registry registry;
    auto &positions = registry.registerComponent<Position>();
    const Entity old = registry.spawnEntity();
    positions.insertAt(old, Position{});

    registry.killEntity(old);
    const Entity reused = registry.spawnEntity();

    ASSERT_EQ(reused.index(), old.index());
    EXPECT_FALSE(positions.contains(reused));
    EXPECT_TRUE(positions.empty());
}

TEST(RegistryEntity, AKillErasesComponentsOfATypeRegisteredAfterTheSpawn)
{
    Registry registry;
    const Entity entity = registry.spawnEntity();
    auto &healths = registry.registerComponent<Health>();
    healths.insertAt(entity, Health{1});

    registry.killEntity(entity);

    EXPECT_TRUE(healths.empty());
}

// The slot exists because a later type grew the vector, but holds nullptr.
TEST(RegistryEntity, AKillSkipsEmptyStorageSlots)
{
    struct Skipped {};
    struct RegisteredAfter {};
    Registry registry;
    const auto skipped = typeId<Skipped>();
    registry.registerComponent<RegisteredAfter>();
    ASSERT_LT(skipped, typeId<RegisteredAfter>());
    const Entity entity = registry.spawnEntity();

    registry.killEntity(entity);

    EXPECT_FALSE(registry.isAlive(entity));
}

TEST(RegistryEntity, AliveCountFollowsSpawnsAndKills)
{
    Registry registry;
    EXPECT_EQ(registry.aliveCount(), 0U);

    const Entity first = registry.spawnEntity();
    const Entity second = registry.spawnEntity();
    EXPECT_EQ(registry.aliveCount(), 2U);

    registry.killEntity(first);
    EXPECT_EQ(registry.aliveCount(), 1U);

    registry.spawnEntity();
    EXPECT_EQ(registry.aliveCount(), 2U);

    registry.killEntity(second);
    registry.killEntity(second);
    EXPECT_EQ(registry.aliveCount(), 1U);
}
