/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** EcsIntegrationTests
*/

#include <gtest/gtest.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <map>
#include <optional>
#include <tuple>
#include <unordered_set>
#include <vector>

#include "engine/core/Errors.hpp"
#include "engine/core/Rng.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/Resources.hpp"
#include "engine/ecs/SparseSet.hpp"

using engine::core::EngineError;
using engine::core::Rng;
using engine::ecs::DeltaTime;
using engine::ecs::Entity;
using engine::ecs::Registry;
using engine::ecs::SparseSet;

namespace {
enum class Stage: std::uint8_t {
    SIMULATION,
};

struct Position {
    float x;
    float y;
};

struct Velocity {
    float dx;
    float dy;
};

struct Radius {
    float value;
};

struct Lifetime {
    float seconds;
};

struct Spawner {
    float seconds;
};

struct Counters {
    std::size_t spawned;
    std::size_t killed;
};

constexpr std::uint32_t SEED = 42;
constexpr int STEPS = 600;
constexpr int OPERATIONS = 10000;
constexpr std::size_t MAX_ALIVE = 32;
constexpr std::size_t MAX_HANDLES = 64;
constexpr float STEP = 1.0F / 60.0F;
constexpr float SPAWN_INTERVAL = 0.1F;

void buildWorld(Registry &registry, std::uint32_t seed)
{
    registry.registerComponent<Position>();
    registry.registerComponent<Velocity>();
    registry.registerComponent<Radius>();
    registry.registerComponent<Lifetime>();
    registry.registerComponent<Spawner>();

    registry.setResource(DeltaTime{STEP});
    registry.setResource(Rng{seed});
    registry.setResource(Counters{.spawned = 1, .killed = 0});

    const Entity spawner = registry.spawnEntity();
    registry.addComponent(spawner, Spawner{0.0F});

    registry.addSystem<Spawner>(Stage::SIMULATION, "spawner",
        [](Registry &registry, SparseSet<Spawner> &spawners) {
            const float dt = registry.getResource<DeltaTime>().seconds;
            Rng &rng = registry.getResource<Rng>();

            for (Spawner &spawner: spawners.components()) {
                spawner.seconds -= dt;
                if (spawner.seconds > 0.0F)
                    continue;
                spawner.seconds += SPAWN_INTERVAL;

                const Position position{.x=rng.uniformFloat(0.0F, 100.0F),
                    .y=rng.uniformFloat(0.0F, 100.0F),};
                const Velocity velocity{.dx=rng.uniformFloat(-20.0F, 20.0F),
                    .dy=rng.uniformFloat(-20.0F, 20.0F),};
                const Radius radius{rng.uniformFloat(1.0F, 3.0F)};
                const Lifetime lifetime{rng.uniformFloat(1.0F, 3.0F)};

                registry.commands().spawn(
                    [position, velocity, radius, lifetime](Registry &world,
                        const Entity &entity) {
                        world.addComponent(entity, position);
                        world.addComponent(entity, velocity);
                        world.addComponent(entity, radius);
                        world.addComponent(entity, lifetime);
                        ++world.getResource<Counters>().spawned;
                    });
            }
        });
    registry.addSystem<Position, const Velocity>(Stage::SIMULATION,
        "movement",
        [](Registry &registry, SparseSet<Position> &,
            const SparseSet<Velocity> &) {
            const float dt = registry.getResource<DeltaTime>().seconds;

            for (auto &&[position, velocity]:
                registry.view<Position, const Velocity>()) {
                position.x += velocity.dx * dt;
                position.y += velocity.dy * dt;
            }
        });
    registry.addSystem<Lifetime>(Stage::SIMULATION, "lifetime",
        [](Registry &registry, SparseSet<Lifetime> &lifetimes) {
            const float dt = registry.getResource<DeltaTime>().seconds;
            Counters &counters = registry.getResource<Counters>();

            for (std::size_t i = 0; i < lifetimes.size(); ++i) {
                Lifetime &lifetime = lifetimes.components()[i];

                lifetime.seconds -= dt;
                if (lifetime.seconds > 0.0F)
                    continue;
                registry.commands().kill(lifetimes.entities()[i]);
                ++counters.killed;
            }
        });
    registry.addSystem<const Position, const Radius>(Stage::SIMULATION,
        "collision",
        [](Registry &registry, const SparseSet<Position> &positions,
            const SparseSet<Radius> &radii) {
            const auto &entities = positions.entities();
            const auto &components = positions.components();
            std::unordered_set<Entity> doomed;

            for (std::size_t i = 0; i < entities.size(); ++i) {
                const Radius *first = radii.tryGet(entities[i]);
                if (first == nullptr)
                    continue;
                for (std::size_t j = i + 1; j < entities.size(); ++j) {
                    const Radius *second = radii.tryGet(entities[j]);
                    if (second == nullptr)
                        continue;
                    const float dx = components[i].x - components[j].x;
                    const float dy = components[i].y - components[j].y;
                    const float reach = first->value + second->value;
                    if ((dx * dx) + (dy * dy) >= reach * reach)
                        continue;
                    registry.commands().kill(entities[i]);
                    registry.commands().kill(entities[j]);
                    doomed.insert(entities[i]);
                    doomed.insert(entities[j]);
                }
            }
            registry.getResource<Counters>().killed += doomed.size();
        });
}

void runSteps(Registry &registry, int steps)
{
    for (int step = 0; step < steps; ++step)
        registry.runStage(Stage::SIMULATION);
}

template <class Component>
bool holdsOnlyLiveEntities(const Registry &registry)
{
    for (const Entity &entity: registry.getComponents<Component>().entities())
        if (!registry.isAlive(entity))
            return false;
    return true;
}

testing::AssertionResult worldIsConsistent(const Registry &registry)
{
    for (const Entity &entity: registry.getComponents<Position>().entities())
        if (registry.getComponent<Velocity>(entity) == nullptr)
            return testing::AssertionFailure() << "entity " << entity.index()
                                               << " has no Velocity";
    if (!holdsOnlyLiveEntities<Position>(registry) ||
        !holdsOnlyLiveEntities<Velocity>(registry) ||
        !holdsOnlyLiveEntities<Radius>(registry) ||
        !holdsOnlyLiveEntities<Lifetime>(registry) ||
        !holdsOnlyLiveEntities<Spawner>(registry))
        return testing::AssertionFailure()
            << "a storage holds a dead or stale entity";

    const Counters &counters = registry.getResource<Counters>();
    if (registry.aliveCount() != counters.spawned - counters.killed)
        return testing::AssertionFailure()
            << "aliveCount is " << registry.aliveCount() << ", expected "
            << counters.spawned << " - " << counters.killed;
    return testing::AssertionSuccess();
}

using Snapshot = std::vector<std::tuple<std::uint32_t, float, float>>;

Snapshot positionsByIndex(const Registry &registry)
{
    const SparseSet<Position> &positions = registry.getComponents<Position>();
    Snapshot snapshot;

    for (std::size_t i = 0; i < positions.size(); ++i)
        snapshot.emplace_back(positions.entities()[i].index(),
            positions.components()[i].x, positions.components()[i].y);
    std::ranges::sort(snapshot);
    return snapshot;
}

class EcsSimulation: public testing::Test {
protected:
    void SetUp() override
    {
        buildWorld(_registry, SEED);
    }

