/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RenderSystems.cpp
*/

#include "RenderSystems.hpp"

#include <algorithm>
#include <cstddef>

#include "ClientSettings.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/render/DrawCommand.hpp"
#include "engine/render/RenderQueue.hpp"
#include "PresentationComponents.hpp"
#include "rtype/game/Components.hpp"
#include "SpriteCatalog.hpp"
#include "Starfield.hpp"

namespace rtype::client {

namespace {
constexpr float PLAYFIELD_WIDTH  = 1280.0F;
constexpr float PLAYFIELD_HEIGHT = 720.0F;
constexpr float OUTLINE_PADDING  = 1.0F;
} // namespace

std::int16_t layerFor(protocol::EntityKind kind) noexcept
{
    switch (kind) {
    case protocol::EntityKind::PLAYER_SHIP:
        return layers::PLAYERS;
    case protocol::EntityKind::BYDOS_SLAVE:
        return layers::ENEMIES;
    case protocol::EntityKind::ENEMY_MISSILE:
    case protocol::EntityKind::PLAYER_MISSILE:
        return layers::MISSILES;
    default:
        return 0;
    }
}

bool setAnimationState(Animation &animation, const SpriteDef &def,
    std::string_view name) noexcept
{
    auto idxOpt = def.stateIndex(name);

    if (!idxOpt.has_value())
        return false;
    auto state = idxOpt.value();
    if (animation.state != state) {
        animation.state = state;
        animation.index = 0;
        animation.timer = 0.0F;
    }
    return true;
}

void animationSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<Animation> &, engine::ecs::SparseSet<Sprite> &)
{
    for (auto &&[animation, sprite]: registry.view<Animation, Sprite>()) {
        const auto &delta = registry.getResource<engine::ecs::DeltaTime>();
        auto &catalog     = registry.getResource<SpriteCatalog>();
        auto def          = catalog.get(animation.def);
        auto frames       = def.states[animation.state].frames;
        if (animation.index >= frames.size())
            animation.index = std::clamp(animation.index, 0UL,
                static_cast<std::size_t>(frames.size()) - 1);
        animation.timer += delta.seconds;
        if (def.frameTime > 0 && animation.timer >= def.frameTime) {
            auto steps =
                static_cast<std::size_t>(animation.timer / def.frameTime);
            animation.timer -= static_cast<float>(steps) * def.frameTime;
            animation.index = def.loop
                ? (animation.index + steps) % frames.size()
                : std::min(animation.index + steps, frames.size() - 1);
        }
        sprite.frame = frames[animation.index];
    }
}

void shipTiltSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<ShipTilt> &,
    const engine::ecs::SparseSet<game::Transform> &,
    engine::ecs::SparseSet<Animation> &)
{
    auto dt       = registry.getResource<engine::ecs::DeltaTime>().seconds;
    auto &catalog = registry.getResource<SpriteCatalog>();

    for (auto &&[tilt, transform, animation]:
        registry.view<ShipTilt, game::Transform, Animation>()) {
        auto vy =
            (dt > 0.0F) ? (transform.position.y() - tilt.lastY) / dt : 0.0F;

        int direction = 0;

        if (vy < -TILT_SPEED_THRESHOLD)
            direction = -1;
        else if (vy > TILT_SPEED_THRESHOLD)
            direction = 1;

        if (direction == 0)
            tilt.heldTime = 0.0F;
        else if (direction != tilt.direction)
            tilt.heldTime = dt;
        else
            tilt.heldTime += dt;
        tilt.direction = static_cast<std::int8_t>(direction);

        bool full        = tilt.heldTime >= TILT_FULL_DELAY;
        std::string name = "idle";
        if (direction == -1)
            name = full ? "top" : "up";
        else if (direction == 1)
            name = full ? "bottom" : "down";

        setAnimationState(animation, catalog.get(animation.def), name);
        tilt.lastY = transform.position.y();
    }
}

