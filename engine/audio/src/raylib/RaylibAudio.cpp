/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RaylibAudio.cpp
*/

#include "engine/audio/RaylibAudio.hpp"

#include <array>
#include <vector>

#include "engine/core/Errors.hpp"

#include <raylib.h>

namespace engine::audio {

namespace {

constexpr std::size_t ALIASES_PER_SOUND = 4;

/*
** One loaded sound and its aliases: 5 copies that share the same samples, so
** the same sound can overlap itself without the memory growing.
*/
struct SoundSlot {
    Sound original{};
    std::array<Sound, ALIASES_PER_SOUND> aliases{};
};

}

struct RaylibAudio::Impl {
    Volume volume;
    std::vector<SoundSlot> sounds;   // index == SoundId::value
    std::vector<Music> musics;       // index == MusicId::value
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

/* ** TODO(RTP-81): load the sound and its 4 aliases, same path same id */
SoundId RaylibAudio::loadSound(const std::filesystem::path &path)
{
    throw core::ResourceError("RaylibAudio::loadSound is not implemented yet: " + path.string());
}

/* ** TODO(RTP-82): open the music as a stream, same path same id */
MusicId RaylibAudio::loadMusic(const std::filesystem::path &path)
{
    throw core::ResourceError("RaylibAudio::loadMusic is not implemented yet: " + path.string());
}

/* ** TODO(RTP-81): play a free copy, chosen by chooseVoice */
void RaylibAudio::playSound(SoundId /*id*/, float /*volume*/) {}

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
