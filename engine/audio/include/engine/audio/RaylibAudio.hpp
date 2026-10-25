/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RaylibAudio.hpp
*/

#ifndef RAYLIB_AUDIO_HPP
#define RAYLIB_AUDIO_HPP

#include <filesystem>
#include <memory>

#include "engine/audio/IAudio.hpp"

namespace engine::audio {

/*
** The real IAudio, on the Raylib sound device. Raylib has one global device,
** so only one RaylibAudio can exist at a time, like render::Window. It
** doesn't need the window and can be created before or after it. Build it
** through makeAudio(), which falls back to NullAudio when it throws.
*/
class RaylibAudio final : public IAudio {
public:
    /* ** Throws EngineError when the device can't open or one already exists */
    RaylibAudio();

    /* ** Unloads every alias before its original sound, then closes the device */
    ~RaylibAudio() override;

    SoundId loadSound(const std::filesystem::path &path) override;

    MusicId loadMusic(const std::filesystem::path &path) override;

    void playSound(SoundId id, float volume) override;

    void playMusic(MusicId id, bool loop) override;

    void stopMusic() override;

    void setMasterVolume(float volume) override;

    void setSfxVolume(float volume) override;

    void setMusicVolume(float volume) override;

    void setMuted(bool muted) override;

    [[nodiscard]] const Volume &volumes() const override;

    void update() override;

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif // !RAYLIB_AUDIO_HPP
