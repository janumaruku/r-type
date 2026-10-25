/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** World
*/

#include "World.hpp"

#include "engine/core/Rng.hpp"
#include "engine/ecs/Resources.hpp"

namespace ecs_example {
namespace {
constexpr int STARS = 80;
constexpr float PLAYER_RADIUS = 13.0F;
constexpr float ENEMY_RADIUS = 14.0F;
constexpr float BULLET_RADIUS = 4.0F;
constexpr float BULLET_SPEED = 430.0F;
constexpr float BULLET_LIFETIME = 2.0F;

void spawnPlayer(Registry &registry)
{
    const Entity player = registry.spawnEntity();

    registry.addComponent(player, Player{});
    registry.addComponent(player, Position{Vec2f(100.0F, HEIGHT / 2.0F)});
    registry.addComponent(player, Radius{PLAYER_RADIUS});
    registry.addComponent(player, Cooldown{0.0F});
}

void spawnStars(Registry &registry, engine::core::Rng &rng)
{
    for (int i = 0; i < STARS; ++i) {
        const Entity star = registry.spawnEntity();
        const auto brightness =
            static_cast<unsigned char>(rng.uniformInt(60, 200));

        registry.addComponent(star, Star{brightness});
        registry.addComponent(star, Position{Vec2f(
            rng.uniformFloat(0.0F, WIDTH), rng.uniformFloat(0.0F, HEIGHT))});
        registry.addComponent(star, Velocity{Vec2f(
            -static_cast<float>(brightness) / 4.0F, 0.0F)});
    }
}
}

void buildWorld(Registry &registry, const std::uint32_t seed)
{
    registry.setResource(engine::ecs::DeltaTime{0.0F});
    registry.setResource(Controls{.direction = Vec2f(0.0F, 0.0F),
                                  .fire = false});
    registry.setResource(Game{.score = 0, .over = false, .nextEnemy = 1.0F});
    engine::core::Rng &rng = registry.setResource(engine::core::Rng{seed});

    addGameplaySystems(registry);
    addRenderSystems(registry);

    spawnPlayer(registry);
    spawnStars(registry, rng);
}

void makeEnemy(Registry &registry, const Entity &entity, const Vec2f at,
    const float speed)
{
    registry.addComponent(entity, Enemy{});
    registry.addComponent(entity, Position{at});
    registry.addComponent(entity, Velocity{Vec2f(-speed, 0.0F)});
    registry.addComponent(entity, Radius{ENEMY_RADIUS});
    registry.addComponent(entity, Tint{Color{240, 70, 80, 255}});
}

void makeBullet(Registry &registry, const Entity &entity, const Vec2f at)
{
    registry.addComponent(entity, Bullet{});
    registry.addComponent(entity, Position{at});
    registry.addComponent(entity, Velocity{Vec2f(BULLET_SPEED, 0.0F)});
    registry.addComponent(entity, Radius{BULLET_RADIUS});
    registry.addComponent(entity,
        Tint{Color{.r = 255, .g = 225, .b = 80, .a = 255}});
    registry.addComponent(entity, Lifetime{BULLET_LIFETIME});
}
}
