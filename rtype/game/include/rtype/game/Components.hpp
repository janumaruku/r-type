/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Components
*/

#ifndef R_TYPE_COMPONENTS_HPP
#define R_TYPE_COMPONENTS_HPP

#include "Types.hpp"
#include "engine/core/Vec2.hpp"
#include "engine/ecs/Registry.hpp"

namespace rtype::game {
struct Transform {
    engine::core::Vec2f position;
};

struct Velocity {
    engine::core::Vec2f value;
};

struct Collider {
    engine::core::Vec2f halfSize{};
    CollisionLayer layer = CollisionLayer::PLAYER;
};

struct Health {
    std::int16_t current = 1;
    std::int16_t max = 1;
};

struct Damage {
    std::int16_t amount = 1;
};

struct Replicated {
    NetId netId = NO_NET_ID;
    EntityKind kind = EntityKind::PLAYER_SHIP;
    std::uint8_t variant = 0;
};

struct PlayerShip {
    std::uint8_t playerId = 0;
    InputButtons buttons;
    float invulnerableFor = 0.0F;
};

struct Weapon {
    float cooldown = 0.0F;
    float timer = 0.0F;
    engine::core::Vec2f muzzleOffset{};
    float projectileSpeed = 0.0F;
    engine::core::Vec2f aim{1.0F, 0.0F};
    bool triggered = false;
};

struct StraightMover {
    float speed = 0.0F;
};

struct SineMover {
    float speed = 0.0F;
    float baseY = 0.0F;
    float amplitude = 0.0F;
    float frequency = 0.0F;
    float age = 0.0F;
};

struct AimedShooter {
    FloatRange interval;
    float timer = 0.0F;
};

struct Projectile {
    NetId owner = NO_NET_ID;
    std::uint8_t ownerPlayerId = NO_PLAYER;
};

struct LastHit {
    NetId by = NO_NET_ID;
    std::uint8_t byPlayerId = NO_PLAYER;
};

struct Lifetime {
    float remaining = 0.0F;
};

struct ConfinedToPlayfield {
};

struct DespawnOutsidePlayfield {
    float margin = 0.0F;
};

struct ScoreValue {
    std::uint32_t points = 0;
};

void registerComponents(engine::ecs::Registry &registry);
}

#endif //R_TYPE_COMPONENTS_HPP
