/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** MotionSystems
*/

#include "rtype/game/Systems.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <optional>
#include <vector>

#include "engine/core/Rect2.hpp"
#include "engine/core/Rng.hpp"
#include "engine/ecs/Resources.hpp"
#include "rtype/game/Builders.hpp"
#include "rtype/game/Events.hpp"
#include "rtype/game/GamePhase.hpp"
#include "rtype/game/Motion.hpp"
#include "rtype/game/PlayerRoster.hpp"
#include "rtype/game/Rules.hpp"
#include "rtype/game/SpawnDirectorState.hpp"

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

struct Hitbox {
    engine::ecs::Entity entity;
    engine::core::FloatRect box;
};

using LayerHitboxes = std::array<std::vector<Hitbox>, 4>;

std::uint8_t hitterPlayerId(engine::ecs::Registry &registry,
    const engine::ecs::Entity &hitter)
{
    if (const auto *ship = registry.getComponent<rtype::game::PlayerShip>(
        hitter))
        return ship->playerId;
    if (const auto *projectile =
        registry.getComponent<rtype::game::Projectile>(hitter))
        return projectile->ownerPlayerId;
    return rtype::game::NO_PLAYER;
}

void hit(engine::ecs::Registry &registry, const engine::ecs::Entity &target,
    const engine::ecs::Entity &hitter)
{
    auto *health = registry.getComponent<rtype::game::Health>(target);
    const auto *damage = registry.getComponent<rtype::game::Damage>(hitter);
    const auto *replicated =
        registry.getComponent<rtype::game::Replicated>(hitter);

    if (health == nullptr || damage == nullptr)
        return;
    health->current =
        static_cast<std::int16_t>(health->current - damage->amount);

    const rtype::game::LastHit lastHit{
        .by = replicated != nullptr
        ? replicated->netId
        : rtype::game::NO_NET_ID,
        .byPlayerId = hitterPlayerId(registry, hitter),
    };

    if (auto *existing = registry.getComponent<rtype::game::LastHit>(target))
        *existing = lastHit;
    else
        registry.commands().add(target, lastHit);
}

bool spend(engine::ecs::Registry &registry, const engine::ecs::Entity &missile)
{
    auto *health = registry.getComponent<rtype::game::Health>(missile);

    if (health == nullptr || health->current <= 0)
        return false;
    health->current = 0;
    return true;
}

template <typename OnOverlap>
void forEachOverlap(const std::vector<Hitbox> &targets,
    const std::vector<Hitbox> &hitters, OnOverlap onOverlap)
{
    for (const Hitbox &target: targets)
        for (const Hitbox &hitter: hitters)
            if (target.box.intersects(hitter.box))
                onOverlap(target.entity, hitter.entity);
}

const std::vector<Hitbox> &layer(const LayerHitboxes &hitboxes,
    const rtype::game::CollisionLayer which) noexcept
{
    return hitboxes[static_cast<std::size_t>(which)];
}

float spawnInterval(const rtype::game::Rules &rules, const float elapsed)
{
    const float steps = std::floor(elapsed / rules.spawnIntervalStepEvery);

    return std::max(rules.spawnIntervalMin,
        rules.spawnIntervalStart - rules.spawnIntervalStep * steps);
}

std::size_t countBydos(engine::ecs::Registry &registry)
{
    std::size_t count = 0;

    for (auto &&[replicated]: registry.view<const rtype::game::Replicated>())
        if (replicated.kind == rtype::game::EntityKind::BYDOS_SLAVE)
            ++count;
    return count;
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
                    Fired{.shooter = shooter, .missile = missileId,
                          .side = side, .position = muzzle,});
            });
    }
}

