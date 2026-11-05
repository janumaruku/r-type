/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** KeyTests
*/

#include <gtest/gtest.h>

#include <cstddef>
#include <optional>

#include "engine/input/Key.hpp"

using engine::input::Key;
using engine::input::KEY_COUNT;
using engine::input::keyFromString;
using engine::input::toString;

TEST(KeyTests, EveryKeyNameReadsBackAsTheSameKey)
{
    for (std::size_t id = 1; id < KEY_COUNT; ++id) {
        const auto key = static_cast<Key>(id);

        EXPECT_EQ(keyFromString(toString(key)), std::optional<Key>{key}) << toString(key);
    }
}

TEST(KeyTests, FunctionKeysAreNamedF1ToF12)
{
    EXPECT_EQ(toString(Key::F1), "F1");
    EXPECT_EQ(toString(Key::F12), "F12");
    EXPECT_EQ(keyFromString("F1"), std::optional<Key>{Key::F1});
    EXPECT_EQ(keyFromString("f12"), std::optional<Key>{Key::F12});
}

TEST(KeyTests, FunctionKeysPastF12AreUnknown)
{
    EXPECT_EQ(keyFromString("F13"), std::nullopt);
    EXPECT_EQ(keyFromString("F0"), std::nullopt);
}
