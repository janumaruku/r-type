/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ComponentsTests
*/

#include <gtest/gtest.h>

#include "engine/ecs/Component.hpp"
#include "engine/ecs/Registry.hpp"
#include "rtype/game/Components.hpp"

using namespace rtype::game;

namespace {

template <class Component>
void expectStorage(engine::ecs::Registry &registry)
{
    EXPECT_NO_THROW(registry.getComponents<Component>());
}

template <class... Components>
struct ComponentList {
    static constexpr bool ALL_COMPONENTS =
        (engine::ecs::ComponentConcept<Components> && ...);

    static void expectRegistered(engine::ecs::Registry &registry)
    {
        (expectStorage<Components>(registry), ...);
    }
};

using AllComponents = ComponentList<Transform, Velocity, Collider, Health,
    Damage, Replicated, PlayerShip, Weapon, StraightMover, SineMover,
    AimedShooter, Projectile, LastHit, Lifetime, ConfinedToPlayfield,
    DespawnOutsidePlayfield, ScoreValue>;

static_assert(AllComponents::ALL_COMPONENTS);

}

TEST(Components, RegisterComponentsRegistersAllSeventeen)
{
    engine::ecs::Registry registry;

    registerComponents(registry);

    AllComponents::expectRegistered(registry);
}

TEST(Components, RegisterComponentsTwiceKeepsTheData)
{
    engine::ecs::Registry registry;
    registerComponents(registry);
    const auto entity = registry.spawnEntity();
    registry.addComponent(entity, Health{.current = 3, .max = 5});
    registry.addComponent(entity, ScoreValue{.points = 100});

    EXPECT_NO_THROW(registerComponents(registry));

    const auto *health = registry.getComponent<Health>(entity);
    ASSERT_NE(health, nullptr);
    EXPECT_EQ(health->current, 3);
    EXPECT_EQ(health->max, 5);
    const auto *score = registry.getComponent<ScoreValue>(entity);
    ASSERT_NE(score, nullptr);
    EXPECT_EQ(score->points, 100U);
}

TEST(Components, WeaponAimsForwardByDefault)
{
    const Weapon weapon{};

    EXPECT_FLOAT_EQ(weapon.aim.x(), 1.0F);
    EXPECT_FLOAT_EQ(weapon.aim.y(), 0.0F);
    EXPECT_FALSE(weapon.triggered);
}

TEST(Components, ProjectileAndLastHitDefaultToNobody)
{
    const Projectile projectile{};
    const LastHit lastHit{};

    EXPECT_EQ(projectile.owner, NO_NET_ID);
    EXPECT_EQ(projectile.ownerPlayerId, NO_PLAYER);
    EXPECT_EQ(lastHit.by, NO_NET_ID);
    EXPECT_EQ(lastHit.byPlayerId, NO_PLAYER);
}
