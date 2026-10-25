/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** NullAudio.cpp
*/

#include "engine/audio/NullAudio.hpp"

#include <cstdint>
#include <limits>

#include "engine/core/Errors.hpp"

namespace engine::audio {

namespace {

constexpr std::size_t MAX_IDS = std::numeric_limits<std::uint16_t>::max();

template<typename Id>
Id idFor(std::unordered_map<std::filesystem::path, Id> &ids,
    const std::filesystem::path &path)
{
    const std::filesystem::path key = path.lexically_normal();

    if (const auto it = ids.find(key); it != ids.end())
        return it->second;
    if (ids.size() >= MAX_IDS)
        throw core::ResourceError("Too many audio files (max 65535): " + key.string());

    const Id id{static_cast<std::uint16_t>(ids.size())};
    ids.emplace(key, id);
    return id;
}

}

SoundId NullAudio::loadSound(const std::filesystem::path &path)
{
    return idFor(_sounds, path);
}

MusicId NullAudio::loadMusic(const std::filesystem::path &path)
{
    return idFor(_musics, path);
}

void NullAudio::playSound(SoundId /*id*/, float /*volume*/) {}

void NullAudio::playMusic(MusicId /*id*/, bool /*loop*/) {}

void NullAudio::stopMusic() {}

void NullAudio::setMasterVolume(float volume)
{
    _volume.setMaster(volume);
}

void NullAudio::setSfxVolume(float volume)
{
    _volume.setSfx(volume);
}

void NullAudio::setMusicVolume(float volume)
{
    _volume.setMusic(volume);
}

void NullAudio::setMuted(bool muted)
{
    _volume.setMuted(muted);
}

const Volume &NullAudio::volumes() const
{
    return _volume;
}

void NullAudio::update() {}

}
