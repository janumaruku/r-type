/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** CommandBufferTests
*/

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "Errors.hpp"
#include "engine/ecs/CommandBuffer.hpp"
#include "engine/ecs/Registry.hpp"

using engine::core::EngineError;
using engine::ecs::CommandBuffer;
using engine::ecs::Entity;
using engine::ecs::Registry;

namespace {
struct Position {
    int x;
};

struct Health {
    int points;
};

struct Unregistered {
    int value;
};

struct MoveOnly {
    std::unique_ptr<int> value;
};

class CommandBufferTest: public testing::Test {
protected:
    void SetUp() override
    {
        _registry.registerComponent<Position>();
        _registry.registerComponent<Health>();
    }

    Registry _registry;
    CommandBuffer _buffer;
};

static_assert(!std::is_copy_constructible_v<MoveOnly>);
} // namespace

TEST_F(CommandBufferTest, NothingChangesBeforeApplyAndTheBufferIsEmptyAfter)
{
    const Entity entity = _registry.spawnEntity();

    _buffer.add(entity, Position{1});
    _buffer.spawn();
    _buffer.kill(entity);

    EXPECT_EQ(_buffer.size(), 3U);
    EXPECT_FALSE(_buffer.empty());
    EXPECT_TRUE(_registry.isAlive(entity));
    EXPECT_EQ(_registry.getComponent<Position>(entity), nullptr);
    EXPECT_EQ(_registry.aliveCount(), 1U);

    _buffer.apply(_registry);

    EXPECT_TRUE(_buffer.empty());
    EXPECT_EQ(_buffer.size(), 0U);
    EXPECT_FALSE(_registry.isAlive(entity));
    EXPECT_EQ(_registry.aliveCount(), 1U);
}

TEST_F(CommandBufferTest, AddThenRemoveLeavesNoComponent)
{
    const Entity entity = _registry.spawnEntity();

    _buffer.add(entity, Position{1});
    _buffer.remove<Position>(entity);
    _buffer.apply(_registry);

    EXPECT_EQ(_registry.getComponent<Position>(entity), nullptr);
}

TEST_F(CommandBufferTest, RemoveThenAddLeavesOneComponent)
{
    const Entity entity = _registry.spawnEntity();
    _registry.addComponent(entity, Position{1});

    _buffer.remove<Position>(entity);
    _buffer.add(entity, Position{2});
    _buffer.apply(_registry);

    ASSERT_NE(_registry.getComponent<Position>(entity), nullptr);
    EXPECT_EQ(_registry.getComponent<Position>(entity)->x, 2);
}

TEST_F(CommandBufferTest, TwoKillsOfOneEntityKillItOnce)
{
    const Entity first = _registry.spawnEntity();
    const Entity second = _registry.spawnEntity();

    _buffer.kill(first);
    _buffer.kill(second);
    _buffer.kill(second);
    _buffer.kill(first);

    EXPECT_NO_THROW(_buffer.apply(_registry));
    EXPECT_FALSE(_registry.isAlive(first));
    EXPECT_FALSE(_registry.isAlive(second));
    EXPECT_EQ(_registry.aliveCount(), 0U);
}

TEST_F(CommandBufferTest, AnAddToAnEntityKilledEarlierInTheBatchIsIgnored)
{
    const Entity entity = _registry.spawnEntity();

    _buffer.kill(entity);
    _buffer.add(entity, Position{1});

    EXPECT_NO_THROW(_buffer.apply(_registry));
    EXPECT_TRUE(_registry.getComponents<Position>().empty());
}

TEST_F(CommandBufferTest, AnAddThroughAStaleHandleLeavesTheNewEntityAlone)
{
    const Entity stale = _registry.spawnEntity();
    _registry.killEntity(stale);
    const Entity reused = _registry.spawnEntity();
    ASSERT_EQ(reused.index(), stale.index());

    _buffer.add(stale, Position{1});
    _buffer.remove<Health>(stale);
    _registry.addComponent(reused, Health{3});
    _buffer.apply(_registry);

    EXPECT_EQ(_registry.getComponent<Position>(reused), nullptr);
    ASSERT_NE(_registry.getComponent<Health>(reused), nullptr);
    EXPECT_EQ(_registry.getComponent<Health>(reused)->points, 3);
}

TEST_F(CommandBufferTest, ARemoveOnADeadEntityIsIgnored)
{
    const Entity entity = _registry.spawnEntity();
    _registry.addComponent(entity, Position{1});

    _buffer.kill(entity);
    _buffer.remove<Position>(entity);

    EXPECT_NO_THROW(_buffer.apply(_registry));
    EXPECT_TRUE(_registry.getComponents<Position>().empty());
}

TEST_F(CommandBufferTest, ASpawnCreatesTheEntityWithWhatInitAdds)
{
    Entity spawned = Entity::NONE;

    _buffer.spawn([&spawned](Registry &registry, const Entity &entity) {
        spawned = entity;
        registry.addComponent(entity, Position{4});
        registry.addComponent(entity, Health{10});
    });
    _buffer.apply(_registry);

    ASSERT_TRUE(_registry.isAlive(spawned));
    ASSERT_NE(_registry.getComponent<Position>(spawned), nullptr);
    ASSERT_NE(_registry.getComponent<Health>(spawned), nullptr);
    EXPECT_EQ(_registry.getComponent<Position>(spawned)->x, 4);
    EXPECT_EQ(_registry.getComponent<Health>(spawned)->points, 10);
}

