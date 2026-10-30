/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Builders
*/

#include "rtype/game/Builders.hpp"
#include <cassert>
#include <format>

#include "engine/core/Errors.hpp"
#include "engine/core/Rng.hpp"
#include "rtype/game/Components.hpp"
#include "rtype/game/Rules.hpp"
#include "rtype/game/NetIdAllocator.hpp"

namespace rtype::game {

NetId makePlayerShip(engine::ecs::Registry &registry,
    const engine::ecs::Entity entity, const std::uint8_t playerId)
{
    const auto &rules = registry.getResource<Rules>();

    if (playerId >= rules.playerSpawn.size())
        throw engine::core::EngineError(std::format(
            "makePlayerShip: player id {} is out of range",
            static_cast<unsigned>(playerId)));

    const NetId netId = registry.getResource<NetIdAllocator>().allocate(entity);

    registry.addComponent(
        entity,
        Transform{.position = rules.playerSpawn[playerId]}
    );
    registry.addComponent(entity, Velocity{});
    registry.addComponent(
        entity, Collider{
            .halfSize = rules.playerHalfSize,
            .layer = CollisionLayer::PLAYER,
        }
    );
    registry.addComponent(entity, Health{.current = 1, .max = 1});
    registry.addComponent(entity, Damage{.amount = 1});
    registry.addComponent(
        entity,
        Replicated{
            .netId = netId,
            .kind = EntityKind::PLAYER_SHIP,
            .variant = playerId,
        }
    );
    registry.addComponent(
        entity, PlayerShip{
            .playerId = playerId,
            .buttons = {},
            .invulnerableFor = rules.respawnInvulnerability,
        }
    );
    registry.addComponent(
        entity,
        Weapon{
            .cooldown = rules.playerFireCooldown,
            .timer = 0.0F, .muzzleOffset = rules.playerMuzzleOffset,
            .projectileSpeed = rules.playerMissileSpeed, .aim = {1.0F, 0.0F},
            .triggered = false,
        }
    );
    registry.addComponent(entity, ConfinedToPlayfield{});
    return netId;
}

NetId makeBydosSlave(engine::ecs::Registry &registry,
    engine::ecs::Entity entity, engine::core::Vec2f position,
    MovePattern pattern)
{
    const auto &rules = registry.getResource<Rules>();
    auto &rng = registry.getResource<engine::core::Rng>();
    const auto netId = registry.getResource<NetIdAllocator>().allocate(entity);
    const float speed = rng.uniformFloat(rules.enemySpeed.min,
        rules.enemySpeed.max);

    registry.addComponent(entity, Transform{.position = position});
    registry.addComponent(entity, Velocity{.value = {-speed, 0.0F}});
    registry.addComponent(
        entity,
        Collider{
            .halfSize = rules.enemyHalfSize,
            .layer = CollisionLayer::ENEMY,
        }
    );
    registry.addComponent(entity, Health{.current = 1, .max = 1});
    registry.addComponent(entity, Damage{.amount = 1});
    registry.addComponent(
        entity,
        Replicated{
            .netId = netId,
            .kind = EntityKind::BYDOS_SLAVE,
            .variant = static_cast<std::uint8_t>(pattern),
        }
    );
    if (pattern == MovePattern::SINE) {
        registry.addComponent(
            entity,
            SineMover{
                .speed = speed,
                .baseY = position.y(),
                .amplitude = rng.uniformFloat(rules.sineAmplitude.min,
                    rules.sineAmplitude.max),
                .frequency = rng.uniformFloat(rules.sineFrequency.min,
                    rules.sineFrequency.max),
                .age = 0.0F,
            }
        );
    } else {
        registry.addComponent(entity, StraightMover{.speed = speed});
    }
    registry.addComponent(
        entity,
        AimedShooter{
            .interval = rules.enemyFireInterval,
            .timer = rng.uniformFloat(rules.enemyFireInterval.min,
                rules.enemyFireInterval.max),
        }
    );
    registry.addComponent(
        entity,
        Weapon{
            .cooldown = 0.0F,
            .timer = 0.0F,
            .muzzleOffset = rules.enemyMuzzleOffset,
            .projectileSpeed = rules.enemyMissileSpeed,
            .aim = {-1.0F, 0.0F},
            .triggered = false,
        }
    );
    registry.addComponent(entity, ScoreValue{.points = rules.killScore});
    registry.addComponent(
        entity,
        DespawnOutsidePlayfield{.margin = rules.despawnMargin}
    );

    return netId;
}

NetId makePlayerMissile(engine::ecs::Registry &registry,
    const engine::ecs::Entity entity, const NetId owner,
    const std::uint8_t ownerPlayerId, const engine::core::Vec2f position)
{
    const auto &rules = registry.getResource<Rules>();
    const auto netId = registry.getResource<NetIdAllocator>().allocate(entity);

    registry.addComponent(entity, Transform{.position = position});
    registry.addComponent(
        entity,
        Velocity{.value = {rules.playerMissileSpeed, 0.0F}}
    );
    registry.addComponent(
        entity,
        Collider{
            .halfSize = rules.playerMissileHalfSize,
            .layer = CollisionLayer::PLAYER_SHOT,
        }
    );
    registry.addComponent(entity, Health{.current = 1, .max = 1});
    registry.addComponent(entity, Damage{.amount = 1});
    registry.addComponent(
        entity,
        Replicated{
            .netId = netId,
            .kind = EntityKind::PLAYER_MISSILE,
            .variant = 0,
        }
    );
    registry.addComponent(
        entity,
        Projectile{.owner = owner, .ownerPlayerId = ownerPlayerId}
    );
    registry.addComponent(
        entity,
        Lifetime{.remaining = rules.playerMissileLifetime}
    );
    registry.addComponent(
        entity,
        DespawnOutsidePlayfield{.margin = rules.despawnMargin}
    );

    return netId;
}

NetId makeEnemyMissile(engine::ecs::Registry &registry,
    const engine::ecs::Entity entity, const NetId owner,
    const engine::core::Vec2f position, const engine::core::Vec2f direction)
{
    const auto &rules = registry.getResource<Rules>();
    const bool aimed = direction != engine::core::Vec2f{0.0F, 0.0F};

    assert(aimed && "makeEnemyMissile: zero direction");
    const engine::core::Vec2f heading = aimed ? direction.normalized()
                                              : engine::core::Vec2f{-1.0F, 0.0F};
    const auto netId = registry.getResource<NetIdAllocator>().allocate(entity);

    registry.addComponent(entity, Transform{.position = position});
    registry.addComponent(
        entity,
        Velocity{.value = heading * rules.enemyMissileSpeed}
    );
    registry.addComponent(
        entity,
        Collider{
            .halfSize = rules.enemyMissileHalfSize,
            .layer = CollisionLayer::ENEMY_SHOT,
        }
    );
    registry.addComponent(entity, Health{.current = 1, .max = 1});
    registry.addComponent(entity, Damage{.amount = 1});
    registry.addComponent(
        entity,
        Replicated{
            .netId = netId,
            .kind = EntityKind::ENEMY_MISSILE,
            .variant = 0,
        }
    );
    registry.addComponent(
        entity,
        Projectile{.owner = owner, .ownerPlayerId = NO_PLAYER}
    );
    registry.addComponent(
        entity,
        Lifetime{.remaining = rules.enemyMissileLifetime}
    );
    registry.addComponent(
        entity,
        DespawnOutsidePlayfield{.margin = rules.despawnMargin}
    );

    return netId;
}
}
