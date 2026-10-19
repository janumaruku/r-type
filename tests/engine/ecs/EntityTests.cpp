/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** EtityTests
*/

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <unordered_map>

#include "engine/ecs/Entity.hpp"

using engine::ecs::Entity;
using engine::ecs::detail::EntityAccess;

static_assert(sizeof(Entity) == 8);
static_assert(std::is_trivially_copyable_v<Entity>);
static_assert(!std::is_default_constructible_v<Entity>);
static_assert(!std::is_convertible_v<Entity, std::size_t>);
static_assert(!std::is_convertible_v<Entity, std::uint32_t>);
static_assert(!std::is_constructible_v<Entity, std::uint32_t>);
static_assert(!std::is_constructible_v<Entity, std::uint32_t, std::uint32_t>);

static_assert(EntityAccess::make(7, 3).index() == 7);
static_assert(EntityAccess::make(7, 3).generation() == 3);
static_assert(Entity::NONE != EntityAccess::make(0, 0));

TEST(Entity, MakeStoresIndexAndGeneration)
{
    const Entity entity = EntityAccess::make(7, 3);

    EXPECT_EQ(entity.index(), 7u);
    EXPECT_EQ(entity.generation(), 3u);
}

TEST(Entity, SameIndexAndGenerationAreEqual)
{
    EXPECT_EQ(EntityAccess::make(5, 1), EntityAccess::make(5, 1));
}

TEST(Entity, SameIndexDifferentGenerationAreNotEqual)
{
    EXPECT_NE(EntityAccess::make(5, 0), EntityAccess::make(5, 1));
}

TEST(Entity, SameGenerationDifferentIndexAreNotEqual)
{
    EXPECT_NE(EntityAccess::make(4, 1), EntityAccess::make(5, 1));
}

TEST(Entity, NoneHasAllBitsSet)
{
    EXPECT_EQ(Entity::NONE.index(), 0xFFFFFFFFu);
    EXPECT_EQ(Entity::NONE.generation(), 0xFFFFFFFFu);
}

TEST(Entity, NoneDiffersFromFirstEntity)
{
    EXPECT_NE(Entity::NONE, EntityAccess::make(0, 0));
}

TEST(Entity, CopyComparesEqualToOriginal)
{
    const Entity original = EntityAccess::make(12, 4);
    const Entity copy = original;

    EXPECT_EQ(copy, original);
}

TEST(Entity, EqualHandlesHaveEqualHashes)
{
    const std::hash<Entity> hash;

    EXPECT_EQ(hash(EntityAccess::make(5, 1)), hash(EntityAccess::make(5, 1)));
}

TEST(Entity, WorksAsUnorderedMapKey)
{
    std::unordered_map<Entity, int> values;

    values[EntityAccess::make(5, 0)] = 10;
    values[EntityAccess::make(5, 1)] = 20;

    EXPECT_EQ(values.size(), 2u);
    EXPECT_EQ(values.at(EntityAccess::make(5, 0)), 10);
    EXPECT_EQ(values.at(EntityAccess::make(5, 1)), 20);
}
