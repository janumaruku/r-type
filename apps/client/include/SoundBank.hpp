/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SoundBank.hpp
*/

#ifndef SOUND_BANK_HPP
#define SOUND_BANK_HPP

#include "engine/audio/AudioId.hpp"
#include "engine/audio/IAudio.hpp"

namespace rtype::client {
struct SoundBank {
    engine::audio::SoundId shot;
    engine::audio::SoundId explosionSmall;
    engine::audio::SoundId explosionBig;
    engine::audio::SoundId join;
    engine::audio::SoundId leave;
    engine::audio::MusicId stage1;

    [[nodiscard]] static SoundBank load(engine::audio::IAudio &audio);
};
} // namespace rtype::client

#endif /* !SOUND_BANK_HPP_ */
