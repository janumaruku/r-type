/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** LetterboxTests
*/

#include <gtest/gtest.h>

#include "Letterbox.hpp"

using engine::core::FloatRect;
using engine::core::Vec2f;
using engine::core::Vec2u;
using engine::render::letterbox;

namespace {
const Vec2f LOGICAL{1280.0F, 720.0F};

void expectRect(const FloatRect &actual, float x, float y, float width,
    float height)
{
    EXPECT_FLOAT_EQ(actual.origin().x(), x);
    EXPECT_FLOAT_EQ(actual.origin().y(), y);
    EXPECT_FLOAT_EQ(actual.width(), width);
    EXPECT_FLOAT_EQ(actual.height(), height);
}
}

TEST(LetterboxTests, TheLogicalSizeFillsTheWindowExactly)
{
    expectRect(letterbox(Vec2u{1280, 720}, LOGICAL), 0, 0, 1280, 720);
}

TEST(LetterboxTests, TheSameRatioScalesWithoutBars)
{
    expectRect(letterbox(Vec2u{1920, 1080}, LOGICAL), 0, 0, 1920, 1080);
}

TEST(LetterboxTests, ATallerWindowGetsBarsAtTheTopAndBottom)
{
    expectRect(letterbox(Vec2u{1920, 1200}, LOGICAL), 0, 60, 1920, 1080);
}

TEST(LetterboxTests, ASquareWindowIsCentredVertically)
{
    expectRect(letterbox(Vec2u{1000, 1000}, LOGICAL), 0, 218.75F, 1000,
        562.5F);
}

TEST(LetterboxTests, AWiderWindowGetsBarsOnTheSides)
{
    expectRect(letterbox(Vec2u{2560, 720}, LOGICAL), 640, 0, 1280, 720);
}

TEST(LetterboxTests, AMinimisedWindowGivesAnEmptyRectangle)
{
    expectRect(letterbox(Vec2u{0, 0}, LOGICAL), 0, 0, 0, 0);
    expectRect(letterbox(Vec2u{800, 0}, LOGICAL), 0, 0, 0, 0);
}
