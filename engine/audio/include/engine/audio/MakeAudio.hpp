/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** MakeAudio.hpp
*/

#ifndef MAKE_AUDIO_HPP
#define MAKE_AUDIO_HPP

#include <memory>

#include "engine/audio/IAudio.hpp"

namespace engine::audio {

/*
** The only way the client creates audio: tries RaylibAudio and, if the device
** can't open, logs one warning and returns NullAudio. Never throws, so a
** machine without a sound card still runs the game, silently.
*/
[[nodiscard]] std::unique_ptr<IAudio> makeAudio();

}

#endif // !MAKE_AUDIO_HPP
