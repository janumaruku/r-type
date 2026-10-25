/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RaylibAudio.cpp
*/

#include "engine/audio/RaylibAudio.hpp"

#include <array>
#include <cstdint>
#include <limits>
#include <unordered_map>
#include <vector>

#include "engine/audio/Voices.hpp"
#include "engine/core/Errors.hpp"
#include "engine/core/ResourcePath.hpp"

#include <raylib.h>

namespace engine::audio {

namespace {

constexpr std::size_t ALIASES_PER_SOUND = 4;
constexpr std::size_t COPIES_PER_SOUND = ALIASES_PER_SOUND + 1;
constexpr std::size_t MAX_SOUNDS = std::numeric_limits<std::uint16_t>::max();   // 65 535

/*
** One loaded sound and its aliases: 5 copies that share the same samples, so
** the same sound can overlap itself without the memory growing.
*/
struct SoundSlot {
    Sound original{};
    std::array<Sound, ALIASES_PER_SOUND> aliases{};
    std::array<std::uint64_t, COPIES_PER_SOUND> startedAt{};   // copy 0 is the original

    [[nodiscard]] Sound &copy(std::size_t index)
    {
        return index == 0 ? original : aliases[index - 1];
    }
};

}

struct RaylibAudio::Impl {
    Volume volume;
    std::vector<SoundSlot> sounds;                                   // index == SoundId::value
    std::unordered_map<std::filesystem::path, SoundId> pathToSound;   // key: cacheKey(path)
    std::uint64_t playCounter = 0;
    std::vector<Music> musics;                                       // index == MusicId::value
};

RaylibAudio::RaylibAudio() : _impl(std::make_unique<Impl>())
{
    if (IsAudioDeviceReady())
        throw core::EngineError("only one RaylibAudio can exist at a time");
    InitAudioDevice();
    if (!IsAudioDeviceReady())
        throw core::EngineError("cannot open the sound device");
}

RaylibAudio::~RaylibAudio()
{
    for (const Music &music : _impl->musics)
        UnloadMusicStream(music);
    for (const SoundSlot &slot : _impl->sounds) {
        for (const Sound &alias : slot.aliases)
            UnloadSoundAlias(alias);
        UnloadSound(slot.original);
    }
    CloseAudioDevice();
}

SoundId RaylibAudio::loadSound(const std::filesystem::path &path)
{
    const std::filesystem::path key = core::cacheKey(path);

    if (const auto it = _impl->pathToSound.find(key); it != _impl->pathToSound.end())
        return it->second;

    if (!std::filesystem::is_regular_file(key))
        throw core::ResourceError("Sound file does not exist: " + key.string());
    if (_impl->sounds.size() >= MAX_SOUNDS)
        throw core::ResourceError("Too many sounds (max 65535): " + key.string());

    SoundSlot slot;
    slot.original = LoadSound(key.string().c_str());
    if (!IsSoundValid(slot.original))
        throw core::ResourceError("Failed to decode sound: " + key.string());
    for (Sound &alias : slot.aliases)
        alias = LoadSoundAlias(slot.original);

    const SoundId id{static_cast<std::uint16_t>(_impl->sounds.size())};
    _impl->sounds.push_back(slot);
    _impl->pathToSound.emplace(key, id);
    return id;
}

/* ** TODO(RTP-82): open the music as a stream, same path same id */
MusicId RaylibAudio::loadMusic(const std::filesystem::path &path)
{
    throw core::ResourceError("RaylibAudio::loadMusic is not implemented yet: " + path.string());
}

/*
** An unknown id plays nothing, like NullAudio. A free copy is used, else the
** one that started first is restarted.
*/
void RaylibAudio::playSound(SoundId id, float volume)
{
    if (_impl->volume.muted() || id.value >= _impl->sounds.size())
        return;

    SoundSlot &slot = _impl->sounds[id.value];
    std::array<Voice, COPIES_PER_SOUND> voices{};

    for (std::size_t i = 0; i < COPIES_PER_SOUND; ++i)
        voices[i] = {IsSoundPlaying(slot.copy(i)), slot.startedAt[i]};

    const std::size_t chosen = chooseVoice(voices);
    Sound &copy = slot.copy(chosen);

    SetSoundVolume(copy, _impl->volume.effectiveSfx(volume));
    PlaySound(copy);
    slot.startedAt[chosen] = ++_impl->playCounter;
}

/* ** TODO(RTP-82) */
void RaylibAudio::playMusic(MusicId /*id*/, bool /*loop*/) {}

/* ** TODO(RTP-82) */
void RaylibAudio::stopMusic() {}

void RaylibAudio::setMasterVolume(float volume)
{
    _impl->volume.setMaster(volume);
}

void RaylibAudio::setSfxVolume(float volume)
{
    _impl->volume.setSfx(volume);
}

void RaylibAudio::setMusicVolume(float volume)
{
    _impl->volume.setMusic(volume);
}

void RaylibAudio::setMuted(bool muted)
{
    _impl->volume.setMuted(muted);
}

const Volume &RaylibAudio::volumes() const
{
    return _impl->volume;
}

/* ** TODO(RTP-82): stream the current music */
void RaylibAudio::update() {}

}
