/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** AudioReactor.hpp
*/

#ifndef AUDIO_REACTOR_HPP
#define AUDIO_REACTOR_HPP

#include "ClientWorld.hpp"
#include "GameEventBus.hpp"
#include "SoundBank.hpp"

namespace rtype::client {

constexpr float BASE_VOLUME = 1.0F;
constexpr float OTHER_SHOT_VOLUME = 0.4F;

class AudioReactor {
public:

    explicit AudioReactor(GameEventBus &bus, engine::audio::IAudio &audio,
        const SoundBank &sounds, const ClientWorld &world);
    void onEvent(const protocol::Event &event) const;

private:
    engine::audio::IAudio &_audio;
    const SoundBank &_sounds;
    const ClientWorld &_world;
    GameEventBus::Subscription _subscription;
};
} // namespace rtype::client

#endif /* !AUDIO_REACTOR_HPP_ */
