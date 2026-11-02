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

namespace rtype::game {
GameWorld::GameWorld(Rules rules, std::uint32_t seed)
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

void GameWorld::apply(const PlayerCommand &command)
{
    std::visit([this](const auto &alternative) { handle(alternative); },
        command);
}

void GameWorld::step(float dt)
{
    _registry.getResource<engine::ecs::DeltaTime>().seconds = dt;
    _registry.runStage(Stage::SIMULATION);
    ++_tick;
}

std::vector<GameEvent> GameWorld::drainEvents()
{
    return _registry.getResource<EventOutbox>().drain();
}

void GameWorld::collectReplicated(std::vector<ReplicatedEntity> &out) const
{
    out.clear();
    for (auto &&[entity, replicated]:
         _registry.indexedView<const Replicated>()) {
        const auto *transform = _registry.getComponent<Transform>(entity);
        const auto *velocity = _registry.getComponent<Velocity>(entity);

        out.push_back({.netId = replicated.netId,
            .kind = replicated.kind,
            .variant = replicated.variant,
            .position = transform != nullptr ? transform->position
                                             : engine::core::Vec2f{},
            .velocity = velocity != nullptr ? velocity->value
                                            : engine::core::Vec2f{}});
    }
}

void GameWorld::handle(const JoinPlayer &command)
{
    auto &roster = _registry.getResource<PlayerRoster>();
    const bool gameOver =
        _registry.getResource<GamePhase>().phase() == Phase::GAME_OVER;
    const bool joined = gameOver
        ? roster.joinWaiting(command.playerId, command.name)
        : roster.join(command.playerId, command.name,
              _registry.getResource<Rules>().playerLives);

    if (joined)
        _registry.getResource<EventOutbox>().push(
            PlayerJoined{.playerId = command.playerId});
}

void GameWorld::handle(const LeavePlayer &command)
{
    auto &roster = _registry.getResource<PlayerRoster>();
    const PlayerSlot *slot = roster.get(command.playerId);

    if (slot == nullptr)
        return;

    std::vector<engine::ecs::Entity> doomed;

    if (const auto ship = _registry.getResource<NetIdAllocator>().find(
            slot->shipNetId, _registry))
        doomed.push_back(*ship);
    for (auto &&[entity, projectile]:
         _registry.indexedView<const Projectile>())
        if (projectile.ownerPlayerId == command.playerId)
            doomed.push_back(entity);
    for (const auto &entity: doomed)
        _registry.killEntity(entity);
    roster.release(command.playerId);
    _registry.getResource<EventOutbox>().push(
        PlayerLeft{.playerId = command.playerId, .reason = command.reason});
}

void GameWorld::handle(const SetInput &command)
{
    auto &roster = _registry.getResource<PlayerRoster>();

    if (!roster.acceptInput(command.playerId, command.inputSequence))
        return;

    const auto ship = _registry.getResource<NetIdAllocator>().find(
        roster.get(command.playerId)->shipNetId, _registry);

    if (!ship)
        return;
    if (auto *player = _registry.getComponent<PlayerShip>(*ship))
        player->buttons = command.buttons;
}
}
