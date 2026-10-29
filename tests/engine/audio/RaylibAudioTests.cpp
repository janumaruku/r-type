/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RaylibAudioTests
*/

#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <random>
#include <string>

#include "engine/audio/RaylibAudio.hpp"
#include "engine/core/Errors.hpp"

using engine::audio::MusicId;
using engine::audio::RaylibAudio;
using engine::audio::SoundId;

namespace {

void writeLittleEndian(std::ofstream &out, std::uint32_t value, int bytes)
{
    for (int i = 0; i < bytes; ++i)
        out.put(static_cast<char>((value >> (8 * i)) & 0xFF));
}

/* ** A tenth of a second of silence, 16-bit mono PCM at 22050 Hz */
void writeSilentWav(const std::filesystem::path &path)
{
    constexpr std::uint32_t SAMPLE_RATE = 22050; // NOLINT
    constexpr std::uint32_t SAMPLES = SAMPLE_RATE / 10; // NOLINT
    constexpr std::uint32_t DATA_SIZE = SAMPLES * 2; // NOLINT
    std::ofstream out(path, std::ios::binary);

    out << "RIFF";
    writeLittleEndian(out, 36 + DATA_SIZE, 4);
    out << "WAVEfmt ";
    writeLittleEndian(out, 16, 4);
    writeLittleEndian(out, 1, 2);
    writeLittleEndian(out, 1, 2);
    writeLittleEndian(out, SAMPLE_RATE, 4);
    writeLittleEndian(out, SAMPLE_RATE * 2, 4);
    writeLittleEndian(out, 2, 2);
    writeLittleEndian(out, 16, 2);
    out << "data";
    writeLittleEndian(out, DATA_SIZE, 4);
    out << std::string(DATA_SIZE, '\0');
}

void writeBigEndian(std::ofstream &out, std::uint32_t value, int bytes)
{
    for (int i = bytes - 1; i >= 0; --i)
        out.put(static_cast<char>((value >> (8 * i)) & 0xFF));
}

/*
** One QOA frame of near silence, 5120 mono samples at 22050 Hz, all slices
** zero. Music tests use QOA, not WAV: Raylib 5.5 leaks the decoder of a WAV
** music stream on unload, and it has no OGG encoder to write a test file.
*/
void writeQuietQoa(const std::filesystem::path &path)
{
    constexpr std::uint32_t SAMPLES = 5120; // NOLINT
    constexpr std::uint32_t SLICES = SAMPLES / 20; // NOLINT
    constexpr std::uint32_t FRAME_SIZE = 8 + 16 + (8 * SLICES); // NOLINT
    std::ofstream out(path, std::ios::binary);

    out << "qoaf";
    writeBigEndian(out, SAMPLES, 4);
    writeBigEndian(out, 1, 1);
    writeBigEndian(out, 22050, 3);
    writeBigEndian(out, SAMPLES, 2);
    writeBigEndian(out, FRAME_SIZE, 2);
    out << std::string(FRAME_SIZE - 8, '\0');
}

}

/* ** Skipped on a machine without a sound device, like most CI runners */
class RaylibAudioTests : public testing::Test {
protected:
    void SetUp() override
    {
        try {
            _audio = std::make_unique<RaylibAudio>();
        } catch (const engine::core::EngineError &) {
            GTEST_SKIP() << "no sound device on this machine";
        }
        _dir = std::filesystem::temp_directory_path() / ("rtype_audio_" + std::to_string(std::random_device{}()));
        std::filesystem::create_directories(_dir);
        writeSilentWav(_dir / "shot.wav");
        writeSilentWav(_dir / "boom.wav");
        writeQuietQoa(_dir / "stage.qoa");
        writeQuietQoa(_dir / "boss.qoa");
        std::ofstream(_dir / "broken.wav") << "not a sound";
    }

    void TearDown() override
    {
        _audio.reset();
        if (!_dir.empty())
            std::filesystem::remove_all(_dir);
    }

