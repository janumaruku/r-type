/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** KeyTableTests
*/

#include <gtest/gtest.h>

#include <cstddef>
#include <unordered_set>

#include "KeyTable.hpp"

using engine::input::Key;
using engine::input::KEY_COUNT;
using engine::render::detail::KEY_TABLE;
using engine::render::detail::keyIndex;

TEST(KeyTableTests, UnknownHasNoCode)
{
    EXPECT_EQ(KEY_TABLE[keyIndex(Key::UNKNOWN)], KEY_NULL);
}

TEST(KeyTableTests, EveryEngineKeyHasACode)
{
    for (std::size_t i = keyIndex(Key::UNKNOWN) + 1; i < KEY_COUNT; ++i)
        EXPECT_NE(KEY_TABLE[i], KEY_NULL)
            << engine::input::toString(static_cast<Key>(i)) << " has no code";
}

TEST(KeyTableTests, NoTwoEngineKeysShareACode)
{
    std::unordered_set<int> seen;

    for (std::size_t i = keyIndex(Key::UNKNOWN) + 1; i < KEY_COUNT; ++i)
        EXPECT_TRUE(seen.insert(KEY_TABLE[i]).second)
            << engine::input::toString(static_cast<Key>(i))
            << " shares code " << KEY_TABLE[i];
}
