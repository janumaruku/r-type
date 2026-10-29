/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** TypesTests
*/

#include <gtest/gtest.h>

#include "rtype/game/Types.hpp"

using namespace rtype::game;

TEST(InputButtons, HeldReportsOnlyTheSetBits)
{
    const InputButtons buttons{InputButtons::UP | InputButtons::FIRE};

    EXPECT_TRUE(buttons.held(InputButtons::UP));
    EXPECT_TRUE(buttons.held(InputButtons::FIRE));
    EXPECT_FALSE(buttons.held(InputButtons::DOWN));
}

TEST(InputButtons, DefaultHoldsNothing)
{
    const InputButtons buttons{};

    for (const auto button : {InputButtons::UP, InputButtons::DOWN,
             InputButtons::LEFT, InputButtons::RIGHT, InputButtons::FIRE})
        EXPECT_FALSE(buttons.held(button)) << static_cast<int>(button);
}
