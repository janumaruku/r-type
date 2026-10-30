/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** BuildersTests
*/

#include <gtest/gtest.h>

#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "engine/core/Errors.hpp"
#include "engine/core/Rng.hpp"
#include "engine/core/Vec2.hpp"
#include "engine/ecs/Registry.hpp"
#include "rtype/game/Builders.hpp"
#include "rtype/game/Components.hpp"
#include "rtype/game/NetIdAllocator.hpp"
#include "rtype/game/Rules.hpp"

using engine::core::Rng;
using engine::core::Vec2f;
using engine::ecs::Entity;
using engine::ecs::Registry;
using rtype::game::AimedShooter;
using rtype::game::Collider;
using rtype::game::CollisionLayer;
using rtype::game::ConfinedToPlayfield;
using rtype::game::Damage;
using rtype::game::DespawnOutsidePlayfield;
using rtype::game::EntityKind;
using rtype::game::FloatRange;
using rtype::game::Health;
using rtype::game::LastHit;
using rtype::game::Lifetime;
using rtype::game::makeBydosSlave;
using rtype::game::makeEnemyMissile;
using rtype::game::makePlayerMissile;
using rtype::game::makePlayerShip;
using rtype::game::MovePattern;
using rtype::game::NetId;
using rtype::game::NetIdAllocator;
using rtype::game::NO_PLAYER;
using rtype::game::PlayerShip;
using rtype::game::Projectile;
using rtype::game::registerComponents;
using rtype::game::Replicated;
using rtype::game::Rules;
using rtype::game::ScoreValue;
using rtype::game::SineMover;
using rtype::game::StraightMover;
using rtype::game::Transform;
using rtype::game::Velocity;
using rtype::game::Weapon;

namespace {

constexpr std::uint32_t SEED = 42;
constexpr NetId SHOOTER = 99;

void setUpWorld(Registry &registry)
{
    registerComponents(registry);
    registry.setResource(Rules{});
    registry.emplaceResource<Rng>(SEED);
    registry.setResource(NetIdAllocator{});
}

template <class Component>
void collect(const Registry &registry, const Entity entity,
    const std::string &name, std::vector<std::string> &found)
{
    if (registry.getComponent<Component>(entity) != nullptr)
        found.push_back(name);
}

/*
** The names of the components the entity has, among the 17 game types, in
** the order of the component table.
*/
std::vector<std::string> componentsOf(const Registry &registry,
    const Entity entity)
{
    std::vector<std::string> found;

    collect<Transform>(registry, entity, "Transform", found);
    collect<Velocity>(registry, entity, "Velocity", found);
    collect<Collider>(registry, entity, "Collider", found);
    collect<Health>(registry, entity, "Health", found);
    collect<Damage>(registry, entity, "Damage", found);
    collect<Replicated>(registry, entity, "Replicated", found);
    collect<PlayerShip>(registry, entity, "PlayerShip", found);
    collect<Weapon>(registry, entity, "Weapon", found);
    collect<StraightMover>(registry, entity, "StraightMover", found);
    collect<SineMover>(registry, entity, "SineMover", found);
    collect<AimedShooter>(registry, entity, "AimedShooter", found);
    collect<Projectile>(registry, entity, "Projectile", found);
    collect<LastHit>(registry, entity, "LastHit", found);
    collect<Lifetime>(registry, entity, "Lifetime", found);
    collect<ConfinedToPlayfield>(registry, entity, "ConfinedToPlayfield",
        found);
    collect<DespawnOutsidePlayfield>(registry, entity,
        "DespawnOutsidePlayfield", found);
    collect<ScoreValue>(registry, entity, "ScoreValue", found);
    return found;
}

void expectVec(const Vec2f actual, const Vec2f expected)
{
    EXPECT_FLOAT_EQ(actual.x(), expected.x());
    EXPECT_FLOAT_EQ(actual.y(), expected.y());
}

void expectInRange(const float value, const FloatRange range)
{
    EXPECT_GE(value, range.min);
    EXPECT_LT(value, range.max);
}

}

class Builders: public testing::Test {
protected:
    void SetUp() override
    {
        setUpWorld(_registry);
    }

    Registry _registry;
    Rules _rules;
};

