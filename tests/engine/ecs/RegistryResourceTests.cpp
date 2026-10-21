/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RegistryResourceTests
*/

#include <gtest/gtest.h>

#include <memory>
#include <mutex>

#include "engine/ecs/Resources.hpp"

using engine::ecs::DeltaTime;
using engine::ecs::ResourceConcept;

namespace {
struct MoveOnly {
    std::unique_ptr<int> value;
};

static_assert(ResourceConcept<int>);
static_assert(ResourceConcept<DeltaTime>);
static_assert(ResourceConcept<MoveOnly>);
static_assert(ResourceConcept<std::mutex>);
static_assert(!ResourceConcept<const int>);
static_assert(!ResourceConcept<int &>);
static_assert(!ResourceConcept<int &&>);
static_assert(!ResourceConcept<void>);
static_assert(!ResourceConcept<void()>);
} // namespace

TEST(DeltaTime, StartsAtZeroSeconds)
{
    EXPECT_EQ(DeltaTime{}.seconds, 0.0F);
}
