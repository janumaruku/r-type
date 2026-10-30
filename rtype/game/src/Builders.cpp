/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Builders
*/

#include "rtype/game/Builders.hpp"
#include <format>

#include "engine/core/Errors.hpp"
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
}
