/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** NullAudio.hpp
*/

#ifndef NULL_AUDIO_HPP
#define NULL_AUDIO_HPP

#include <filesystem>
#include <unordered_map>

#include "engine/audio/IAudio.hpp"

namespace engine::audio {

/*
** Used when there's no sound device, and in tests: every call succeeds and
** plays nothing. Loads never open the file, but give out ids like the real
** implementation, the same id for the same path, and the volumes are kept so
** an options menu still shows the right values.
*/
class NullAudio final : public IAudio {
public:
    NullAudio() = default;

    ~NullAudio() override = default;

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
    Volume _volume;
    std::unordered_map<std::filesystem::path, SoundId> _sounds;
    std::unordered_map<std::filesystem::path, MusicId> _musics;
};

}

#endif // !NULL_AUDIO_HPP
