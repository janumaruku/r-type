/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** EnemyBehaviourTests
*/

#include <gtest/gtest.h>

#include <cmath>

#include "GameSystemsFixture.hpp"

using engine::core::Vec2f;
using engine::ecs::Entity;
using game_tests::DT;
using rtype::game::AimedShooter;
using rtype::game::SineMover;
using rtype::game::StraightMover;
using rtype::game::Transform;
using rtype::game::Velocity;
using rtype::game::Weapon;

namespace {

constexpr float BASE_Y = 360.0F;
constexpr float AMPLITUDE = 80.0F;

}

class EnemyBehaviour: public game_tests::GameSystems {
protected:
    Entity spawnSineMover()
    {
        const Entity entity = _registry.spawnEntity();

        _registry.addComponent(entity,
            Transform{.position = {640.0F, BASE_Y}});
        _registry.addComponent(entity, Velocity{});
        _registry.addComponent(entity,
            SineMover{.speed = 200.0F, .baseY = BASE_Y,
                .amplitude = AMPLITUDE, .frequency = 0.5F, .age = 0.0F});
        return entity;
    }
};

TEST_F(EnemyBehaviour, StraightMoverMovesLeftAtItsSpeed)
{
    const Entity entity = _registry.spawnEntity();

    addStraightMoverSystem();
    _registry.addComponent(entity, Velocity{});
    _registry.addComponent(entity, StraightMover{.speed = 200.0F});
    tick();
    EXPECT_FLOAT_EQ(component<Velocity>(entity).value.x(), -200.0F);
    EXPECT_FLOAT_EQ(component<Velocity>(entity).value.y(), 0.0F);
}

TEST_F(EnemyBehaviour, SineMoverStaysWithinItsAmplitudeOverOnePeriod)
{
    const Entity entity = spawnSineMover();

    addSineMoverSystem();
    addMovementSystem();
    for (int i = 0; i < 120; ++i) {
        tick();
        ASSERT_LE(std::abs(component<Transform>(entity).position.y() - BASE_Y),
            AMPLITUDE + 1e-3F) << "tick " << i + 1;
    }
    EXPECT_NEAR(component<Transform>(entity).position.y(), BASE_Y, 0.5F);
    EXPECT_FLOAT_EQ(component<Transform>(entity).position.x(),
        640.0F - (200.0F * 2.0F));
}

TEST_F(EnemyBehaviour, SineMoverReachesItsPeakAfterAQuarterPeriod)
{
    const Entity entity = spawnSineMover();

    addSineMoverSystem();
    addMovementSystem();
    tick(30);
    EXPECT_NEAR(component<Transform>(entity).position.y(), BASE_Y + AMPLITUDE,
        0.01F);
}

TEST_F(EnemyBehaviour, SineMoverWinsOverStraightMoverOnTheSameEntity)
{
    const Entity entity = spawnSineMover();

    addStraightMoverSystem();
    addSineMoverSystem();
    _registry.addComponent(entity, StraightMover{.speed = 100.0F});
    tick();
    EXPECT_FLOAT_EQ(component<Velocity>(entity).value.x(), -200.0F);
    EXPECT_GT(component<Velocity>(entity).value.y(), 0.0F);
}

TEST_F(EnemyBehaviour, ShooterAimsAtTheNearerShipFromItsMuzzle)
{
    const Entity farther = spawnAliveShip();
    const Entity nearer = spawnAliveShip();
    const Vec2f position{800.0F, 550.0F};
    const Entity bydos = spawnBydos(position);

    addAimedShooterSystem();
    moveTo(farther, {200.0F, 100.0F});
    moveTo(nearer, {200.0F, 600.0F});
    component<AimedShooter>(bydos).timer = DT / 2.0F;
    tick();

    const Vec2f muzzle = position + _rules.enemyMuzzleOffset;
    const Vec2f expected = (Vec2f{200.0F, 600.0F} - muzzle).normalized();
    const Weapon &weapon = component<Weapon>(bydos);
    const float timer = component<AimedShooter>(bydos).timer;

    EXPECT_TRUE(weapon.triggered);
    EXPECT_FLOAT_EQ(weapon.aim.x(), expected.x());
    EXPECT_FLOAT_EQ(weapon.aim.y(), expected.y());
    EXPECT_GE(timer, _rules.enemyFireInterval.min);
    EXPECT_LT(timer, _rules.enemyFireInterval.max);
}

TEST_F(EnemyBehaviour, ShooterWithNoShipAliveDrawsATimerWithoutFiring)
{
    const Entity bydos = spawnBydos({800.0F, 360.0F});

    addAimedShooterSystem();
    component<AimedShooter>(bydos).timer = DT / 2.0F;
    tick();

    const Weapon &weapon = component<Weapon>(bydos);
    const float timer = component<AimedShooter>(bydos).timer;

    EXPECT_FALSE(weapon.triggered);
    EXPECT_FLOAT_EQ(weapon.aim.x(), -1.0F);
    EXPECT_FLOAT_EQ(weapon.aim.y(), 0.0F);
    EXPECT_GE(timer, _rules.enemyFireInterval.min);
    EXPECT_LT(timer, _rules.enemyFireInterval.max);
}

TEST_F(EnemyBehaviour, ShooterWaitsUntilItsTimerExpires)
{
    (void)spawnAliveShip();
    const Entity bydos = spawnBydos({800.0F, 360.0F});

    addAimedShooterSystem();
    component<AimedShooter>(bydos).timer = 1.0F;
    tick();
    EXPECT_FALSE(component<Weapon>(bydos).triggered);
    EXPECT_FLOAT_EQ(component<AimedShooter>(bydos).timer, 1.0F - DT);
}
