/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** WeaponTests
*/

#include <gtest/gtest.h>

#include <set>

#include "GameSystemsFixture.hpp"

using engine::core::Vec2f;
using engine::ecs::Entity;
using rtype::game::EntityKind;
using rtype::game::Fired;
using rtype::game::InputButtons;
using rtype::game::NetId;
using rtype::game::PlayerShip;
using rtype::game::Projectile;
using rtype::game::Replicated;
using rtype::game::Side;
using rtype::game::Velocity;
using rtype::game::Weapon;

class Weapons: public game_tests::GameSystems {
protected:
    void expectPlayerMissile(const Fired &event, const Entity ship,
        const Vec2f muzzle)
    {
        const Entity missile = entityOf(event.missile);

        EXPECT_EQ(component<Replicated>(missile).kind,
            EntityKind::PLAYER_MISSILE);
        EXPECT_EQ(component<Projectile>(missile).owner, netIdOf(ship));
        EXPECT_EQ(event.shooter, netIdOf(ship));
        EXPECT_EQ(event.side, Side::PLAYER);
        EXPECT_EQ(event.position, muzzle);
    }
};

TEST_F(Weapons, FireHeldOneSecondFiresFiveMissiles)
{
    const Entity ship = spawnAliveShip();
    const Vec2f muzzle = _rules.playerSpawn[0] + _rules.playerMuzzleOffset;

    addPlayerInputSystem();
    addWeaponSystem();
    component<PlayerShip>(ship).buttons = InputButtons{InputButtons::FIRE};
    tick(60);

    const auto fired = eventsOf<Fired>();
    std::set<NetId> missiles;

    ASSERT_EQ(fired.size(), 5U);
    EXPECT_EQ(countOf<Projectile>(), 5U);
    for (const Fired &event : fired) {
        expectPlayerMissile(event, ship, muzzle);
        missiles.insert(event.missile);
    }
    EXPECT_EQ(missiles.size(), 5U);
}

TEST_F(Weapons, ShipWithoutFireDoesNotShoot)
{
    (void)spawnAliveShip();

    addPlayerInputSystem();
    addWeaponSystem();
    tick(60);
    EXPECT_TRUE(eventsOf<Fired>().empty());
    EXPECT_EQ(countOf<Projectile>(), 0U);
}

TEST_F(Weapons, EnemyFiresOnceInItsAimedDirection)
{
    const Entity bydos = spawnBydos({800.0F, 360.0F});
    const Vec2f aim{-0.6F, 0.8F};

    addWeaponSystem();
    component<Weapon>(bydos).aim = aim;
    component<Weapon>(bydos).triggered = true;
    tick();

    const auto fired = eventsOf<Fired>();

    ASSERT_EQ(fired.size(), 1U);
    EXPECT_FALSE(component<Weapon>(bydos).triggered);
    EXPECT_EQ(fired[0].shooter, netIdOf(bydos));
    EXPECT_EQ(fired[0].side, Side::ENEMY);

    const Entity missile = entityOf(fired[0].missile);

    EXPECT_EQ(component<Replicated>(missile).kind,
        EntityKind::ENEMY_MISSILE);
    EXPECT_FLOAT_EQ(component<Velocity>(missile).value.x(),
        aim.x() * _rules.enemyMissileSpeed);
    EXPECT_FLOAT_EQ(component<Velocity>(missile).value.y(),
        aim.y() * _rules.enemyMissileSpeed);

    tick();
    EXPECT_TRUE(eventsOf<Fired>().empty());
    EXPECT_EQ(countOf<Projectile>(), 1U);
}
