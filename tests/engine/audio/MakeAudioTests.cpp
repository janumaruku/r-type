/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** MakeAudioTests
*/

#include <gtest/gtest.h>

#include <memory>

#include "engine/audio/MakeAudio.hpp"
#include "engine/audio/RaylibAudio.hpp"
#include "engine/core/Errors.hpp"

using engine::audio::IAudio;
using engine::audio::makeAudio;
using engine::audio::RaylibAudio;

TEST(MakeAudioTests, NeverThrowsAndGivesAUsableAudio)
{
    std::unique_ptr<IAudio> audio;

    ASSERT_NO_THROW(audio = makeAudio());
    ASSERT_NE(audio, nullptr);
    audio->setMasterVolume(0.5F);
    EXPECT_FLOAT_EQ(audio->volumes().master(), 0.5F);
    EXPECT_NO_THROW(audio->update());
}

TEST(MakeAudioTests, ASecondRaylibAudioThrows)
{
    std::unique_ptr<RaylibAudio> first;

    try {
        first = std::make_unique<RaylibAudio>();
    } catch (const engine::core::EngineError &) {
        GTEST_SKIP() << "no sound device on this machine";
    }
    EXPECT_THROW(RaylibAudio{}, engine::core::EngineError);
}

TEST(MakeAudioTests, TheDeviceCanBeOpenedAgainAfterClosing)
{
    for (int i = 0; i < 2; ++i) {
        std::unique_ptr<IAudio> audio = makeAudio();
        EXPECT_NE(audio, nullptr);
    }
}
