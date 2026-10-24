/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RegistryTests
*/

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>

#include "engine/core/Errors.hpp"
#include "engine/ecs/Registry.hpp"

using engine::core::EngineError;
using engine::ecs::Entity;
using engine::ecs::Registry;
using engine::ecs::SparseSet;
using engine::ecs::detail::EntityAccess;
using engine::ecs::detail::typeId;

namespace {
struct Position {
    float x;
    float y;
};

struct Velocity {
    float dx;
    float dy;
};

struct NeverRegistered {
};

template <std::size_t N>
struct Tag {
};

Entity entity(std::uint32_t index, std::uint32_t generation = 0)
{
    return EntityAccess::make(index, generation);
}

template <class C, class R>
testing::AssertionResult throwsNamingTheType(R &registry,
    const std::string &name)
{
    try {
        (void)registry.template getComponents<C>();
    } catch (const EngineError &error) {
        if (std::string{error.what()}.find(name) != std::string::npos)
            return testing::AssertionSuccess();
        return testing::AssertionFailure()
            << "message \"" << error.what() << "\" doesn't name " << name;
    }
    return testing::AssertionFailure() << "no EngineError thrown";
}
} // namespace

static_assert(!std::is_copy_constructible_v<Registry>);
static_assert(!std::is_copy_assignable_v<Registry>);
static_assert(std::is_nothrow_move_constructible_v<Registry>);
static_assert(std::is_nothrow_move_assignable_v<Registry>);

TEST(Registry, RegisteringReturnsAnEmptyStorageThatGetComponentsFinds)
{
    Registry registry;

    auto &positions = registry.registerComponent<Position>();

    EXPECT_TRUE(positions.empty());
    EXPECT_EQ(&registry.getComponents<Position>(), &positions);
}

TEST(Registry, WritesThroughOneReferenceAreVisibleThroughAnother)
{
    Registry registry;

    registry.registerComponent<Position>().insertAt(entity(0),
        Position{1.0F, 2.0F});

    EXPECT_FLOAT_EQ(registry.getComponents<Position>().get(entity(0)).x, 1.0F);
}

TEST(Registry, EachTypeHasItsOwnStorage)
{
    Registry registry;
    auto &positions = registry.registerComponent<Position>();
    const auto &velocities = registry.registerComponent<Velocity>();

    positions.insertAt(entity(0), Position{});

    EXPECT_NE(static_cast<const void *>(&positions),
        static_cast<const void *>(&velocities));
    EXPECT_EQ(positions.size(), 1U);
    EXPECT_TRUE(velocities.empty());
}

TEST(Registry, RegisteringTwiceKeepsTheData)
{
    Registry registry;
    auto &first = registry.registerComponent<Position>();
    first.insertAt(entity(0), Position{3.0F, 4.0F});

    auto &second = registry.registerComponent<Position>();

    EXPECT_EQ(&second, &first);
    EXPECT_EQ(second.size(), 1U);
    EXPECT_FLOAT_EQ(second.get(entity(0)).y, 4.0F);
}

TEST(Registry, GetComponentsOnAnUnregisteredTypeThrowsNamingIt)
{
    Registry registry;

    EXPECT_TRUE(throwsNamingTheType<NeverRegistered>(registry,
        "NeverRegistered"));
}

// The slot exists because a later type grew the vector, but holds nullptr.
TEST(Registry, AnEmptySlotInsideTheVectorStillThrows)
{
    struct Skipped {
    };
    struct RegisteredAfter {
    };
    Registry registry;
    const auto skipped = typeId<Skipped>();

    registry.registerComponent<RegisteredAfter>();

    ASSERT_LT(skipped, typeId<RegisteredAfter>());
    EXPECT_TRUE(throwsNamingTheType<Skipped>(registry, "Skipped"));
    EXPECT_TRUE(throwsNamingTheType<Skipped>(std::as_const(registry),
        "Skipped"));
}

TEST(Registry, TheConstOverloadWorksThroughAConstRegistry)
{
    Registry registry;
    registry.registerComponent<Position>().insertAt(entity(3),
        Position{.x=4.0F, .y=5.0F});
    const Registry &readOnly = registry;

    const auto &positions = readOnly.getComponents<Position>();

    static_assert(std::is_same_v<decltype(readOnly.getComponents<Position>()),
        const SparseSet<Position> &>);
    EXPECT_EQ(&positions, &registry.getComponents<Position>());
    EXPECT_FLOAT_EQ(positions.get(entity(3)).y, 5.0F);
    EXPECT_TRUE(throwsNamingTheType<Velocity>(readOnly, "Velocity"));
}

TEST(Registry, AReferenceSurvivesSixtyFourMoreRegistrations)
{
    Registry registry;
    auto &positions = registry.registerComponent<Position>();
    positions.insertAt(entity(0), Position{.x=1.0F, .y=1.0F});

    [&registry]<std::size_t... I>(std::index_sequence<I...>) {
        (registry.registerComponent<Tag<I>>(), ...);
    }(std::make_index_sequence<64>{});

    EXPECT_EQ(&positions, &registry.getComponents<Position>());
    EXPECT_FLOAT_EQ(positions.get(entity(0)).x, 1.0F);
}

TEST(Registry, TwoRegistriesAreIndependent)
{
    Registry first;
    Registry second;

    first.registerComponent<Position>();
    EXPECT_THROW((void)second.getComponents<Position>(), EngineError);

    second.registerComponent<Position>().insertAt(entity(0), Position{});
    EXPECT_TRUE(first.getComponents<Position>().empty());
}

TEST(Registry, MoveConstructionKeepsTheStoragesInPlace)
{
    Registry original;
    auto &positions = original.registerComponent<Position>();
    positions.insertAt(entity(2), Position{7.0F, 8.0F});

    Registry moved(std::move(original));

    EXPECT_EQ(&moved.getComponents<Position>(), &positions);
    EXPECT_FLOAT_EQ(moved.getComponents<Position>().get(entity(2)).x, 7.0F);
}

TEST(Registry, MoveAssignmentTakesTheStoragesAndDropsTheOldOnes)
{
    Registry original;
    auto &positions = original.registerComponent<Position>();
    positions.insertAt(entity(2), Position{7.0F, 8.0F});
    Registry target;
    target.registerComponent<Velocity>();

    target = std::move(original);

    EXPECT_EQ(&target.getComponents<Position>(), &positions);
    EXPECT_FLOAT_EQ(target.getComponents<Position>().get(entity(2)).y, 8.0F);
    EXPECT_THROW((void)target.getComponents<Velocity>(), EngineError);
}
