/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Voices.hpp
*/

#ifndef VOICES_HPP
#define VOICES_HPP

#include <cstddef>
#include <cstdint>
#include <span>

namespace engine::audio {

/*
** One playable copy of a sound. startedAt is the value of a play counter that
** goes up on every play, so a smaller value means the copy started earlier.
*/
struct Voice {
    bool playing{false};
    std::uint64_t startedAt{0};
};

/*
** Picks the copy to play next: the first one that isn't playing, else the one
** that started first, which gets cut. Throws EngineError when voices is empty.
*/
[[nodiscard]] std::size_t chooseVoice(std::span<const Voice> voices);

}

#endif // !VOICES_HPP
