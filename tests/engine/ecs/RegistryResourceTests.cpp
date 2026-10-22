/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RegistryResourceTests
*/

#include <gtest/gtest.h>

#include <cstddef>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include "Errors.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/Resources.hpp"

using engine::core::EngineError;
using engine::ecs::DeltaTime;
using engine::ecs::Entity;
using engine::ecs::Registry;
using engine::ecs::ResourceConcept;

namespace {
struct MoveOnly {
    std::unique_ptr<int> value;
};

struct Score {
    int points;
};

struct NeverSet {
    int value;
};

template <std::size_t N>
struct Extra {
    int value;
};

struct ThrowsOnDemand {
    int value;

    ThrowsOnDemand(int initial, bool shouldThrow): value(initial)
    {
        if (shouldThrow)
            throw std::runtime_error("ThrowsOnDemand");
    }
};

class RegistryResource: public testing::Test {
protected:
    Registry _registry;
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

TEST_F(RegistryResource, GetAfterSetReturnsTheValue)
{
    _registry.setResource(DeltaTime{0.5F});

    EXPECT_EQ(_registry.getResource<DeltaTime>().seconds, 0.5F);
    ASSERT_NE(_registry.tryGetResource<DeltaTime>(), nullptr);
    EXPECT_EQ(_registry.tryGetResource<DeltaTime>()->seconds, 0.5F);
}

TEST_F(RegistryResource, SetReturnsTheStoredResource)
{
    DeltaTime &stored = _registry.setResource(DeltaTime{0.5F});
    stored.seconds = 2.0F;

    EXPECT_EQ(&stored, &_registry.getResource<DeltaTime>());
    EXPECT_EQ(_registry.getResource<DeltaTime>().seconds, 2.0F);
}

TEST_F(RegistryResource, SettingAgainUpdatesTheValueAndKeepsTheAddress)
{
    const DeltaTime *first = &_registry.setResource(DeltaTime{0.5F});

    const DeltaTime *second = &_registry.setResource(DeltaTime{0.25F});

    EXPECT_EQ(first, second);
    EXPECT_EQ(&_registry.getResource<DeltaTime>(), first);
    EXPECT_EQ(_registry.getResource<DeltaTime>().seconds, 0.25F);
}

TEST_F(RegistryResource, AnUnsetResourceThrowsNamingItsType)
{
    EXPECT_EQ(_registry.tryGetResource<NeverSet>(), nullptr);

    try {
        static_cast<void>(_registry.getResource<NeverSet>());
        FAIL() << "getResource didn't throw";
    } catch (const EngineError &error) {
        EXPECT_NE(std::string{error.what()}.find(typeid(NeverSet).name()),
            std::string::npos);
    }
}

TEST_F(RegistryResource, ConstOverloadsReadThroughAConstRegistry)
{
    _registry.setResource(DeltaTime{0.5F});
    const Registry &registry = _registry;

    static_assert(std::is_same_v<decltype(registry.getResource<DeltaTime>()),
        const DeltaTime &>);
    static_assert(std::is_same_v<decltype(registry.tryGetResource<DeltaTime>()),
        const DeltaTime *>);
    EXPECT_EQ(registry.getResource<DeltaTime>().seconds, 0.5F);
    EXPECT_EQ(registry.tryGetResource<DeltaTime>(),
        _registry.tryGetResource<DeltaTime>());
    EXPECT_EQ(registry.tryGetResource<NeverSet>(), nullptr);
    EXPECT_THROW(static_cast<void>(registry.getResource<NeverSet>()),
        EngineError);
}

TEST_F(RegistryResource, AMoveOnlyResourceCanBeSetAndReplaced)
{
    MoveOnly &stored =
        _registry.setResource(MoveOnly{std::make_unique<int>(4)});

    _registry.setResource(MoveOnly{std::make_unique<int>(5)});

    ASSERT_NE(stored.value, nullptr);
    EXPECT_EQ(*stored.value, 5);
    EXPECT_EQ(&stored, &_registry.getResource<MoveOnly>());
}

TEST_F(RegistryResource, AComponentAndAResourceOfOneTypeAreIndependent)
{
    _registry.registerComponent<Score>();
    const Entity entity = _registry.spawnEntity();
    _registry.addComponent(entity, Score{10});

    _registry.setResource(Score{99});
    _registry.getComponent<Score>(entity)->points = 11;

    EXPECT_EQ(_registry.getResource<Score>().points, 99);
    EXPECT_EQ(_registry.getComponent<Score>(entity)->points, 11);
    EXPECT_NE(&_registry.getResource<Score>(),
        _registry.getComponent<Score>(entity));
}

TEST_F(RegistryResource, ReferencesSurviveSettingManyOtherResources)
{
    DeltaTime &kept = _registry.setResource(DeltaTime{0.5F});

    [this]<std::size_t... I>(std::index_sequence<I...>) {
        (_registry.setResource(Extra<I>{static_cast<int>(I)}), ...);
    }(std::make_index_sequence<32>{});

    EXPECT_EQ(&kept, &_registry.getResource<DeltaTime>());
    EXPECT_EQ(kept.seconds, 0.5F);
    EXPECT_EQ(_registry.getResource<Extra<31>>().value, 31);
}

TEST_F(RegistryResource, ReferencesSurviveMovingTheRegistry)
{
    DeltaTime &kept = _registry.setResource(DeltaTime{0.5F});

    Registry moved = std::move(_registry);

    EXPECT_EQ(&kept, &moved.getResource<DeltaTime>());
    EXPECT_EQ(moved.getResource<DeltaTime>().seconds, 0.5F);
}

TEST_F(RegistryResource, EmplaceBuildsAMoveOnlyResourceInPlace)
{
    MoveOnly &stored =
        _registry.emplaceResource<MoveOnly>(std::make_unique<int>(4));

    ASSERT_NE(stored.value, nullptr);
    EXPECT_EQ(*stored.value, 4);
    EXPECT_EQ(&stored, &_registry.getResource<MoveOnly>());
}

TEST_F(RegistryResource, EmplaceAcceptsATypeThatCannotBeMoved)
{
    std::mutex &stored = _registry.emplaceResource<std::mutex>();

    EXPECT_EQ(&stored, &_registry.getResource<std::mutex>());
    EXPECT_TRUE(_registry.getResource<std::mutex>().try_lock());
    _registry.getResource<std::mutex>().unlock();
}

TEST_F(RegistryResource, EmplaceAgainReplacesTheResource)
{
    _registry.emplaceResource<DeltaTime>(0.5F);

    const DeltaTime &replaced = _registry.emplaceResource<DeltaTime>(1.0F);

    EXPECT_EQ(&replaced, &_registry.getResource<DeltaTime>());
    EXPECT_EQ(_registry.getResource<DeltaTime>().seconds, 1.0F);
}

TEST_F(RegistryResource, EmplaceBuildsAnAggregateFromItsFields)
{
    _registry.emplaceResource<Extra<0>>(7);

    EXPECT_EQ(_registry.getResource<Extra<0>>().value, 7);
}

TEST_F(RegistryResource, AThrowingEmplaceKeepsTheOldResource)
{
    _registry.emplaceResource<ThrowsOnDemand>(3, false);

    EXPECT_THROW(_registry.emplaceResource<ThrowsOnDemand>(4, true),
        std::runtime_error);

    ASSERT_NE(_registry.tryGetResource<ThrowsOnDemand>(), nullptr);
    EXPECT_EQ(_registry.getResource<ThrowsOnDemand>().value, 3);
}