    std::unique_ptr<RaylibAudio> _audio;
    std::filesystem::path _dir;
};

TEST_F(RaylibAudioTests, TheSamePathGivesTheSameId)
{
    const SoundId shot = _audio->loadSound(_dir / "shot.wav");

    EXPECT_EQ(_audio->loadSound(_dir / "." / "shot.wav"), shot);
    EXPECT_NE(_audio->loadSound(_dir / "boom.wav"), shot);
}

TEST_F(RaylibAudioTests, AMissingFileThrowsWithThePath)
{
    try {
        (void)_audio->loadSound(_dir / "missing.wav");
        FAIL() << "expected a ResourceError";
    } catch (const engine::core::ResourceError &error) {
        EXPECT_NE(std::string{error.what()}.find("missing.wav"), std::string::npos);
    }
}

TEST_F(RaylibAudioTests, AFileThatIsNotASoundThrows)
{
    EXPECT_THROW((void)_audio->loadSound(_dir / "broken.wav"), engine::core::ResourceError);
}

TEST_F(RaylibAudioTests, TenPlaysInARowOverlapWithoutThrowing)
{
    const SoundId boom = _audio->loadSound(_dir / "boom.wav");

    for (int i = 0; i < 10; ++i)
        EXPECT_NO_THROW(_audio->playSound(boom, 1.0F));
}

TEST_F(RaylibAudioTests, PlayingWhileMutedOrWithAnUnknownIdDoesNothing)
{
    const SoundId shot = _audio->loadSound(_dir / "shot.wav");

    _audio->setMuted(true);
    EXPECT_NO_THROW(_audio->playSound(shot, 1.0F));
    _audio->setMuted(false);
    EXPECT_NO_THROW(_audio->playSound(SoundId{999}, 1.0F));
}

TEST_F(RaylibAudioTests, MusicFollowsTheSameLoadingRules)
{
    const MusicId stage = _audio->loadMusic(_dir / "stage.qoa");

    EXPECT_EQ(_audio->loadMusic(_dir / "." / "stage.qoa"), stage);
    EXPECT_NE(_audio->loadMusic(_dir / "boss.qoa"), stage);
    EXPECT_THROW((void)_audio->loadMusic(_dir / "missing.ogg"), engine::core::ResourceError);
    EXPECT_THROW((void)_audio->loadMusic(_dir / "broken.wav"), engine::core::ResourceError);
}

TEST_F(RaylibAudioTests, PlayingUpdatingAndStoppingMusicDoNotThrow)
{
    const MusicId stage = _audio->loadMusic(_dir / "stage.qoa");
    const MusicId boss = _audio->loadMusic(_dir / "boss.qoa");

    EXPECT_NO_THROW(_audio->stopMusic());
    EXPECT_NO_THROW(_audio->playMusic(stage, true));
    EXPECT_NO_THROW(_audio->playMusic(stage, true));
    EXPECT_NO_THROW(_audio->update());
    EXPECT_NO_THROW(_audio->playMusic(boss, false));
    EXPECT_NO_THROW(_audio->playMusic(MusicId{999}, true));
    for (int i = 0; i < 20; ++i)
        EXPECT_NO_THROW(_audio->update());
    EXPECT_NO_THROW(_audio->stopMusic());
    EXPECT_NO_THROW(_audio->update());
}

TEST_F(RaylibAudioTests, VolumeAndMuteChangesWhileMusicPlays)
{
    const MusicId stage = _audio->loadMusic(_dir / "stage.qoa");

    _audio->playMusic(stage, true);
    EXPECT_NO_THROW(_audio->setMasterVolume(0.5F));
    EXPECT_NO_THROW(_audio->setMusicVolume(0.2F));
    EXPECT_NO_THROW(_audio->setMuted(true));
    EXPECT_NO_THROW(_audio->update());
    EXPECT_NO_THROW(_audio->setMuted(false));
    EXPECT_FLOAT_EQ(_audio->volumes().effectiveMusic(), 0.1F);
}
