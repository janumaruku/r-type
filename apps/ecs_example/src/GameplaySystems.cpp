/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GameplaySystems
*/

#include <algorithm>
#include <unordered_set>

#include "World.hpp"
#include "engine/core/Rng.hpp"
#include "engine/ecs/Resources.hpp"
#include "engine/ecs/SparseSet.hpp"

namespace ecs_example {
namespace {
using engine::core::Rng;
using engine::ecs::DeltaTime;
using engine::ecs::SparseSet;

constexpr float PLAYER_SPEED = 260.0F;
constexpr float FIRE_DELAY = 0.22F;
constexpr float ENEMY_INTERVAL = 1.1F;
constexpr float ENEMY_MIN_SPEED = 80.0F;
constexpr float ENEMY_MAX_SPEED = 140.0F;
constexpr float MARGIN = 30.0F;

bool overlaps(const Position &first, const Radius &firstRadius,
    const Position &second, const Radius &secondRadius)
{
    const float reach = firstRadius.value + secondRadius.value;
    const Vec2f gap = first.value - second.value;

    return (gap.x() * gap.x()) + (gap.y() * gap.y()) < reach * reach;
}

void addPlayerSystem(Registry &registry)
{
    registry.addSystem<const Player, Position, Cooldown>(Stage::UPDATE,
        "player",
        [](Registry &registry, const SparseSet<Player> &,
        SparseSet<Position> &, SparseSet<Cooldown> &) {
            const float dt = registry.getResource<DeltaTime>().seconds;
            const Controls &controls = registry.getResource<Controls>();

            for (auto &&[player, position, cooldown]:
                 registry.view<const Player, Position, Cooldown>()) {
                const Vec2f moved = position.value
                    + controls.direction * (PLAYER_SPEED * dt);
                position.value = Vec2f(
                    std::clamp(moved.x(), MARGIN, WIDTH - MARGIN),
                    std::clamp(moved.y(), MARGIN, HEIGHT - MARGIN));
                cooldown.seconds = std::max(0.0F, cooldown.seconds - dt);
                if (!controls.fire || cooldown.seconds > 0.0F)
                    continue;
                cooldown.seconds = FIRE_DELAY;
                const Vec2f muzzle = position.value + Vec2f(20.0F, 0.0F);
                registry.commands().spawn(
                    [muzzle](Registry &world, const Entity &bullet) {
                        makeBullet(world, bullet, muzzle);
                    });
            }
        });
}

void addSpawnerSystem(Registry &registry)
{
    registry.addSystem<>(Stage::UPDATE, "spawner", [](Registry &registry) {
        Game &game = registry.getResource<Game>();
        Rng &rng = registry.getResource<Rng>();

        game.nextEnemy -= registry.getResource<DeltaTime>().seconds;
        if (game.nextEnemy > 0.0F)
            return;
        game.nextEnemy += ENEMY_INTERVAL;
        const Vec2f at(WIDTH + 20.0F,
            rng.uniformFloat(MARGIN, HEIGHT - MARGIN));
        const float speed = rng.uniformFloat(ENEMY_MIN_SPEED, ENEMY_MAX_SPEED);
        registry.commands().spawn(
            [at, speed](Registry &world, const Entity &enemy) {
                makeEnemy(world, enemy, at, speed);
            });
    });
}

void addMovementSystems(Registry &registry)
{
    registry.addSystem<Position, const Velocity>(Stage::UPDATE, "movement",
        [](Registry &registry, SparseSet<Position> &,
        const SparseSet<Velocity> &) {
            const float dt = registry.getResource<DeltaTime>().seconds;

            for (auto &&[position, velocity]:
                 registry.view<Position, const Velocity>())
                position.value += velocity.value * dt;
        });
    registry.addSystem<const Star, Position>(Stage::UPDATE, "starfield",
        [](Registry &registry, const SparseSet<Star> &,
        SparseSet<Position> &) {
            for (auto &&[star, position]:
                 registry.view<const Star, Position>())
                if (position.value.x() < 0.0F)
                    position.value += Vec2f(WIDTH, 0.0F);
        });
}

void addLifetimeSystem(Registry &registry)
{
    registry.addSystem<Lifetime>(Stage::UPDATE, "lifetime",
        [](Registry &registry, SparseSet<Lifetime> &lifetimes) {
            const float dt = registry.getResource<DeltaTime>().seconds;

            for (std::size_t i = 0; i < lifetimes.size(); ++i) {
                lifetimes.components()[i].seconds -= dt;
                if (lifetimes.components()[i].seconds <= 0.0F)
                    registry.commands().kill(lifetimes.entities()[i]);
            }
        });
}

/*
** A bullet is spent on the first enemy it touches, so one shot never scores
** twice; the kills themselves are recorded and applied after the system.
*/
void addCollisionSystem(Registry &registry)
{
    registry.addSystem<const Position, const Radius, const Player,
        const Enemy, const Bullet>(Stage::UPDATE, "collision",
        [](Registry &registry, const SparseSet<Position> &,
        const SparseSet<Radius> &, const SparseSet<Player> &,
        const SparseSet<Enemy> &, const SparseSet<Bullet> &) {
            Game &game = registry.getResource<Game>();
            std::unordered_set<Entity> spent;

            for (auto &&[enemy, enemyTag, enemyPosition, enemyRadius]:
                 registry.indexedView<const Enemy, const Position,
                     const Radius>()) {
                for (auto &&[player, position, radius]: registry.view<
                         const Player, const Position, const Radius>())
                    if (overlaps(position, radius, enemyPosition, enemyRadius))
                        game.over = true;
                for (auto &&[bullet, bulletTag, position, radius]:
                     registry.indexedView<const Bullet, const Position,
                         const Radius>()) {
                    if (spent.contains(bullet)
                        || !overlaps(position, radius, enemyPosition,
                            enemyRadius))
                        continue;
                    spent.insert(bullet);
                    registry.commands().kill(bullet);
                    registry.commands().kill(enemy);
                    ++game.score;
                    break;
                }
            }
        });
}

void addOffscreenSystem(Registry &registry)
{
    registry.addSystem<const Position, const Radius>(Stage::UPDATE,
        "offscreen",
        [](Registry &registry, const SparseSet<Position> &,
        const SparseSet<Radius> &) {
            for (auto &&[entity, position, radius]:
                 registry.indexedView<const Position, const Radius>())
                if (position.value.x() < -MARGIN
                    || position.value.x() > WIDTH + MARGIN)
                    registry.commands().kill(entity);
        });
}
}

void addGameplaySystems(Registry &registry)
{
    addPlayerSystem(registry);
    addSpawnerSystem(registry);
    addMovementSystems(registry);
    addLifetimeSystem(registry);
    addCollisionSystem(registry);
    addOffscreenSystem(registry);
}
}