void renderSystem(engine::ecs::Registry &registry,
    const engine::ecs::SparseSet<game::Transform> &,
    const ClientSettings &settings)
{
    auto &queue = registry.getResource<engine::render::RenderQueue>();

    for (auto &&[transform, sprite]: registry.view<game::Transform, Sprite>()) {
        const auto width  = static_cast<float>(sprite.frame.width());
        const auto height = static_cast<float>(sprite.frame.height());

        engine::render::SpriteCmd cmd{
            .texture     = sprite.texture,
            .source      = sprite.frame,
            .position    = transform.position,
            .origin      = engine::core::Vec2f(width / 2.0F, height / 2.0F),
            .scale       = sprite.scale,
            .rotationDeg = 0.0F,
            .tint        = sprite.tint,
            .layer       = sprite.layer,
        };

        queue.push(cmd);
    }

    if (!settings.highContrast)
        return;
    for (auto &&[transform, sprite, outline]:
        registry.view<game::Transform, Sprite, HighContrastOutline>()) {
        (void)outline;
        const float width =
            static_cast<float>(sprite.frame.width()) * sprite.scale.x();
        const float height =
            static_cast<float>(sprite.frame.height()) * sprite.scale.y();
        const engine::core::Vec2f corner{
            transform.position.x() - (width / 2.0F) - OUTLINE_PADDING,
            transform.position.y() - (height / 2.0F) - OUTLINE_PADDING,
        };
        engine::render::RectCmd rectCmd{
            .rect =
                engine::core::FloatRect(corner, width + 2.0F * OUTLINE_PADDING,
                    height + 2.0F * OUTLINE_PADDING),
            .fill             = engine::core::Color::CLEAR,
            .outline          = engine::core::Color::YELLOW,
            .outlineThickness = 2.0F,
            .layer            = static_cast<int16_t>(sprite.layer + 1),
        };

        queue.push(rectCmd);
    }
}

void effectSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<TransientEffect> &)
{
    const auto dt = registry.getResource<engine::ecs::DeltaTime>().seconds;

    for (auto &&[entity, effect]: registry.indexedView<TransientEffect>()) {
        effect.remaining -= dt;
        if (effect.remaining <= 0.0F)
            registry.commands().kill(entity);
    }
}

void spawnStarfield(engine::ecs::Registry &registry, engine::core::Rng &rng,
    engine::render::TextureId whitePixel)
{
    for (const auto &star: STAR_LAYERS) {
        for (std::size_t i = 0; i != star.count; i++) {
            const auto entity = registry.spawnEntity();

            game::Transform transform{};
            transform.position = {rng.uniformFloat(0.0F, PLAYFIELD_WIDTH),
                rng.uniformFloat(0.0F, PLAYFIELD_HEIGHT)};
            registry.addComponent(entity, transform);

            registry.addComponent(entity, Star{star.speed});

            Sprite sprite{};
            auto rectFrame = engine::core::IntRect{{0, 0}, 1, 1};
            sprite.texture = whitePixel;
            sprite.frame   = rectFrame;
            sprite.tint    = {
                star.brightness, star.brightness, star.brightness, 255};
            sprite.layer = star.layer;
            sprite.scale = {1.0F, 1.0F};
            registry.addComponent(entity, sprite);
        }
    }
}

void starfieldSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<game::Transform> &,
    const engine::ecs::SparseSet<Star> &, engine::core::Rng &rng)
{
    auto dt = registry.getResource<engine::ecs::DeltaTime>().seconds;

    for (auto &&[transform, star]: registry.view<game::Transform, Star>()) {
        transform.position.x() -= star.speed * dt;
        if (transform.position.x() < -1.0F) {
            transform.position.x() += PLAYFIELD_WIDTH;
            transform.position.y() = rng.uniformFloat(0.0F, PLAYFIELD_HEIGHT);
        }
    }
}
} // namespace rtype::client
