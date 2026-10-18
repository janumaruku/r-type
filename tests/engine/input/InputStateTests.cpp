/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** InputStateTests
*/

#include <gtest/gtest.h>

#include <cstddef>

#include "InputState.hpp"

using engine::input::InputEvent;
using engine::input::InputState;
using engine::input::Key;
using engine::input::KEY_COUNT;

namespace {
InputEvent event(InputEvent::Type type, Key key = Key::UNKNOWN)
{
    InputEvent result;
    result.type = type;
    result.key = key;
    return result;
}

InputEvent pressed(Key key)
{
    return event(InputEvent::Type::KEY_PRESSED, key);
}

InputEvent released(Key key)
{
    return event(InputEvent::Type::KEY_RELEASED, key);
}

testing::AssertionResult noBitSet(const InputState &state)
{
    for (std::size_t i = 0; i < KEY_COUNT; ++i) {
        const auto key = static_cast<Key>(i);
        if (state.isDown(key) || state.wasPressed(key)
            || state.wasReleased(key))
            return testing::AssertionFailure() << "key " << i << " has a bit set";
    }
    return testing::AssertionSuccess();
}
} // namespace

TEST(InputState, PressingAKeyMakesItHeldAndPressed)
{
    InputState state;

    state.apply(pressed(Key::KEY_A));

    EXPECT_TRUE(state.isDown(Key::KEY_A));
    EXPECT_TRUE(state.wasPressed(Key::KEY_A));
    EXPECT_FALSE(state.wasReleased(Key::KEY_A));
}

TEST(InputState, ANewFrameKeepsHeldAndClearsPressed)
{
    InputState state;
    state.apply(pressed(Key::KEY_A));

    state.beginFrame();

    EXPECT_TRUE(state.isDown(Key::KEY_A));
    EXPECT_FALSE(state.wasPressed(Key::KEY_A));
}

TEST(InputState, KeyRepeatCountsAsPressedOnlyOnTheFirstFrame)
{
    InputState state;
    state.apply(pressed(Key::KEY_A));
    EXPECT_TRUE(state.wasPressed(Key::KEY_A));

    state.beginFrame();
    state.apply(pressed(Key::KEY_A));

    EXPECT_TRUE(state.isDown(Key::KEY_A));
    EXPECT_FALSE(state.wasPressed(Key::KEY_A));
}

TEST(InputState, AQuickTapIsBothPressedAndReleasedInTheSameFrame)
{
    InputState state;

    state.apply(pressed(Key::KEY_A));
    state.apply(released(Key::KEY_A));

    EXPECT_FALSE(state.isDown(Key::KEY_A));
    EXPECT_TRUE(state.wasPressed(Key::KEY_A));
    EXPECT_TRUE(state.wasReleased(Key::KEY_A));
}

TEST(InputState, ReleasingAKeyThatIsNotHeldChangesNothing)
{
    InputState state;

    state.apply(released(Key::KEY_A));

    EXPECT_TRUE(noBitSet(state));
}

TEST(InputState, ReleaseAllReleasesEveryHeldKey)
{
    InputState state;
    state.apply(pressed(Key::UP));
    state.apply(pressed(Key::SPACE));
    state.beginFrame();

    state.releaseAll();

    EXPECT_FALSE(state.isDown(Key::UP));
    EXPECT_FALSE(state.isDown(Key::SPACE));
    EXPECT_TRUE(state.wasReleased(Key::UP));
    EXPECT_TRUE(state.wasReleased(Key::SPACE));
    EXPECT_FALSE(state.wasReleased(Key::KEY_A));
}

TEST(InputState, ReleaseAllKeepsAPressFromTheSameFrame)
{
    InputState state;
    state.apply(pressed(Key::SPACE));

    state.releaseAll();

    EXPECT_TRUE(state.wasPressed(Key::SPACE));
    EXPECT_TRUE(state.wasReleased(Key::SPACE));
}

TEST(InputState, UnknownKeysAndWindowEventsChangeNoBit)
{
    InputState state;

    state.apply(pressed(Key::UNKNOWN));
    state.apply(released(Key::UNKNOWN));
    state.apply(event(InputEvent::Type::RESIZED, Key::KEY_A));
    state.apply(event(InputEvent::Type::FOCUS_LOST, Key::KEY_A));

    EXPECT_TRUE(noBitSet(state));
}
