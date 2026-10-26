/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** AudioId.hpp
*/

#ifndef AUDIO_ID_HPP
#define AUDIO_ID_HPP

#include <cstdint>

namespace engine::audio {

/*
** Two distinct types, so a SoundId can't be passed where a MusicId is
** expected, and the other way round.
*/
struct SoundId {
    std::uint16_t value{};

    SoundId() = default;
    constexpr explicit SoundId(std::uint16_t id) : value{id} {}

    bool operator==(const SoundId &) const = default;
};

struct MusicId {
    std::uint16_t value{};

    MusicId() = default;
    constexpr explicit MusicId(std::uint16_t id) : value{id} {}

    bool operator==(const MusicId &) const = default;
};

}

#endif // !AUDIO_ID_HPP
