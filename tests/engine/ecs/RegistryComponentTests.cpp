/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RegistryComponentTests
*/

#include <gtest/gtest.h>

#include <memory>

#include "engine/core/Errors.hpp"
#include "engine/ecs/Registry.hpp"

using engine::core::EngineError;
using engine::ecs::Entity;
using engine::ecs::Registry;

namespace {
struct Position {
    float x;
    float y;
};

struct Health {
    int points;
};

struct Unregistered {
    int value;
};

struct Counted {
    static inline int copies = 0;
    static inline int moves = 0;
    int value = 0;

    Counted() = default;

    explicit Counted(int initial): value(initial)
    {
    }

    Counted(const Counted &other): value(other.value)
    {
        ++copies;
    }

    Counted(Counted &&other) noexcept: value(other.value)
    {
        ++moves;
    }

    Counted &operator=(const Counted &other)
    {
        value = other.value;
        ++copies;
        return *this;
    }

    Counted &operator=(Counted &&other) noexcept
    {
        value = other.value;
        ++moves;
        return *this;
    }

    static void reset()
    {
        copies = 0;
        moves = 0;
    }
};

struct MoveOnly {
    std::unique_ptr<int> value;
};

class RegistryComponent : public testing::Test {
protected:
    void SetUp() override
    {
        _registry.registerComponent<Position>();
        _registry.registerComponent<Health>();
        _registry.registerComponent<Counted>();
        _registry.registerComponent<MoveOnly>();
        Counted::reset();
    }

    // Kills the entity and spawns a new one on the same index.
    Entity reuseIndexOf(const Entity &entity)
    {
        _registry.killEntity(entity);
        const Entity newOwner = _registry.spawnEntity();
        EXPECT_EQ(newOwner.index(), entity.index());
        return newOwner;
    }

    Registry _registry;
};
} // namespace

TEST_F(RegistryComponent, AddingANamedComponentCopiesIt)
{
    const Entity entity = _registry.spawnEntity();
    Counted original(5);
    Counted::reset();

    _registry.addComponent(entity, original);

    EXPECT_EQ(Counted::copies, 1);
    EXPECT_EQ(original.value, 5);
    EXPECT_EQ(_registry.getComponent<Counted>(entity)->value, 5);
}

TEST_F(RegistryComponent, AddingAConstComponentCopiesIt)
{
    const Entity entity = _registry.spawnEntity();
    const Counted original(5);
    Counted::reset();

    _registry.addComponent(entity, original);

    EXPECT_EQ(Counted::copies, 1);
    EXPECT_EQ(_registry.getComponent<Counted>(entity)->value, 5);
}

TEST_F(RegistryComponent, AddingATemporaryNeverCopies)
{
    const Entity entity = _registry.spawnEntity();

    _registry.addComponent(entity, Counted(5));
    _registry.addComponent(entity, Counted(6));

    EXPECT_EQ(Counted::copies, 0);
    EXPECT_EQ(_registry.getComponent<Counted>(entity)->value, 6);
}

TEST_F(RegistryComponent, AddingReturnsTheStoredComponent)
{
    const Entity entity = _registry.spawnEntity();

    Position &added = _registry.addComponent(entity, Position{1.0F, 2.0F});
    added.x = 10.0F;
    Health &emplaced = _registry.emplaceComponent<Health>(entity, 3);
    emplaced.points = 30;

    EXPECT_EQ(&added, _registry.getComponent<Position>(entity));
    EXPECT_FLOAT_EQ(_registry.getComponent<Position>(entity)->x, 10.0F);
    EXPECT_EQ(&emplaced, _registry.getComponent<Health>(entity));
    EXPECT_EQ(_registry.getComponent<Health>(entity)->points, 30);
}

TEST_F(RegistryComponent, EmplaceBuildsTheComponentInPlace)
{
    const Entity entity = _registry.spawnEntity();

    _registry.emplaceComponent<Counted>(entity, 7);

    EXPECT_EQ(Counted::copies, 0);
    EXPECT_EQ(Counted::moves, 0);
    EXPECT_EQ(_registry.getComponent<Counted>(entity)->value, 7);
}

TEST_F(RegistryComponent, AddingAgainReplacesTheComponentWithoutGrowingTheStorage)
{
    const Entity entity = _registry.spawnEntity();
    _registry.addComponent(entity, Position{1.0F, 2.0F});

    _registry.addComponent(entity, Position{3.0F, 4.0F});
    EXPECT_FLOAT_EQ(_registry.getComponent<Position>(entity)->x, 3.0F);

    _registry.emplaceComponent<Position>(entity, 5.0F, 6.0F);
    EXPECT_FLOAT_EQ(_registry.getComponent<Position>(entity)->x, 5.0F);

    EXPECT_EQ(_registry.getComponents<Position>().size(), 1U);
}

TEST_F(RegistryComponent, RemovingAComponentKeepsTheEntitysOtherComponents)
{
    const Entity entity = _registry.spawnEntity();
    _registry.addComponent(entity, Position{1.0F, 2.0F});
    _registry.addComponent(entity, Health{3});

    _registry.removeComponent<Position>(entity);

    EXPECT_EQ(_registry.getComponent<Position>(entity), nullptr);
    ASSERT_NE(_registry.getComponent<Health>(entity), nullptr);
    EXPECT_EQ(_registry.getComponent<Health>(entity)->points, 3);
    EXPECT_TRUE(_registry.isAlive(entity));
}