TEST_F(Builders, PlayerShipHasExactlyItsRow)
{
    const Entity ship = _registry.spawnEntity();
    const NetId netId = makePlayerShip(_registry, ship, 0);
    const std::vector<std::string> expected{"Transform", "Velocity",
        "Collider", "Health", "Damage", "Replicated", "PlayerShip", "Weapon",
        "ConfinedToPlayfield"};

    EXPECT_EQ(componentsOf(_registry, ship), expected);
    expectVec(_registry.getComponent<Collider>(ship)->halfSize, {16.0F, 7.0F});
    EXPECT_EQ(_registry.getComponent<Collider>(ship)->layer,
        CollisionLayer::PLAYER);
    expectVec(_registry.getComponent<Velocity>(ship)->value, {0.0F, 0.0F});
    EXPECT_EQ(_registry.getComponent<Replicated>(ship)->netId, netId);
    EXPECT_EQ(_registry.getComponent<Replicated>(ship)->kind,
        EntityKind::PLAYER_SHIP);
}

TEST_F(Builders, BydosSlaveHasExactlyItsRow)
{
    const Entity bydos = _registry.spawnEntity();
    const NetId netId = makeBydosSlave(_registry, bydos, {1312.0F, 300.0F},
        MovePattern::STRAIGHT);
    const std::vector<std::string> expected{"Transform", "Velocity",
        "Collider", "Health", "Damage", "Replicated", "Weapon",
        "StraightMover", "AimedShooter", "DespawnOutsidePlayfield",
        "ScoreValue"};

    EXPECT_EQ(componentsOf(_registry, bydos), expected);
    expectVec(_registry.getComponent<Collider>(bydos)->halfSize,
        {10.5F, 12.0F});
    EXPECT_EQ(_registry.getComponent<Collider>(bydos)->layer,
        CollisionLayer::ENEMY);
    EXPECT_EQ(_registry.getComponent<ScoreValue>(bydos)->points,
        _rules.killScore);
    EXPECT_EQ(_registry.getComponent<Replicated>(bydos)->netId, netId);
    EXPECT_EQ(_registry.getComponent<Replicated>(bydos)->kind,
        EntityKind::BYDOS_SLAVE);
}

TEST_F(Builders, PlayerMissileHasExactlyItsRow)
{
    const Entity missile = _registry.spawnEntity();
    const NetId netId = makePlayerMissile(_registry, missile, SHOOTER, 2,
        {136.0F, 144.0F});
    const std::vector<std::string> expected{"Transform", "Velocity",
        "Collider", "Health", "Damage", "Replicated", "Projectile",
        "Lifetime", "DespawnOutsidePlayfield"};

    EXPECT_EQ(componentsOf(_registry, missile), expected);
    expectVec(_registry.getComponent<Collider>(missile)->halfSize,
        {6.0F, 2.0F});
    EXPECT_EQ(_registry.getComponent<Collider>(missile)->layer,
        CollisionLayer::PLAYER_SHOT);
    expectVec(_registry.getComponent<Velocity>(missile)->value,
        {_rules.playerMissileSpeed, 0.0F});
    EXPECT_EQ(_registry.getComponent<Projectile>(missile)->owner, SHOOTER);
    EXPECT_EQ(_registry.getComponent<Projectile>(missile)->ownerPlayerId, 2);
    EXPECT_FLOAT_EQ(_registry.getComponent<Lifetime>(missile)->remaining,
        _rules.playerMissileLifetime);
    EXPECT_EQ(_registry.getComponent<Replicated>(missile)->netId, netId);
    EXPECT_EQ(_registry.getComponent<Replicated>(missile)->kind,
        EntityKind::PLAYER_MISSILE);
}