    Registry _registry;
};

struct ModelEntity {
    Entity handle;
    std::optional<Position> position;
    std::optional<Radius> radius;
};

/*
** The shadow model keys live entities by index, since Entity has no order,
** and keeps the handle to tell a stale one apart. The last MAX_HANDLES
** spawned handles stay in _handles, so kills, adds and removes also hit dead
** and stale ones; older handles are dropped to keep each check cheap.
*/
class EcsModelCheck: public testing::Test {
protected:
    void SetUp() override
    {
        _registry.registerComponent<Position>();
        _registry.registerComponent<Radius>();
        for (int i = 0; i < 10; ++i)
            spawn();
    }

    bool isAliveInModel(const Entity &handle) const
    {
        const auto found = _model.find(handle.index());

        return found != _model.end() && found->second.handle == handle;
    }

    Entity pickHandle()
    {
        if (!_model.empty() && _rng.chance(0.8F)) {
            const int index =
                _rng.uniformInt(0, static_cast<int>(_model.size()) - 1);
            return std::next(_model.begin(), index)->second.handle;
        }
        return _handles[static_cast<std::size_t>(
            _rng.uniformInt(0, static_cast<int>(_handles.size()) - 1))];
    }

    void spawn()
    {
        const Entity entity = _registry.spawnEntity();
        ModelEntity model{.handle = entity, .position = std::nullopt,
            .radius = std::nullopt};

        if (_rng.chance(0.5F))
            model.position = _registry.addComponent(entity,
                Position{.x = 1.0F, .y = 2.0F});
        if (_rng.chance(0.5F))
            model.radius = _registry.addComponent(entity, Radius{3.0F});
        _model.insert_or_assign(entity.index(), model);
        _handles.push_back(entity);
        if (_handles.size() > MAX_HANDLES)
            _handles.erase(_handles.begin());
    }