TEST_F(RegistryComponent, RemovingAnAbsentComponentDoesNothing)
{
    const Entity entity = _registry.spawnEntity();
    const Entity other = _registry.spawnEntity();
    _registry.addComponent(other, Position{1.0F, 2.0F});

    _registry.removeComponent<Position>(entity);
    _registry.removeComponent<Position>(entity);

    EXPECT_EQ(_registry.getComponents<Position>().size(), 1U);
    EXPECT_NE(_registry.getComponent<Position>(other), nullptr);
}

TEST_F(RegistryComponent, AddingToADeadEntityThrowsAndCreatesNothing)
{
    const Entity entity = _registry.spawnEntity();
    _registry.killEntity(entity);

    EXPECT_THROW(_registry.addComponent(entity, Position{}), EngineError);
    EXPECT_THROW(_registry.emplaceComponent<Health>(entity, 1), EngineError);

    EXPECT_TRUE(_registry.getComponents<Position>().empty());
    EXPECT_TRUE(_registry.getComponents<Health>().empty());
}

TEST_F(RegistryComponent, AddingThroughAStaleHandleThrowsAndLeavesTheNewOwnerAlone)
{
    const Entity stale = _registry.spawnEntity();
    const Entity newOwner = reuseIndexOf(stale);

    EXPECT_THROW(_registry.addComponent(stale, Position{1.0F, 2.0F}),
        EngineError);
    EXPECT_THROW(_registry.emplaceComponent<Health>(stale, 1), EngineError);

    EXPECT_EQ(_registry.getComponent<Position>(newOwner), nullptr);
    EXPECT_EQ(_registry.getComponent<Health>(newOwner), nullptr);
    EXPECT_TRUE(_registry.isAlive(newOwner));
}

TEST_F(RegistryComponent, EveryCallOnAnUnregisteredTypeThrows)
{
    const Entity entity = _registry.spawnEntity();
    const Registry &constRegistry = _registry;

    EXPECT_THROW(_registry.addComponent(entity, Unregistered{1}), EngineError);
    EXPECT_THROW(_registry.emplaceComponent<Unregistered>(entity, 1),
        EngineError);
    EXPECT_THROW(_registry.getComponent<Unregistered>(entity), EngineError);
    EXPECT_THROW(constRegistry.getComponent<Unregistered>(entity),
        EngineError);
    EXPECT_THROW(_registry.removeComponent<Unregistered>(entity), EngineError);
}

TEST_F(RegistryComponent, AnUnregisteredTypeThrowsEvenForADeadEntity)
{
    const Entity entity = _registry.spawnEntity();
    _registry.killEntity(entity);

    EXPECT_THROW(_registry.addComponent(entity, Unregistered{1}), EngineError);
    EXPECT_THROW(_registry.getComponent<Unregistered>(entity), EngineError);
    EXPECT_THROW(_registry.removeComponent<Unregistered>(entity), EngineError);
}

TEST_F(RegistryComponent, GetReturnsNullptrForAnAbsentComponent)
{
    const Entity entity = _registry.spawnEntity();
    const Registry &constRegistry = _registry;

    EXPECT_EQ(_registry.getComponent<Position>(entity), nullptr);
    EXPECT_EQ(constRegistry.getComponent<Position>(entity), nullptr);
}

TEST_F(RegistryComponent, GetReadsThroughAConstRegistry)
{
    const Entity entity = _registry.spawnEntity();
    _registry.addComponent(entity, Health{4});
    const Registry &constRegistry = _registry;

    const Health *health = constRegistry.getComponent<Health>(entity);

    ASSERT_NE(health, nullptr);
    EXPECT_EQ(health->points, 4);
}

TEST_F(RegistryComponent, GetReturnsNullptrForAStaleHandle)
{
    const Entity stale = _registry.spawnEntity();
    _registry.addComponent(stale, Health{1});
    const Entity newOwner = reuseIndexOf(stale);
    _registry.addComponent(newOwner, Health{2});
    const Registry &constRegistry = _registry;

    EXPECT_EQ(_registry.getComponent<Health>(stale), nullptr);
    EXPECT_EQ(constRegistry.getComponent<Health>(stale), nullptr);
    EXPECT_EQ(_registry.getComponent<Health>(newOwner)->points, 2);
}

TEST_F(RegistryComponent, RemovingThroughAStaleHandleKeepsTheNewOwnersComponent)
{
    const Entity stale = _registry.spawnEntity();
    const Entity newOwner = reuseIndexOf(stale);
    _registry.addComponent(newOwner, Position{7.0F, 8.0F});

    _registry.removeComponent<Position>(stale);

    ASSERT_NE(_registry.getComponent<Position>(newOwner), nullptr);
    EXPECT_FLOAT_EQ(_registry.getComponent<Position>(newOwner)->x, 7.0F);
}

TEST_F(RegistryComponent, AMoveOnlyComponentCanBeAddedAndReadBack)
{
    const Entity entity = _registry.spawnEntity();

    _registry.addComponent(entity, MoveOnly{std::make_unique<int>(42)});

    const MoveOnly *component = _registry.getComponent<MoveOnly>(entity);
    ASSERT_NE(component, nullptr);
    ASSERT_NE(component->value, nullptr);
    EXPECT_EQ(*component->value, 42);
}