TEST_F(Builders, EnemyMissileHasExactlyItsRow)
{
    const Entity missile = _registry.spawnEntity();
    const NetId netId = makeEnemyMissile(_registry, missile, SHOOTER,
        {600.0F, 300.0F}, {-1.0F, 0.0F});
    const std::vector<std::string> expected{"Transform", "Velocity",
        "Collider", "Health", "Damage", "Replicated", "Projectile",
        "Lifetime", "DespawnOutsidePlayfield"};

    EXPECT_EQ(componentsOf(_registry, missile), expected);
    expectVec(_registry.getComponent<Collider>(missile)->halfSize,
        {3.5F, 3.0F});
    EXPECT_EQ(_registry.getComponent<Collider>(missile)->layer,
        CollisionLayer::ENEMY_SHOT);
    EXPECT_EQ(_registry.getComponent<Projectile>(missile)->owner, SHOOTER);
    EXPECT_EQ(_registry.getComponent<Projectile>(missile)->ownerPlayerId,
        NO_PLAYER);
    EXPECT_FLOAT_EQ(_registry.getComponent<Lifetime>(missile)->remaining,
        _rules.enemyMissileLifetime);
    EXPECT_EQ(_registry.getComponent<Replicated>(missile)->netId, netId);
    EXPECT_EQ(_registry.getComponent<Replicated>(missile)->kind,
        EntityKind::ENEMY_MISSILE);
}

TEST_F(Builders, BydosSlaveGetsTheMoverOfItsPattern)
{
    const Entity straight = _registry.spawnEntity();
    const Entity sine = _registry.spawnEntity();

    (void)makeBydosSlave(_registry, straight, {1312.0F, 200.0F},
        MovePattern::STRAIGHT);
    (void)makeBydosSlave(_registry, sine, {1312.0F, 450.0F},
        MovePattern::SINE);

    EXPECT_NE(_registry.getComponent<StraightMover>(straight), nullptr);
    EXPECT_EQ(_registry.getComponent<SineMover>(straight), nullptr);
    EXPECT_NE(_registry.getComponent<AimedShooter>(straight), nullptr);
    EXPECT_EQ(_registry.getComponent<Replicated>(straight)->variant, 0);

    ASSERT_NE(_registry.getComponent<SineMover>(sine), nullptr);
    EXPECT_EQ(_registry.getComponent<StraightMover>(sine), nullptr);
    EXPECT_FLOAT_EQ(_registry.getComponent<SineMover>(sine)->baseY, 450.0F);
    EXPECT_FLOAT_EQ(_registry.getComponent<SineMover>(sine)->age, 0.0F);
    EXPECT_NE(_registry.getComponent<AimedShooter>(sine), nullptr);
    EXPECT_EQ(_registry.getComponent<Replicated>(sine)->variant, 1);
}

TEST_F(Builders, PlayerShipsForEverySlot)
{
    std::set<NetId> netIds;

    for (std::uint8_t playerId = 0; playerId < 4; ++playerId) {
        SCOPED_TRACE(static_cast<int>(playerId));
        const Entity ship = _registry.spawnEntity();

        netIds.insert(makePlayerShip(_registry, ship, playerId));
        expectVec(_registry.getComponent<Transform>(ship)->position,
            _rules.playerSpawn[playerId]);
        EXPECT_EQ(_registry.getComponent<Replicated>(ship)->variant, playerId);
        EXPECT_EQ(_registry.getComponent<PlayerShip>(ship)->playerId,
            playerId);
        EXPECT_FLOAT_EQ(
            _registry.getComponent<PlayerShip>(ship)->invulnerableFor, 2.0F);
        expectVec(_registry.getComponent<Weapon>(ship)->muzzleOffset,
            {16.0F, 0.0F});
    }
    EXPECT_EQ(netIds.size(), 4U);
}

TEST_F(Builders, PlayerShipRejectsAnIdOutOfRange)
{
    const Entity ship = _registry.spawnEntity();

    EXPECT_THROW((void)makePlayerShip(_registry, ship, 4),
        engine::core::EngineError);
    EXPECT_TRUE(componentsOf(_registry, ship).empty());
    EXPECT_EQ(_registry.getResource<NetIdAllocator>().size(), 0U);
}

TEST(BuildersResources, AMissingResourceThrows)
{
    Registry registry;

    registerComponents(registry);
    registry.setResource(NetIdAllocator{});
    EXPECT_THROW((void)makePlayerShip(registry, registry.spawnEntity(), 0),
        engine::core::EngineError);
}

