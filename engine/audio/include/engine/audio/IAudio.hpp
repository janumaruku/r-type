/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** IAudio.hpp
*/

#ifndef IAUDIO_HPP
#define IAUDIO_HPP

#include <filesystem>

#include "engine/audio/AudioId.hpp"
#include "engine/audio/Volume.hpp"

namespace engine::audio {

/*
** Audio knows how to play, not when: the client picks the sounds from game
** events. Volumes go from 0 to 1, and the effective volume is master times
** the channel. update() must be called once per frame, from the frame loop,
** or streamed music stutters. Not copyable: an implementation owns a device.
*/
class IAudio {
public:
    IAudio() = default;

    IAudio(const IAudio &) = delete;

    IAudio(IAudio &&) = delete;

    virtual ~IAudio();

    IAudio &operator=(const IAudio &) = delete;

    IAudio &operator=(IAudio &&) = delete;

    virtual SoundId loadSound(const std::filesystem::path &path) = 0;

    virtual MusicId loadMusic(const std::filesystem::path &path) = 0;

    virtual void playSound(SoundId id, float volume) = 0;

    virtual void playMusic(MusicId id, bool loop) = 0;

    virtual void stopMusic() = 0;

    virtual void setMasterVolume(float volume) = 0;

    virtual void setSfxVolume(float volume) = 0;

    virtual void setMusicVolume(float volume) = 0;

    virtual void setMuted(bool muted) = 0;

    /* ** The current settings, for an options menu or a HUD */
    [[nodiscard]] virtual const Volume &volumes() const = 0;

    virtual void update() = 0;
};

}

#endif // !IAUDIO_HPP
