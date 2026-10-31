/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** MotionSystems
*/

#include "rtype/game/Systems.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <optional>

#include "engine/core/Rng.hpp"
#include "engine/ecs/Resources.hpp"
#include "rtype/game/Builders.hpp"
#include "rtype/game/Events.hpp"
#include "rtype/game/Motion.hpp"
#include "rtype/game/PlayerRoster.hpp"
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

std::optional<engine::core::Vec2f> nearestShip(
    engine::ecs::Registry &registry, const engine::core::Vec2f from)
{
    std::optional<engine::core::Vec2f> nearest;
    float best = 0.0F;

    for (auto &&[ship, transform]:
         registry.view<const rtype::game::PlayerShip, const
             rtype::game::Transform>()) {
        const float distance = (transform.position - from).lengthSquared();

        if (!nearest || distance < best) {
            nearest = transform.position;
            best = distance;
        }
    }

    return nearest;
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

/*
** The ticket's velocity, amplitude × 2π × frequency × cos(2π × frequency ×
** age), sampled once per tick, overshoots baseY ± amplitude by up to 7.6
** units at 60 Hz. The sine's change over the tick divided by dt is the same
** derivative averaged over the tick, so movementSystem lands y exactly on
** baseY + amplitude × sin(2π × frequency × age) at every tick.
*/
void sineMoverSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<SineMover> &,
    engine::ecs::SparseSet<Velocity> &)
{
    const auto &delta = registry.getResource<engine::ecs::DeltaTime>();
    constexpr float twoPi = 2.0F * std::numbers::pi_v<float>;

    for (auto &&[mover, velocity]: registry.view<SineMover, Velocity>()) {
        const float before = std::sin(twoPi * mover.frequency * mover.age);

        mover.age += delta.seconds;
        const float after = std::sin(twoPi * mover.frequency * mover.age);
        const float vy = delta.seconds > 0.0F
            ? mover.amplitude * (after - before) / delta.seconds
            : 0.0F;

        velocity.value = {-mover.speed, vy};
    }
}

void aimedShooterSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<AimedShooter> &,
    const engine::ecs::SparseSet<Transform> &,
    engine::ecs::SparseSet<Weapon> &)
{
    const auto &delta = registry.getResource<engine::ecs::DeltaTime>();
    const auto &roster = registry.getResource<PlayerRoster>();
    auto &rng = registry.getResource<engine::core::Rng>();

    for (auto &&[shooter, transform, weapon]:
         registry.view<AimedShooter, const Transform, Weapon>()) {
        shooter.timer -= delta.seconds;
        if (shooter.timer > 0.0F)
            continue;
        shooter.timer =
            rng.uniformFloat(shooter.interval.min, shooter.interval.max);
        if (!roster.anyAlive())
            continue;

        const engine::core::Vec2f muzzle =
            transform.position + weapon.muzzleOffset;
        const auto target = nearestShip(registry, muzzle);

        if (!target || (*target - muzzle).lengthSquared() <= 0.0F)
            continue;
        weapon.aim = (*target - muzzle).normalized();
        weapon.triggered = true;
    }
}

void weaponSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<Weapon> &,
    const engine::ecs::SparseSet<Transform> &,
    const engine::ecs::SparseSet<Replicated> &)
{
    const auto &delta = registry.getResource<engine::ecs::DeltaTime>();

    for (auto &&[entity, weapon, transform, replicated]:
         registry.indexedView<Weapon, const Transform, const Replicated>()) {
        const auto *ship = registry.getComponent<PlayerShip>(entity);

        weapon.timer = std::max(0.0F, weapon.timer - delta.seconds);
        const bool ready = weapon.triggered && weapon.timer == 0.0F;

        if (ship == nullptr)
            weapon.triggered = false;
        if (!ready)
            continue;
        weapon.timer = weapon.cooldown;
        registry.commands().spawn(
            [shooter = replicated.netId,
                playerId = ship != nullptr ? ship->playerId : NO_PLAYER,
                side = ship != nullptr ? Side::PLAYER : Side::ENEMY,
                muzzle = transform.position + weapon.muzzleOffset,
                aim = weapon.aim](engine::ecs::Registry &world,
                const engine::ecs::Entity &missile) {
                const NetId missileId = side == Side::PLAYER
                    ? makePlayerMissile(world, missile, shooter, playerId,
                          muzzle)
                    : makeEnemyMissile(world, missile, shooter, muzzle, aim);

                world.getResource<EventOutbox>().push(
                    Fired{.shooter=shooter, .missile=missileId, .side=side, .position=muzzle});
            });
    }
}
}
