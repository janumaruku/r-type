/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** EcsIntegrationTests
*/

#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>
#include <unordered_set>

#include "engine/core/Rng.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/Resources.hpp"
#include "engine/ecs/SparseSet.hpp"

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
        [](Registry &, SparseSet<Lifetime> &) {
        });
    registry.addSystem<const Position, const Radius>(Stage::SIMULATION,
        "collision",
        [](Registry &, const SparseSet<Position> &,
            const SparseSet<Radius> &) {
        });
}

class EcsSimulation: public testing::Test {
protected:
    void SetUp() override
    {
        buildWorld(_registry, SEED);
    }

    Registry _registry;
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
