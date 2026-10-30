/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** MotionSystems
*/

#include "rtype/game/Systems.hpp"

#include <algorithm>

#include "engine/ecs/Resources.hpp"
#include "rtype/game/Motion.hpp"
#include "rtype/game/Rules.hpp"

namespace {
bool isOutside(const engine::core::Vec2f position,
    const engine::core::Vec2f halfSize, const float margin,
    const rtype::game::Rules &rules) noexcept
{
    return position.x() - halfSize.x() > rules.playfield.x() + margin
        || position.x() + halfSize.x() < -margin
        || position.y() - halfSize.y() > rules.playfield.y() + margin
        || position.y() + halfSize.y() < -margin;
}
}

namespace rtype::game {
void playerInputSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<PlayerShip> &,
    engine::ecs::SparseSet<Velocity> &,
    engine::ecs::SparseSet<Weapon> &)
{
    const auto &rules = registry.getResource<Rules>();
    const auto &delta = registry.getResource<engine::ecs::DeltaTime>();

    for (auto &&[ship, velocity, weapon]:
         registry.view<PlayerShip, Velocity, Weapon>()) {
        velocity.value = inputDirection(ship.buttons) * rules.playerSpeed;
        weapon.triggered = ship.buttons.held(InputButtons::FIRE);
        ship.invulnerableFor =
            std::max(0.0F, ship.invulnerableFor - delta.seconds);
    }
}

void movementSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<Transform> &,
    const engine::ecs::SparseSet<Velocity> &)
{
    const auto &delta = registry.getResource<engine::ecs::DeltaTime>();

    for (auto &&[transform, velocity]:
         registry.view<Transform, const Velocity>())
        transform.position += velocity.value * delta.seconds;
}

void playfieldSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<Transform> &,
    const engine::ecs::SparseSet<Collider> &)
{
    const auto &rules = registry.getResource<Rules>();

    for (auto &&[transform, collider, _]:
         registry.view<Transform, const Collider, const ConfinedToPlayfield>())
        transform.position = confineToPlayfield(transform.position,
            collider.halfSize, rules);

    for (auto &&[entity, transform, collider, despawn]:
         registry.indexedView<const Transform, const Collider,
             const DespawnOutsidePlayfield>())
        if (isOutside(transform.position, collider.halfSize, despawn.margin,
                rules))
            registry.commands().kill(entity);
}

void straightMoverSystem(engine::ecs::Registry &registry,
    const engine::ecs::SparseSet<StraightMover> &,
    engine::ecs::SparseSet<Velocity> &)
{
    for (auto &&[mover, velocity]:
         registry.view<const StraightMover, Velocity>())
        velocity.value = {-mover.speed, 0.0F};
}
}
