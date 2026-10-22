/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RegistrySystemTests
*/

#include <gtest/gtest.h>

#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "Logger.hpp"
#include "engine/ecs/Registry.hpp"

using engine::core::log::Level;
using engine::ecs::Entity;
using engine::ecs::Registry;
using engine::ecs::SparseSet;
using engine::ecs::StageConcept;

namespace {
enum class Stage : std::uint8_t {
    INPUT,
    SIMULATION,
    RENDER,
};

enum class WideStage : int {
    ONLY,
};

enum PlainStage : std::uint8_t {
    PLAIN,
};

struct Position {
    int x;
};

struct Velocity {
    int dx;
};

struct Health {
    int points;
};

class RegistrySystem: public testing::Test {
protected:
    Registry _registry;
    std::vector<std::string> _calls;
};

/*
** The log level and output are process-wide: each test captures the output
** and gets the level and std::cerr back when it ends.
*/
class RegistrySystemLog: public RegistrySystem {
protected:
    void SetUp() override
    {
        _saved = engine::core::log::level();
        engine::core::log::setOutput(_output);
    }

    void TearDown() override
    {
        engine::core::log::setLevel(_saved);
        engine::core::log::setOutput(std::cerr);
    }

    Level _saved = Level::EG_INFO;
    std::ostringstream _output;
};

void movement(Registry &, SparseSet<Position> &positions,
    const SparseSet<Velocity> &velocities)
{
    for (std::size_t i = 0; i < positions.components().size(); ++i)
        positions.components()[i].x += velocities.components()[i].dx;
}

static_assert(StageConcept<Stage>);
static_assert(StageConcept<PlainStage>);
static_assert(!StageConcept<WideStage>);
static_assert(!StageConcept<std::uint8_t>);

static_assert(!std::is_invocable_v<void (*)(Registry &, SparseSet<Velocity> &),
    Registry &, const SparseSet<Velocity> &>);
static_assert(std::is_invocable_v<
    void (*)(Registry &, const SparseSet<Velocity> &), Registry &,
    SparseSet<Velocity> &>);
} // namespace

TEST_F(RegistrySystem, SystemsRunInInsertionOrder)
{
    for (const std::string name: {"first", "second", "third"})
        _registry.addSystem<>(Stage::SIMULATION, name,
            [this, name](Registry &) { _calls.push_back(name); });

    _registry.runStage(Stage::SIMULATION);

    EXPECT_EQ(_calls, (std::vector<std::string>{"first", "second", "third"}));
}

TEST_F(RegistrySystem, OnlyTheRequestedStageRuns)
{
    _registry.addSystem<>(Stage::INPUT, "input",
        [this](Registry &) { _calls.emplace_back("input"); });
    _registry.addSystem<>(Stage::SIMULATION, "simulation",
        [this](Registry &) { _calls.emplace_back("simulation"); });
    _registry.addSystem<>(Stage::RENDER, "render",
        [this](Registry &) { _calls.emplace_back("render"); });

    _registry.runStage(Stage::SIMULATION);

    EXPECT_EQ(_calls, (std::vector<std::string>{"simulation"}));
}

TEST_F(RegistrySystem, AStageWithNoSystemsDoesNothing)
{
    _registry.addSystem<>(Stage::RENDER, "render",
        [this](Registry &) { _calls.emplace_back("render"); });

    _registry.runStage(Stage::INPUT);
    _registry.runStage(Stage::SIMULATION);

    EXPECT_TRUE(_calls.empty());
    EXPECT_NO_THROW(Registry{}.runStage(Stage::RENDER));
}

TEST_F(RegistrySystem, AddSystemRegistersTheListedComponents)
{
    _registry.addSystem<Position, const Velocity>(Stage::SIMULATION,
        "movement", movement);

    EXPECT_NO_THROW(static_cast<void>(_registry.getComponents<Position>()));
    EXPECT_NO_THROW(static_cast<void>(_registry.getComponents<Velocity>()));
}

TEST_F(RegistrySystem, StoragesArriveInTheListedOrder)
{
    const SparseSet<Health> *health = nullptr;
    const SparseSet<Position> *position = nullptr;

    _registry.addSystem<Health, const Position>(Stage::SIMULATION, "check",
        [&health, &position](Registry &, SparseSet<Health> &first,
            const SparseSet<Position> &second) {
            health = &first;
            position = &second;
        });
    _registry.runStage(Stage::SIMULATION);

    EXPECT_EQ(health, &_registry.getComponents<Health>());
    EXPECT_EQ(position, &_registry.getComponents<Position>());
}

TEST_F(RegistrySystem, WritesThroughTheStoragesAreVisibleAfterwards)
{
    _registry.addSystem<Position, const Velocity>(Stage::SIMULATION,
        "movement", movement);
    const Entity entity = _registry.spawnEntity();
    _registry.addComponent(entity, Position{1});
    _registry.addComponent(entity, Velocity{2});

    _registry.runStage(Stage::SIMULATION);
    _registry.runStage(Stage::SIMULATION);

    EXPECT_EQ(_registry.getComponent<Position>(entity)->x, 5);
    EXPECT_EQ(_registry.getComponent<Velocity>(entity)->dx, 2);
}