    void kill()
    {
        const Entity handle = pickHandle();

        _registry.killEntity(handle);
        if (isAliveInModel(handle))
            _model.erase(handle.index());
    }

    template <class Component>
    void add(const Component &value,
        std::optional<Component> ModelEntity::*field)
    {
        const Entity handle = pickHandle();
        const bool alive = isAliveInModel(handle);

        if (_rng.chance(0.5F)) {
            _registry.commands().add(handle, value);
            _registry.applyCommands();
        } else if (alive) {
            _registry.addComponent(handle, value);
        } else {
            EXPECT_THROW(_registry.addComponent(handle, value), EngineError);
        }
        if (alive)
            _model.at(handle.index()).*field = value;
    }

    template <class Component>
    void remove(std::optional<Component> ModelEntity::*field)
    {
        const Entity handle = pickHandle();
        const bool alive = isAliveInModel(handle);

        if (alive && _rng.chance(0.5F)) {
            _registry.removeComponent<Component>(handle);
        } else {
            _registry.commands().remove<Component>(handle);
            _registry.applyCommands();
        }
        if (alive)
            (_model.at(handle.index()).*field).reset();
    }

    void killFromView()
    {
        std::vector<Entity> doomed;

        for (auto &&[entity, position]:
            _registry.indexedView<const Position>()) {
            if (!_rng.chance(0.1F))
                continue;
            _registry.commands().kill(entity);
            doomed.push_back(entity);
        }
        _registry.applyCommands();
        for (const Entity &entity: doomed)
            _model.erase(entity.index());
    }

    void runOperation(int operation)
    {
        const auto value = static_cast<float>(operation);

        switch (_rng.uniformInt(0, 11)) {
            case 0:
            case 1:
            case 2:
                if (_model.size() < MAX_ALIVE)
                    spawn();
                else
                    kill();
                break;
            case 3:
            case 4:
                kill();
                break;
            case 5:
            case 6:
                add(Position{.x = value, .y = -value}, &ModelEntity::position);
                break;
            case 7:
            case 8:
                add(Radius{value}, &ModelEntity::radius);
                break;
            case 9:
                remove(&ModelEntity::position);
                break;
            case 10:
                remove(&ModelEntity::radius);
                break;
            default:
                killFromView();
        }
    }

    template <class... Components>
    std::vector<std::uint32_t> viewIndices()
    {
        std::vector<std::uint32_t> indices;

        for (auto &&item: _registry.indexedView<const Components...>())
            indices.push_back(std::get<0>(item).index());
        std::ranges::sort(indices);
        return indices;
    }

    template <class Predicate>
    std::vector<std::uint32_t> modelIndices(Predicate predicate) const
    {
        std::vector<std::uint32_t> indices;

        for (const auto &[index, entity]: _model)
            if (predicate(entity))
                indices.push_back(index);
        return indices;
    }

    testing::AssertionResult matchesModel()
    {
        if (_registry.aliveCount() != _model.size())
            return testing::AssertionFailure()
                << "aliveCount is " << _registry.aliveCount() << ", the model has "
                << _model.size();
        for (const Entity &handle: _handles)
            if (_registry.isAlive(handle) != isAliveInModel(handle))
                return testing::AssertionFailure()
                    << "liveness differs for index " << handle.index()
                    << ", generation " << handle.generation();
        for (const auto &[index, entity]: _model) {
            const Position *position =
                _registry.getComponent<Position>(entity.handle);
            const Radius *radius = _registry.getComponent<Radius>(entity.handle);

            if ((position != nullptr) != entity.position.has_value() ||
                (position != nullptr && (position->x != entity.position->x ||
                    position->y != entity.position->y)))
                return testing::AssertionFailure()
                    << "Position differs for index " << index;
            if ((radius != nullptr) != entity.radius.has_value() ||
                (radius != nullptr && radius->value != entity.radius->value))
                return testing::AssertionFailure()
                    << "Radius differs for index " << index;
        }

        const auto hasPosition = [](const ModelEntity &entity) {
            return entity.position.has_value();
        };
        const auto hasRadius = [](const ModelEntity &entity) {
            return entity.radius.has_value();
        };
        const auto hasBoth = [](const ModelEntity &entity) {
            return entity.position.has_value() && entity.radius.has_value();
        };
        if (viewIndices<Position>() != modelIndices(hasPosition) ||
            viewIndices<Radius>() != modelIndices(hasRadius) ||
            viewIndices<Position, Radius>() != modelIndices(hasBoth))
            return testing::AssertionFailure()
                << "a view yields other entities than the model";
        return testing::AssertionSuccess();
    }

