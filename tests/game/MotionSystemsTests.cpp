/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** MotionSystemsTests
*/

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <numbers>

#include "engine/core/Rng.hpp"
#include "engine/core/Vec2.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/Resources.hpp"
#include "rtype/game/Builders.hpp"
#include "rtype/game/Components.hpp"
#include "rtype/game/Motion.hpp"
#include "rtype/game/MotionSystems.hpp"
#include "rtype/game/NetIdAllocator.hpp"
#include "rtype/game/Rules.hpp"

using engine::core::Rng;
using engine::core::Vec2f;
using engine::ecs::DeltaTime;
using engine::ecs::Entity;
using engine::ecs::Registry;
using rtype::game::Collider;
using rtype::game::InputButtons;
using rtype::game::makeBydosSlave;
using rtype::game::makePlayerMissile;
using rtype::game::makePlayerShip;
using rtype::game::MovePattern;
using rtype::game::movePlayer;
using rtype::game::NetIdAllocator;
using rtype::game::PlayerShip;
using rtype::game::registerComponents;
using rtype::game::Rules;
using rtype::game::Transform;
using rtype::game::Velocity;
using rtype::game::Weapon;

namespace {

enum class Stage : std::uint8_t { SIMULATION };

constexpr float DT = 1.0F / 60.0F;
constexpr std::uint32_t SEED = 7;

}

/*
** A registry set up like GameWorld for these 3 systems: every component
** registered, the resources the builders and systems read, and the systems
** in their tick order.
*/
class MotionSystems: public testing::Test {
protected:
    void SetUp() override
    {
        registerComponents(_registry);
        _registry.setResource(Rules{});
        _registry.emplaceResource<Rng>(SEED);
        _registry.setResource(NetIdAllocator{});
        _registry.setResource(DeltaTime{});
        _registry.addSystem<PlayerShip, Velocity, Weapon>(Stage::SIMULATION,
            "playerInputSystem", rtype::game::playerInputSystem);
        _registry.addSystem<Transform, const Velocity>(Stage::SIMULATION,
            "movementSystem", rtype::game::movementSystem);
        _registry.addSystem<Transform, const Collider>(Stage::SIMULATION,
            "playfieldSystem", rtype::game::playfieldSystem);
    }

    void tick(const float dt = DT)
    {
        _registry.getResource<DeltaTime>().seconds = dt;
        _registry.runStage(Stage::SIMULATION);
    }

    Entity spawnShip()
    {
        const Entity ship = _registry.spawnEntity();

        (void)makePlayerShip(_registry, ship, 0);
        return ship;
    }

    void press(const Entity ship, const std::uint8_t buttons)
    {
        _registry.getComponent<PlayerShip>(ship)->buttons =
            InputButtons{buttons};
    }

    [[nodiscard]] Vec2f positionOf(const Entity entity)
    {
        return _registry.getComponent<Transform>(entity)->position;
    }

    Registry _registry;
    Rules _rules;
};

TEST_F(MotionSystems, RightMovesOneTickAtThePlayerSpeed)
{
    const Entity ship = spawnShip();
    const float startX = positionOf(ship).x();

    press(ship, InputButtons::RIGHT);
    tick();
    EXPECT_FLOAT_EQ(positionOf(ship).x(), startX + (320.0F / 60.0F));
    EXPECT_FLOAT_EQ(positionOf(ship).y(), _rules.playerSpawn[0].y());
}

TEST_F(MotionSystems, UpLeftIsADiagonalAtThePlayerSpeed)
{
    const Entity ship = spawnShip();

    press(ship, InputButtons::UP | InputButtons::LEFT);
    tick();
    const Vec2f velocity = _registry.getComponent<Velocity>(ship)->value;

    EXPECT_FLOAT_EQ(velocity.length(), 320.0F);
    EXPECT_FLOAT_EQ(velocity.x(), velocity.y());
    EXPECT_LT(velocity.x(), 0.0F);
    EXPECT_FLOAT_EQ(std::atan2(-velocity.y(), velocity.x()),
        3.0F * std::numbers::pi_v<float> / 4.0F);
}

TEST_F(MotionSystems, FireTriggersOnlyWhileHeld)
{
    const Entity ship = spawnShip();

    press(ship, InputButtons::FIRE);
    tick();
    EXPECT_TRUE(_registry.getComponent<Weapon>(ship)->triggered);
    press(ship, 0);
    tick();
    EXPECT_FALSE(_registry.getComponent<Weapon>(ship)->triggered);
}