TEST_F(Builders, BydosRandomValuesStayInTheirRanges)
{
    for (int i = 0; i < 1000; ++i) {
        const Entity bydos = _registry.spawnEntity();
        const auto pattern =
            (i % 2 == 0) ? MovePattern::STRAIGHT : MovePattern::SINE;

        (void)makeBydosSlave(_registry, bydos, {1312.0F, 360.0F}, pattern);
        expectInRange(_registry.getComponent<AimedShooter>(bydos)->timer,
            _rules.enemyFireInterval);
        if (pattern == MovePattern::STRAIGHT) {
            expectInRange(_registry.getComponent<StraightMover>(bydos)->speed,
                _rules.enemySpeed);
            continue;
        }
        const SineMover *mover = _registry.getComponent<SineMover>(bydos);

        expectInRange(mover->speed, _rules.enemySpeed);
        expectInRange(mover->amplitude, _rules.sineAmplitude);
        expectInRange(mover->frequency, _rules.sineFrequency);
    }
}

TEST(BuildersDeterminism, SameSeedSameCallsSameComponents)
{
    Registry first;
    Registry second;
    std::vector<Entity> firstEntities;
    std::vector<Entity> secondEntities;

    setUpWorld(first);
    setUpWorld(second);
    for (int i = 0; i < 10; ++i) {
        const auto pattern =
            (i % 3 == 0) ? MovePattern::SINE : MovePattern::STRAIGHT;
        const Vec2f position{1312.0F, 60.0F * static_cast<float>(i + 1)};

        firstEntities.push_back(first.spawnEntity());
        secondEntities.push_back(second.spawnEntity());
        (void)makeBydosSlave(first, firstEntities.back(), position, pattern);
        (void)makeBydosSlave(second, secondEntities.back(), position,
            pattern);
    }
    for (std::size_t i = 0; i < firstEntities.size(); ++i) {
        SCOPED_TRACE(i);
        const Entity a = firstEntities[i];
        const Entity b = secondEntities[i];

        EXPECT_EQ(componentsOf(first, a), componentsOf(second, b));
        expectVec(first.getComponent<Velocity>(a)->value,
            second.getComponent<Velocity>(b)->value);
        EXPECT_EQ(first.getComponent<AimedShooter>(a)->timer,
            second.getComponent<AimedShooter>(b)->timer);
        EXPECT_EQ(first.getComponent<Replicated>(a)->netId,
            second.getComponent<Replicated>(b)->netId);
        if (const auto *mover = first.getComponent<SineMover>(a)) {
            const auto *other = second.getComponent<SineMover>(b);

            ASSERT_NE(other, nullptr);
            EXPECT_EQ(mover->speed, other->speed);
            EXPECT_EQ(mover->amplitude, other->amplitude);
            EXPECT_EQ(mover->frequency, other->frequency);
        } else {
            ASSERT_NE(second.getComponent<StraightMover>(b), nullptr);
            EXPECT_EQ(first.getComponent<StraightMover>(a)->speed,
                second.getComponent<StraightMover>(b)->speed);
        }
    }
}

TEST_F(Builders, EnemyMissileNormalisesItsDirection)
{
    const Entity missile = _registry.spawnEntity();

    (void)makeEnemyMissile(_registry, missile, SHOOTER, {600.0F, 300.0F},
        {3.0F, 4.0F});
    expectVec(_registry.getComponent<Velocity>(missile)->value,
        {252.0F, 336.0F});
}

TEST_F(Builders, EveryReturnedNetIdFindsItsEntity)
{
    const Entity ship = _registry.spawnEntity();
    const Entity bydos = _registry.spawnEntity();
    const Entity playerMissile = _registry.spawnEntity();
    const Entity enemyMissile = _registry.spawnEntity();
    const auto &allocator = _registry.getResource<NetIdAllocator>();

    const NetId shipId = makePlayerShip(_registry, ship, 1);
    const NetId bydosId = makeBydosSlave(_registry, bydos, {1312.0F, 300.0F},
        MovePattern::SINE);
    const NetId playerMissileId = makePlayerMissile(_registry, playerMissile,
        shipId, 1, {136.0F, 288.0F});
    const NetId enemyMissileId = makeEnemyMissile(_registry, enemyMissile,
        bydosId, {1300.0F, 300.0F}, {-1.0F, 0.0F});

    EXPECT_EQ(allocator.find(shipId, _registry), ship);
    EXPECT_EQ(allocator.find(bydosId, _registry), bydos);
    EXPECT_EQ(allocator.find(playerMissileId, _registry), playerMissile);
    EXPECT_EQ(allocator.find(enemyMissileId, _registry), enemyMissile);
}
