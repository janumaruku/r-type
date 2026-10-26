/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** NullAudioTests
*/

#include <gtest/gtest.h>

#include <memory>

#include "engine/audio/IAudio.hpp"
#include "engine/audio/NullAudio.hpp"

using engine::audio::IAudio;
using engine::audio::MusicId;
using engine::audio::NullAudio;
using engine::audio::SoundId;

TEST(NullAudioTests, ThreeSoundsGetThreeDistinctIds)
{
    NullAudio audio;

    const SoundId shot = audio.loadSound("sounds/shot.wav");
    const SoundId boom = audio.loadSound("sounds/boom.wav");
    const SoundId join = audio.loadSound("sounds/join.wav");

    EXPECT_NE(shot, boom);
    EXPECT_NE(boom, join);
    EXPECT_NE(shot, join);
}

TEST(NullAudioTests, TheSamePathGivesTheSameId)
{
    NullAudio audio;

    EXPECT_EQ(audio.loadSound("sounds/shot.wav"), audio.loadSound("sounds/./shot.wav"));
    EXPECT_EQ(audio.loadMusic("music/stage1.ogg"), audio.loadMusic("music/stage1.ogg"));
}

TEST(NullAudioTests, MusicsGetDistinctIds)
{
    NullAudio audio;

    EXPECT_NE(audio.loadMusic("music/stage1.ogg"), audio.loadMusic("music/stage2.ogg"));
}

TEST(NullAudioTests, PlayingStoppingAndUpdatingDoNotThrow)
{
    const std::unique_ptr<IAudio> audio = std::make_unique<NullAudio>();
    const SoundId shot = audio->loadSound("sounds/shot.wav");
    const MusicId stage = audio->loadMusic("music/stage1.ogg");

    EXPECT_NO_THROW(audio->playSound(shot, 1.0F));
    EXPECT_NO_THROW(audio->playSound(SoundId{999}, 1.0F));
    EXPECT_NO_THROW(audio->playMusic(stage, true));
    EXPECT_NO_THROW(audio->update());
    EXPECT_NO_THROW(audio->stopMusic());
    EXPECT_NO_THROW(audio->stopMusic());
}

TEST(NullAudioTests, TheVolumeSettingsAreKept)
{
    NullAudio audio;

    audio.setMasterVolume(0.5F);
    audio.setSfxVolume(0.8F);
    audio.setMusicVolume(0.6F);
    audio.setMuted(true);

    EXPECT_FLOAT_EQ(audio.volumes().master(), 0.5F);
    EXPECT_FLOAT_EQ(audio.volumes().sfx(), 0.8F);
    EXPECT_FLOAT_EQ(audio.volumes().music(), 0.6F);
    EXPECT_TRUE(audio.volumes().muted());
}

TEST(NullAudioTests, SettingsGoThroughTheVolumeRules)
{
    NullAudio audio;

    audio.setMasterVolume(1.7F);
    EXPECT_FLOAT_EQ(audio.volumes().master(), 1.0F);
}