TEST_F(RegistrySystem, AStatefulLambdaKeepsItsStateAcrossRuns)
{
    std::vector<int> seen;

    _registry.addSystem<>(Stage::SIMULATION, "counter",
        [&seen, count = 0](Registry &) mutable { seen.push_back(++count); });

    _registry.runStage(Stage::SIMULATION);
    _registry.runStage(Stage::SIMULATION);
    _registry.runStage(Stage::SIMULATION);

    EXPECT_EQ(seen, (std::vector<int>{1, 2, 3}));
}

TEST_F(RegistrySystem, ASystemCanSpawnAndKillEntities)
{
    _registry.registerComponent<Health>();
    const Entity doomed = _registry.spawnEntity();
    Entity spawned = Entity::NONE;

    _registry.addSystem<>(Stage::SIMULATION, "lifecycle",
        [doomed, &spawned](Registry &registry) {
            registry.killEntity(doomed);
            spawned = registry.spawnEntity();
            registry.addComponent(spawned, Health{3});
        });
    _registry.runStage(Stage::SIMULATION);

    EXPECT_FALSE(_registry.isAlive(doomed));
    ASSERT_TRUE(_registry.isAlive(spawned));
    EXPECT_EQ(_registry.getComponent<Health>(spawned)->points, 3);
    EXPECT_EQ(_registry.aliveCount(), 1U);
}

TEST_F(RegistrySystem, AThrowingSystemReachesTheCallerAndStopsTheStage)
{
    _registry.addSystem<>(Stage::SIMULATION, "before",
        [this](Registry &) { _calls.emplace_back("before"); });
    _registry.addSystem<>(Stage::SIMULATION, "thrower",
        [](Registry &) { throw std::runtime_error("system failed"); });
    _registry.addSystem<>(Stage::SIMULATION, "after",
        [this](Registry &) { _calls.emplace_back("after"); });

    EXPECT_THROW(_registry.runStage(Stage::SIMULATION), std::runtime_error);

    EXPECT_EQ(_calls, (std::vector<std::string>{"before"}));
}

TEST_F(RegistrySystem, TheNextRunWorksAfterASystemThrew)
{
    _registry.addSystem<>(Stage::RENDER, "thrower",
        [](Registry &) { throw std::runtime_error("system failed"); });
    _registry.addSystem<>(Stage::SIMULATION, "simulation",
        [this](Registry &) { _calls.emplace_back("simulation"); });

    EXPECT_THROW(_registry.runStage(Stage::RENDER), std::runtime_error);
    _registry.runStage(Stage::SIMULATION);
    _registry.addSystem<>(Stage::SIMULATION, "added after",
        [this](Registry &) { _calls.emplace_back("added after"); });
    _registry.runStage(Stage::SIMULATION);

    EXPECT_EQ(_calls,
        (std::vector<std::string>{"simulation", "simulation", "added after"}));
}

TEST_F(RegistrySystem, SystemsSurviveMovingTheRegistry)
{
    _registry.addSystem<>(Stage::SIMULATION, "simulation",
        [this](Registry &) { _calls.emplace_back("simulation"); });

    Registry moved = std::move(_registry);
    moved.runStage(Stage::SIMULATION);

    EXPECT_EQ(_calls, (std::vector<std::string>{"simulation"}));
}

TEST_F(RegistrySystemLog, AtTraceEachSystemLogsItsNameInOrder)
{
    engine::core::log::setLevel(Level::EG_TRACE);
    _registry.addSystem<Position, const Velocity>(Stage::SIMULATION,
        "movement", movement);
    _registry.addSystem<>(Stage::SIMULATION, "collision", [](Registry &) {});

    _registry.runStage(Stage::SIMULATION);

    const std::string text = _output.str();
    const auto movementLine = text.find("[ecs] movement took ");
    const auto collisionLine = text.find("[ecs] collision took ");
    ASSERT_NE(movementLine, std::string::npos);
    ASSERT_NE(collisionLine, std::string::npos);
    EXPECT_LT(movementLine, collisionLine);
}

TEST_F(RegistrySystemLog, BelowTraceNothingIsLogged)
{
    engine::core::log::setLevel(Level::EG_DEBUG);
    _registry.addSystem<Position, const Velocity>(Stage::SIMULATION,
        "movement", movement);

    _registry.runStage(Stage::SIMULATION);

    EXPECT_TRUE(_output.str().empty());
}

#ifndef NDEBUG
TEST_F(RegistrySystem, RunningAStageFromASystemAsserts)
{
    _registry.addSystem<>(Stage::SIMULATION, "nested",
        [](Registry &registry) { registry.runStage(Stage::RENDER); });

    EXPECT_DEATH(_registry.runStage(Stage::SIMULATION), "_running");
}

TEST_F(RegistrySystem, AddingASystemDuringARunAsserts)
{
    _registry.addSystem<>(Stage::SIMULATION, "adder",
        [](Registry &registry) {
            registry.addSystem<>(Stage::RENDER, "late", [](Registry &) {});
        });

    EXPECT_DEATH(_registry.runStage(Stage::SIMULATION), "_running");
}
#endif