void collisionSystem(engine::ecs::Registry &registry,
    const engine::ecs::SparseSet<Collider> &,
    const engine::ecs::SparseSet<Transform> &,
    engine::ecs::SparseSet<Health> &)
{
    using enum CollisionLayer;
    LayerHitboxes hitboxes;

    for (auto &&[entity, collider, transform]:
         registry.indexedView<const Collider, const Transform>()) {
        const auto *ship = registry.getComponent<PlayerShip>(entity);

        if (ship == nullptr || ship->invulnerableFor <= 0.0F)
            hitboxes[static_cast<std::size_t>(collider.layer)].push_back(
            {.entity = entity,
             .box = engine::core::FloatRect::fromCenter(
                 transform.position, collider.halfSize),});
    }
    forEachOverlap(layer(hitboxes, PLAYER), layer(hitboxes, ENEMY),
        [&registry](const auto &ship, const auto &enemy) {
            hit(registry, ship, enemy);
            hit(registry, enemy, ship);
        });
    forEachOverlap(layer(hitboxes, PLAYER), layer(hitboxes, ENEMY_SHOT),
        [&registry](const auto &ship, const auto &missile) {
            if (spend(registry, missile))
                hit(registry, ship, missile);
        });
    forEachOverlap(layer(hitboxes, ENEMY), layer(hitboxes, PLAYER_SHOT),
        [&registry](const auto &enemy, const auto &missile) {
            if (spend(registry, missile))
                hit(registry, enemy, missile);
        });
}

void healthSystem(engine::ecs::Registry &registry,
    const engine::ecs::SparseSet<Health> &,
    const engine::ecs::SparseSet<Replicated> &)
{
    const auto &rules = registry.getResource<Rules>();
    auto &roster = registry.getResource<PlayerRoster>();
    auto &outbox = registry.getResource<EventOutbox>();

    for (auto &&[entity, health, replicated]:
         registry.indexedView<const Health, const Replicated>()) {
        if (health.current > 0)
            continue;
        registry.commands().kill(entity);

        const auto *lastHit = registry.getComponent<LastHit>(entity);
        const LastHit killer = lastHit != nullptr ? *lastHit : LastHit{};
        const auto *transform = registry.getComponent<Transform>(entity);
        const engine::core::Vec2f position =
            transform != nullptr ? transform->position : engine::core::Vec2f{};

        if (const auto *ship = registry.getComponent<PlayerShip>(entity)) {
            (void)roster.onShipKilled(ship->playerId, rules.respawnDelay);
            outbox.push(PlayerKilled{.killer = killer.by,
                                     .ship = replicated.netId,
                                     .playerId = ship->playerId,
                                     .position = position,});
        } else if (const auto *score =
            registry.getComponent<ScoreValue>(entity)) {
            if (roster.get(killer.byPlayerId) != nullptr)
                (void)roster.addScore(killer.byPlayerId, score->points);
            outbox.push(EnemyKilled{.killerShip = killer.by,
                                    .enemy = replicated.netId,
                                    .position = position,});
        }
    }
}

/*
** Subtracting 1/60 from 2.0 a hundred and twenty times in float leaves
** 1.2e-6, not 0, so a plain `<= 0` kills a 2 s missile one tick late. The
** tolerance, far below one tick, absorbs that rounding.
*/
void lifetimeSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<Lifetime> &)
{
    const auto &delta = registry.getResource<engine::ecs::DeltaTime>();
    constexpr float tolerance = 1e-4F;

    for (auto &&[entity, lifetime]: registry.indexedView<Lifetime>()) {
        lifetime.remaining -= delta.seconds;
        if (lifetime.remaining <= tolerance)
            registry.commands().kill(entity);
    }
}

void spawnDirectorSystem(engine::ecs::Registry &registry,
    const engine::ecs::SparseSet<Replicated> &)
{
    if (!registry.getResource<PlayerRoster>().anyAlive()
        || registry.getResource<GamePhase>().phase() != Phase::PLAYING)
        return;

    const auto &rules = registry.getResource<Rules>();
    const auto &delta = registry.getResource<engine::ecs::DeltaTime>();
    auto &state = registry.getResource<SpawnDirectorState>();
    auto &rng = registry.getResource<engine::core::Rng>();

    state.elapsed += delta.seconds;
    state.untilNextSpawn -= delta.seconds;
    if (state.untilNextSpawn > 0.0F)
        return;
    if (countBydos(registry) < rules.maxEnemies) {
        const float y = rng.uniformFloat(rules.enemySpawnY.min,
            rules.enemySpawnY.max);
        const MovePattern pattern = rng.chance(rules.sineChance)
            ? MovePattern::SINE
            : MovePattern::STRAIGHT;

        makeBydosSlave(registry, registry.spawnEntity(),
            {rules.enemySpawnX, y}, pattern);
    }
    state.untilNextSpawn = spawnInterval(rules, state.elapsed);
}
}