TEST_F(CommandBufferTest, ASpawnWithoutInitCreatesAnEmptyEntity)
{
    _buffer.spawn();
    _buffer.apply(_registry);

    EXPECT_EQ(_registry.aliveCount(), 1U);
    EXPECT_TRUE(_registry.getComponents<Position>().empty());
    EXPECT_TRUE(_registry.getComponents<Health>().empty());
}

TEST_F(CommandBufferTest, AThrowingInitLeavesNoEntity)
{
    _buffer.spawn([](Registry &registry, const Entity &entity) {
        registry.addComponent(entity, Position{1});
        throw std::runtime_error("init failed");
    });

    EXPECT_THROW(_buffer.apply(_registry), std::runtime_error);

    EXPECT_EQ(_registry.aliveCount(), 0U);
    EXPECT_TRUE(_registry.getComponents<Position>().empty());
}

TEST_F(CommandBufferTest, CommandsRecordedDuringApplyRunInTheSameCall)
{
    std::vector<int> order;

    _buffer.spawn([this, &order](Registry &, const Entity &entity) {
        order.push_back(1);
        _buffer.add(entity, Position{7});
        _buffer.spawn([&order](Registry &, const Entity &) {
            order.push_back(3);
        });
    });
    _buffer.spawn([&order](Registry &, const Entity &) { order.push_back(2); });
    _buffer.apply(_registry);

    EXPECT_TRUE(_buffer.empty());
    EXPECT_EQ(order, (std::vector<int>{1, 2, 3}));
    EXPECT_EQ(_registry.aliveCount(), 3U);
    EXPECT_EQ(_registry.getComponents<Position>().size(), 1U);
}

TEST_F(CommandBufferTest, ManyCommandsRecordedDuringApplyAllRun)
{
    _buffer.spawn([this](Registry &, const Entity &) {
        for (int i = 0; i < 100; ++i)
            _buffer.spawn();
    });
    _buffer.apply(_registry);

    EXPECT_TRUE(_buffer.empty());
    EXPECT_EQ(_registry.aliveCount(), 101U);
}

TEST_F(CommandBufferTest, AThrowingCommandReachesTheCallerAndEmptiesTheBuffer)
{
    const Entity kept = _registry.spawnEntity();

    _buffer.spawn();
    _buffer.spawn([](Registry &, const Entity &) {
        throw std::runtime_error("command failed");
    });
    _buffer.kill(kept);

    EXPECT_THROW(_buffer.apply(_registry), std::runtime_error);

    EXPECT_TRUE(_buffer.empty());
    EXPECT_TRUE(_registry.isAlive(kept));
    EXPECT_EQ(_registry.aliveCount(), 2U);

    _buffer.apply(_registry);
    EXPECT_EQ(_registry.aliveCount(), 2U);
}

TEST_F(CommandBufferTest, AnAddOfAnUnregisteredTypeThrowsWhenApplied)
{
    const Entity entity = _registry.spawnEntity();

    EXPECT_NO_THROW(_buffer.add(entity, Unregistered{1}));

    EXPECT_THROW(_buffer.apply(_registry), EngineError);
    EXPECT_TRUE(_buffer.empty());
}

TEST_F(CommandBufferTest, KillingEveryEntityFromAnIndexedViewLoop)
{
    for (int i = 0; i < 10; ++i) {
        const Entity entity = _registry.spawnEntity();
        _registry.addComponent(entity, Position{i});
        _registry.addComponent(entity, Health{i});
    }

    int visited = 0;
    for (auto &&[entity, position, health]:
        _registry.indexedView<const Position, const Health>()) {
        _buffer.kill(entity);
        ++visited;
    }
    EXPECT_EQ(visited, 10);
    EXPECT_EQ(_registry.aliveCount(), 10U);

    _buffer.apply(_registry);

    EXPECT_EQ(_registry.aliveCount(), 0U);
    EXPECT_TRUE(_registry.getComponents<Position>().empty());
    EXPECT_TRUE(_registry.getComponents<Health>().empty());
}

TEST_F(CommandBufferTest, TheRegistryAppliesItsOwnBuffer)
{
    const Entity entity = _registry.spawnEntity();

    _registry.commands().add(entity, Position{5});
    _registry.commands().kill(_registry.spawnEntity());

    EXPECT_EQ(_registry.commands().size(), 2U);
    EXPECT_EQ(_registry.getComponent<Position>(entity), nullptr);

    _registry.applyCommands();

    EXPECT_TRUE(_registry.commands().empty());
    ASSERT_NE(_registry.getComponent<Position>(entity), nullptr);
    EXPECT_EQ(_registry.getComponent<Position>(entity)->x, 5);
    EXPECT_EQ(_registry.aliveCount(), 1U);
}

TEST_F(CommandBufferTest, ACommandCanRecordIntoTheRegistryBuffer)
{
    Entity spawned = Entity::NONE;

    _registry.commands().spawn(
        [&spawned](Registry &registry, const Entity &entity) {
            spawned = entity;
            registry.commands().add(entity, Health{2});
        });
    _registry.applyCommands();

    ASSERT_TRUE(_registry.isAlive(spawned));
    ASSERT_NE(_registry.getComponent<Health>(spawned), nullptr);
    EXPECT_EQ(_registry.getComponent<Health>(spawned)->points, 2);
}
