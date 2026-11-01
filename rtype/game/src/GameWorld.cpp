/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GameWorld
*/

#include "rtype/game/GameWorld.hpp"

#include "engine/core/Rng.hpp"
#include "rtype/game/Components.hpp"
#include "rtype/game/GamePhase.hpp"
#include "rtype/game/NetIdAllocator.hpp"
#include "rtype/game/SpawnDirectorState.hpp"
#include "rtype/game/Systems.hpp"

rtype::game::GameWorld::GameWorld(Rules rules, std::uint32_t seed)
{
    registerComponents(_registry);

    _registry.setResource(rules);
    _registry.setResource(engine::core::Rng(seed));
    _registry.setResource(NetIdAllocator());
    _registry.setResource(PlayerRoster());
    _registry.setResource(GamePhase());
    _registry.setResource(SpawnDirectorState());
    _registry.setResource(EventOutbox());
    _registry.setResource(engine::ecs::DeltaTime());

    _registry.addSystem<PlayerShip, Velocity, Weapon>(Stage::SIMULATION,
        "playerInputSystem", playerInputSystem);
    _registry.addSystem<const StraightMover, Velocity>(Stage::SIMULATION,
        "straightMoverSystem", straightMoverSystem);
    _registry.addSystem<SineMover, Velocity>(Stage::SIMULATION,
        "sineMoverSystem", sineMoverSystem);
    _registry.addSystem<AimedShooter, const Transform, Weapon>(
        Stage::SIMULATION, "aimedShooterSystem", aimedShooterSystem);
    _registry.addSystem<Weapon, const Transform, const Replicated>(
        Stage::SIMULATION, "weaponSystem", weaponSystem);
    _registry.addSystem<Transform, const Velocity>(Stage::SIMULATION,
        "movementSystem", movementSystem);
    _registry.addSystem<Transform, const Collider>(Stage::SIMULATION,
        "playfieldSystem", playfieldSystem);
    _registry.addSystem<const Collider, const Transform, Health>(
        Stage::SIMULATION, "collisionSystem", collisionSystem);
    _registry.addSystem<const Health, const Replicated>(Stage::SIMULATION,
        "healthSystem", healthSystem);
    _registry.addSystem<Lifetime>(Stage::SIMULATION, "lifetimeSystem",
        lifetimeSystem);
    _registry.addSystem<const Replicated>(Stage::SIMULATION,
        "spawnDirectorSystem", spawnDirectorSystem);
    _registry.addSystem(Stage::SIMULATION, "respawnSystem", respawnSystem);
    _registry.addSystem<const Replicated>(Stage::SIMULATION,
        "gameOverSystem", gameOverSystem);
    _registry.addSystem(Stage::SIMULATION, "netIdCleanupSystem",
        netIdCleanupSystem);
}
