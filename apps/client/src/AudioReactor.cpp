/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** AudioReactor.cpp
*/

#include "AudioReactor.hpp"

namespace rtype::client {

AudioReactor::AudioReactor(GameEventBus &bus, engine::audio::IAudio &audio,
    const SoundBank &sounds, const ClientWorld &world):
    _audio(audio), _sounds(sounds), _world(world),
    _subscription(bus.subscribe([this](const protocol::Event &event) {
        onEvent(event);
    }))
{}

void AudioReactor::onEvent(const protocol::Event &event) const
{
    switch (event.type) {
    case protocol::EventType::PLAYER_JOINED:
        _audio.playSound(_sounds.join, BASE_VOLUME);
        break;
    case protocol::EventType::PLAYER_LEFT:
        _audio.playSound(_sounds.leave, BASE_VOLUME);
        break;
    case protocol::EventType::PLAYER_FIRED:
        _audio.playSound(_sounds.shot,
            event.a == _world.localShipNetId() ? 1.0F : OTHER_SHOT_VOLUME);
        break;
    case protocol::EventType::PLAYER_KILLED:
        _audio.playSound(_sounds.explosionBig, BASE_VOLUME);
        break;
    case protocol::EventType::ENEMY_KILLED:
        _audio.playSound(_sounds.explosionSmall, BASE_VOLUME);
        break;
    default:
        break;
    }
}
} // namespace rtype::client