    Registry _registry;
    Rng _rng{SEED};
    std::map<std::uint32_t, ModelEntity> _model;
    std::vector<Entity> _handles;
};
}

TEST_F(EcsSimulation, TheWorldStartsWithOnlyTheSpawner)
{
    EXPECT_EQ(_registry.aliveCount(), 1U);
    EXPECT_EQ(_registry.getResource<Counters>().spawned, 1U);
    EXPECT_EQ(_registry.getResource<Counters>().killed, 0U);
}

TEST_F(EcsSimulation, TheFirstStepSpawnsOneEntityThatMovesOnTheNext)
{
    _registry.runStage(Stage::SIMULATION);

    EXPECT_EQ(_registry.aliveCount(), 2U);
    EXPECT_EQ(_registry.getResource<Counters>().spawned, 2U);
    ASSERT_EQ(_registry.getComponents<Position>().size(), 1U);
    const Position start = _registry.getComponents<Position>().components()[0];
    const Velocity velocity =
        _registry.getComponents<Velocity>().components()[0];

    _registry.runStage(Stage::SIMULATION);

    const Entity moved = _registry.getComponents<Position>().entities()[0];
    const Position *position = _registry.getComponent<Position>(moved);
    ASSERT_NE(position, nullptr);
    EXPECT_FLOAT_EQ(position->x, start.x + (velocity.dx * STEP));
    EXPECT_FLOAT_EQ(position->y, start.y + (velocity.dy * STEP));
}

TEST_F(EcsSimulation, EntitiesDieWhenTheirLifetimeRunsOut)
{
    runSteps(_registry, 240);

    const Counters &counters = _registry.getResource<Counters>();
    EXPECT_GT(counters.killed, 0U);
    EXPECT_EQ(_registry.aliveCount(), counters.spawned - counters.killed);
    for (const Lifetime &lifetime:
        _registry.getComponents<Lifetime>().components())
        EXPECT_GT(lifetime.seconds, 0.0F);
}

TEST_F(EcsSimulation, EveryEntityOfAnOverlappingPairDiesOnce)
{
    std::vector<Entity> colliders;
    for (const float x: {-50.0F, -49.0F, -48.0F, -10.0F}) {
        const Entity entity = _registry.spawnEntity();
        _registry.addComponent(entity, Position{.x = x, .y = -50.0F});
        _registry.addComponent(entity, Velocity{.dx = 0.0F, .dy = 0.0F});
        _registry.addComponent(entity, Radius{1.0F});
        _registry.addComponent(entity, Lifetime{100.0F});
        colliders.push_back(entity);
    }
    _registry.getResource<Counters>().spawned += colliders.size();

    EXPECT_NO_THROW(_registry.runStage(Stage::SIMULATION));

    EXPECT_FALSE(_registry.isAlive(colliders[0]));
    EXPECT_FALSE(_registry.isAlive(colliders[1]));
    EXPECT_FALSE(_registry.isAlive(colliders[2]));
    EXPECT_TRUE(_registry.isAlive(colliders[3]));
    const Counters &counters = _registry.getResource<Counters>();
    EXPECT_EQ(counters.killed, 3U);
    EXPECT_EQ(_registry.aliveCount(), counters.spawned - counters.killed);
}

TEST_F(EcsSimulation, TheWorldStaysConsistentAfterEveryStep)
{
    for (int step = 0; step < STEPS; ++step) {
        _registry.runStage(Stage::SIMULATION);
        ASSERT_TRUE(worldIsConsistent(_registry)) << "after step " << step;
    }

    const Counters &counters = _registry.getResource<Counters>();
    EXPECT_GE(counters.spawned, 100U);
    EXPECT_GT(counters.killed, 0U);
}

TEST_F(EcsSimulation, TheSameSeedGivesTheSamePositions)
{
    Registry other;
    buildWorld(other, SEED);

    runSteps(_registry, STEPS);
    runSteps(other, STEPS);

    const Snapshot positions = positionsByIndex(_registry);
    EXPECT_FALSE(positions.empty());
    EXPECT_EQ(positions, positionsByIndex(other));
}

TEST_F(EcsSimulation, ADifferentSeedGivesDifferentPositions)
{
    Registry other;
    buildWorld(other, SEED + 1);

    runSteps(_registry, STEPS);
    runSteps(other, STEPS);

    EXPECT_NE(positionsByIndex(_registry), positionsByIndex(other));
}

TEST_F(EcsModelCheck, RandomOperationsMatchTheShadowModel)
{
    for (int operation = 0; operation < OPERATIONS; ++operation) {
        runOperation(operation);
        ASSERT_TRUE(matchesModel())
            << "seed " << SEED << ", operation " << operation;
    }
}
