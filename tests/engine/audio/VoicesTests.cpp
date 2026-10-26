/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** VoicesTests
*/

#include <gtest/gtest.h>

#include <array>
#include <vector>

#include "engine/audio/Voices.hpp"
#include "engine/core/Errors.hpp"

using engine::audio::chooseVoice;
using engine::audio::Voice;

TEST(VoicesTests, NoneIsPlayingSoTheFirstIsChosen)
{
    const std::array<Voice, 5> voices{};

    EXPECT_EQ(chooseVoice(voices), 0U);
}

TEST(VoicesTests, TheFirstFreeVoiceIsChosen)
{
    const std::array<Voice, 5> voices{{{true, 0}, {true, 1}, {false, 0}, {false, 0}, {false, 0}}};

    EXPECT_EQ(chooseVoice(voices), 2U);
}

TEST(VoicesTests, AFreeVoiceWinsOverAnOlderPlayingOne)
{
    const std::array<Voice, 5> voices{{{true, 7}, {true, 2}, {true, 9}, {true, 5}, {false, 0}}};

    EXPECT_EQ(chooseVoice(voices), 4U);
}

TEST(VoicesTests, AllPlayingSoTheOneThatStartedFirstIsCut)
{
    const std::array<Voice, 5> voices{{{true, 4}, {true, 2}, {true, 5}, {true, 1}, {true, 3}}};

    EXPECT_EQ(chooseVoice(voices), 3U);
}

TEST(VoicesTests, ASingleVoiceIsAlwaysChosen)
{
    const std::array<Voice, 1> voices{{{true, 42}}};

    EXPECT_EQ(chooseVoice(voices), 0U);
}

TEST(VoicesTests, NoVoicesThrows)
{
    const std::vector<Voice> voices;

    EXPECT_THROW((void)chooseVoice(voices), engine::core::EngineError);
}
