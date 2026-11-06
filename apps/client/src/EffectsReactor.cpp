/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** EffectsReactor.cpp
*/

#include "EffectsReactor.hpp"

#include "PresentationComponents.hpp"
#include "rtype/game/Components.hpp"
#include "SpriteCatalog.hpp"
#include "Starfield.hpp"

namespace rtype::client {

EffectsReactor::EffectsReactor(GameEventBus &bus,
    engine::ecs::Registry &registry, const ClientSettings &settings):
    _registry(registry), _settings(settings),
    _subscription(bus.subscribe([this](const protocol::Event &event) {
        onEvent(event);
    }))
{}

void EffectsReactor::onEvent(const protocol::Event &event)
{

    engine::core::Vec2f pos = {event.x, event.y};

    switch (event.type) {
    case protocol::EventType::PLAYER_KILLED:
        spawnExplosion(pos, BIG_EXPLOSION_SCALE);
        break;
    case protocol::EventType::ENEMY_KILLED:
        spawnExplosion(pos, SMALL_EXPLOSION_SCALE);
        break;
    default:
        break;
    }
}

void EffectsReactor::spawnExplosion(engine::core::Vec2f position, float scale)
{
    auto &catalog      = _registry.getResource<SpriteCatalog>();
    const auto id      = catalog.find(SpriteKey::EXPLOSION, 0);
    const auto &def    = catalog.get(id);
    const auto &frames = def.states[0].frames;

    const std::uint8_t alpha = _settings.reduceFlashes ? 127 : 255;

    const auto entity = _registry.spawnEntity();
    _registry.addComponent(entity, game::Transform{.position = position});
    _registry.addComponent(entity, Animation{.def = id, .state = 0});
    _registry.addComponent(entity,
        Sprite{
            .texture = def.texture,
            .frame   = frames.front(),
            .layer   = layers::EXPLOSIONS,
            .tint    = engine::core::Color(255, 255, 255, alpha),
            .scale   = engine::core::Vec2f(scale, scale),
        });
    _registry.addComponent(entity,
        TransientEffect{
            .remaining = static_cast<float>(frames.size()) * def.frameTime,
        });
}

} // namespace rtype::client
