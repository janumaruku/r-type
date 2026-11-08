/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** InputSamplingTests
*/

#include <gtest/gtest.h>

#include "Actions.hpp"
#include "InterpolationSystem.hpp"

using engine::input::InputEvent;
using engine::input::Key;
using rtype::client::Action;

TEST(SampleButtons, UpAndFireHeldGive0x11)
{
    engine::input::ActionMap<Action> actions;
    engine::input::InputState input;

    rtype::client::bindDefaults(actions);
    input.apply(InputEvent{.type = InputEvent::Type::KEY_PRESSED,
        .key = Key::UP});
    input.apply(InputEvent{.type = InputEvent::Type::KEY_PRESSED,
        .key = Key::SPACE});
    EXPECT_EQ(rtype::client::sampleButtons(actions, input), 0x11);
}

TEST(SampleButtons, NothingHeldGivesZero)
{
    engine::input::ActionMap<Action> actions;
    const engine::input::InputState input;

    rtype::client::bindDefaults(actions);
    EXPECT_EQ(rtype::client::sampleButtons(actions, input), 0);
}
