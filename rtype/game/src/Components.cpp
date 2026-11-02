/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Components
*/

#include "rtype/game/Components.hpp"

namespace rtype::game {

void registerComponents(engine::ecs::Registry &registry)
{
    registry.registerComponent<Transform>();
    registry.registerComponent<Velocity>();
    registry.registerComponent<Collider>();
    registry.registerComponent<Health>();
    registry.registerComponent<Damage>();
    registry.registerComponent<Replicated>();
    registry.registerComponent<PlayerShip>();
    registry.registerComponent<Weapon>();
    registry.registerComponent<StraightMover>();
    registry.registerComponent<SineMover>();
    registry.registerComponent<AimedShooter>();
    registry.registerComponent<Projectile>();
    registry.registerComponent<LastHit>();
    registry.registerComponent<Lifetime>();
    registry.registerComponent<ConfinedToPlayfield>();
    registry.registerComponent<DespawnOutsidePlayfield>();
    registry.registerComponent<ScoreValue>();
}
}
