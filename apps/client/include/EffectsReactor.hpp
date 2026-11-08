/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** EffectsReactor.hpp
*/

#ifndef EFFECTS_REACTOR_HPP
#define EFFECTS_REACTOR_HPP

#include "ClientSettings.hpp"
#include "engine/core/Vec2.hpp"
#include "engine/ecs/Registry.hpp"
#include "GameEventBus.hpp"

namespace rtype::client {

struct TransientEffect {
    float remaining;
};

class EffectsReactor {
public:
    static constexpr float SMALL_EXPLOSION_SCALE = 1.0F;
    static constexpr float BIG_EXPLOSION_SCALE   = 2.0F;

    explicit EffectsReactor(GameEventBus &bus, engine::ecs::Registry &registry,
        const ClientSettings &settings);
    void onEvent(const protocol::Event &event);

private:
    void spawnExplosion(engine::core::Vec2f position, float scale);

    engine::ecs::Registry &_registry;
    const ClientSettings &_settings;
    GameEventBus::Subscription _subscription;
};
} // namespace rtype::client

#endif /* !EFFECTS_REACTOR_HPP_ */