TEST_F(MotionSystems, InvulnerabilityCountsDownToZero)
{
    const Entity ship = spawnShip();
    const auto *shipData = _registry.getComponent<PlayerShip>(ship);

    ASSERT_FLOAT_EQ(shipData->invulnerableFor, 2.0F);
    for (int i = 0; i < 130; ++i) {
        tick();
        EXPECT_GE(shipData->invulnerableFor, 0.0F) << "tick " << i + 1;
        if (i + 1 == 120)
            EXPECT_NEAR(shipData->invulnerableFor, 0.0F, 1e-4F);
    }
    EXPECT_EQ(shipData->invulnerableFor, 0.0F);
}

TEST_F(MotionSystems, ShipStopsAtTheLeftEdge)
{
    const Entity ship = spawnShip();

    press(ship, InputButtons::LEFT);
    for (int i = 0; i < 600; ++i) {
        tick();
        ASSERT_GE(positionOf(ship).x(), 16.0F) << "tick " << i + 1;
    }
    EXPECT_FLOAT_EQ(positionOf(ship).x(), 16.0F);
}

TEST_F(MotionSystems, ShipStopsAtTheTopEdge)
{
    const Entity ship = spawnShip();

    press(ship, InputButtons::UP);
    for (int i = 0; i < 600; ++i) {
        tick();
        ASSERT_GE(positionOf(ship).y(), 7.0F) << "tick " << i + 1;
    }
    EXPECT_FLOAT_EQ(positionOf(ship).y(), 7.0F);
}

TEST_F(MotionSystems, ClampedShipKeepsItsVelocity)
{
    const Entity ship = spawnShip();

    press(ship, InputButtons::LEFT);
    for (int i = 0; i < 600; ++i)
        tick();
    EXPECT_FLOAT_EQ(_registry.getComponent<Velocity>(ship)->value.x(),
        -320.0F);
}

TEST_F(MotionSystems, BydosJustOutsideOnTheRightIsKept)
{
    const Entity bydos = _registry.spawnEntity();

    (void)makeBydosSlave(_registry, bydos, {_rules.enemySpawnX, 360.0F},
        MovePattern::STRAIGHT);
    _registry.getComponent<Velocity>(bydos)->value = {0.0F, 0.0F};
    tick();
    EXPECT_TRUE(_registry.isAlive(bydos));
}

TEST_F(MotionSystems, MissilePastTheMarginIsKilled)
{
    const Entity missile = _registry.spawnEntity();
    const float limit = _rules.playfield.x() + _rules.despawnMargin + 6.0F;

    (void)makePlayerMissile(_registry, missile, 1, 0, {limit - 1.0F, 360.0F});
    _registry.getComponent<Velocity>(missile)->value = {0.0F, 0.0F};
    tick();
    EXPECT_TRUE(_registry.isAlive(missile));
    _registry.getComponent<Transform>(missile)->position = {limit + 1.0F,
        360.0F};
    tick();
    EXPECT_FALSE(_registry.isAlive(missile));
}

TEST_F(MotionSystems, MissileOffEverySideIsKilled)
{
    const float margin = _rules.despawnMargin + 10.0F;
    const std::array<Vec2f, 4> outside{{{-margin, 360.0F},
        {_rules.playfield.x() + margin, 360.0F}, {640.0F, -margin},
        {640.0F, _rules.playfield.y() + margin}}};

    for (const Vec2f position : outside) {
        const Entity missile = _registry.spawnEntity();

        (void)makePlayerMissile(_registry, missile, 1, 0, position);
        _registry.getComponent<Velocity>(missile)->value = {0.0F, 0.0F};
        tick();
        EXPECT_FALSE(_registry.isAlive(missile))
            << position.x() << ", " << position.y();
    }
}

TEST_F(MotionSystems, SystemsMatchMovePlayerAtEveryTick)
{
    const Entity ship = spawnShip();
    Rng buttons(SEED);
    Vec2f predicted = positionOf(ship);

    for (int i = 0; i < 600; ++i) {
        const auto held = static_cast<std::uint8_t>(buttons.uniformInt(0, 31));

        press(ship, held);
        tick();
        predicted = movePlayer(predicted, InputButtons{held}, DT, _rules);
        ASSERT_EQ(positionOf(ship).x(), predicted.x()) << "tick " << i + 1;
        ASSERT_EQ(positionOf(ship).y(), predicted.y()) << "tick " << i + 1;
    }
}
