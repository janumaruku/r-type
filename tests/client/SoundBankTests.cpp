/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SoundBankTests
*/

#include "SoundBank.hpp"

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <vector>

#include "engine/audio/AudioId.hpp"
#include "engine/audio/IAudio.hpp"
#include "engine/audio/Volume.hpp"
#include "engine/core/Errors.hpp"

namespace rtype::client {
namespace {
using engine::audio::IAudio;
using engine::audio::MusicId;
using engine::audio::SoundId;
using engine::audio::Volume;

/** Sets an environment variable for one test and puts it back afterwards. */
class ScopedEnv {
public:
    ScopedEnv(const char *name, const char *value): _name(name)
    {
#ifdef _WIN32
        char *saved      = nullptr;
        std::size_t size = 0;

        if (_dupenv_s(&saved, &size, name) == 0 && saved != nullptr) {
            _saved = saved;
            std::free(saved);
        }
#else
        if (const char *saved = std::getenv(name))
            _saved = saved;
#endif
        set(value);
    }

    ~ScopedEnv()
    {
        set(_saved ? _saved->c_str() : nullptr);
    }

    ScopedEnv(const ScopedEnv &)            = delete;
    ScopedEnv &operator=(const ScopedEnv &) = delete;

private:
    void set(const char *value) const
    {
#ifdef _WIN32
        _putenv_s(_name, value ? value : "");
#else
        if (value)
            setenv(_name, value, 1);
        else
            unsetenv(_name);
#endif
    }

    const char *_name;
    std::optional<std::string> _saved;
};

/** An IAudio that records each loaded path and hands out ids 1, 2, 3... */
class LoadingAudio final: public IAudio {
public:
    SoundId loadSound(const std::filesystem::path &path) override
    {
        soundPaths.push_back(path);
        return SoundId{static_cast<std::uint16_t>(soundPaths.size())};
    }
    MusicId loadMusic(const std::filesystem::path &path) override
    {
        musicPaths.push_back(path);
        return MusicId{static_cast<std::uint16_t>(musicPaths.size())};
    }
    void playSound(SoundId, float) override
    {}
    void playMusic(MusicId, bool) override
    {}
    void stopMusic() override
    {}
    void setMasterVolume(float) override
    {}
    void setSfxVolume(float) override
    {}
    void setMusicVolume(float) override
    {}
    void setMuted(bool) override
    {}
    [[nodiscard]] const Volume &volumes() const override
    {
        return _volumes;
    }
    void update() override
    {}

    std::vector<std::filesystem::path> soundPaths;
    std::vector<std::filesystem::path> musicPaths;

private:
    Volume _volumes;
};

std::vector<std::string> fileNames(
    const std::vector<std::filesystem::path> &paths)
{
    std::vector<std::string> names;

    for (const auto &path: paths)
        names.push_back(path.parent_path().filename().string() + "/" +
            path.filename().string());
    return names;
}
} // namespace

TEST(SoundBank, LoadsTheFiveSoundsAndTheMusicOfTheSoundTable)
{
    const ScopedEnv assets("RTYPE_ASSETS_DIR", RTYPE_ASSETS_DIR);
    LoadingAudio audio;

    (void)SoundBank::load(audio);

    const std::vector<std::string> sounds{"sounds/shot.wav",
        "sounds/explosion_small.wav", "sounds/explosion_big.wav",
        "sounds/ui_join.wav", "sounds/ui_leave.wav"};
    EXPECT_EQ(fileNames(audio.soundPaths), sounds);
    EXPECT_EQ(fileNames(audio.musicPaths),
        std::vector<std::string>{"music/stage1.ogg"});
    for (const auto &path: audio.soundPaths)
        EXPECT_TRUE(std::filesystem::is_regular_file(path)) << path;
    EXPECT_TRUE(std::filesystem::is_regular_file(audio.musicPaths.front()));
}

TEST(SoundBank, KeepsTheIdEachLoadReturned)
{
    const ScopedEnv assets("RTYPE_ASSETS_DIR", RTYPE_ASSETS_DIR);
    LoadingAudio audio;

    const SoundBank bank = SoundBank::load(audio);

    EXPECT_EQ(bank.shot, SoundId{1});
    EXPECT_EQ(bank.explosionSmall, SoundId{2});
    EXPECT_EQ(bank.explosionBig, SoundId{3});
    EXPECT_EQ(bank.join, SoundId{4});
    EXPECT_EQ(bank.leave, SoundId{5});
    EXPECT_EQ(bank.stage1, MusicId{1});
}

TEST(SoundBank, AMissingFileThrowsBeforeTheAudioSeesIt)
{
    const std::filesystem::path empty =
        std::filesystem::temp_directory_path() / "rtype-soundbank-tests";
    std::filesystem::create_directories(empty);
    const ScopedEnv assets("RTYPE_ASSETS_DIR", empty.string().c_str());
    LoadingAudio audio;

    EXPECT_THROW((void)SoundBank::load(audio), engine::core::ResourceError);
    EXPECT_TRUE(audio.soundPaths.empty());
}

} // namespace rtype::client
