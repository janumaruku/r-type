/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** StylesTests.cpp
*/

#include <gtest/gtest.h>

#include "kollection/core/Styles.hpp"

using kollection::core::Color;
using kollection::core::TextRun;
using kollection::core::TextStyle;

TEST(KollectionTextStyle, default_has_every_field_unset)
{
    const TextStyle style;

    EXPECT_FALSE(style.fg.has_value());
    EXPECT_FALSE(style.bg.has_value());
    EXPECT_FALSE(style.size.has_value());
    EXPECT_FALSE(style.bold.has_value());
    EXPECT_FALSE(style.underline.has_value());
    EXPECT_FALSE(style.font.has_value());
}

TEST(KollectionTextStyle, unset_differs_from_set_to_the_default_value)
{
    EXPECT_NE(TextStyle{}, (TextStyle{.bold = false}));
    EXPECT_NE(TextStyle{}, (TextStyle{.fg = Color::BLACK}));
}

TEST(KollectionTextStyle, merge_of_two_empty_styles_is_empty)
{
    EXPECT_EQ(TextStyle{}.merge(TextStyle{}), TextStyle{});
}

TEST(KollectionTextStyle, merge_keeps_base_fields_the_override_leaves_unset)
{
    const TextStyle base{.fg = Color::WHITE, .size = 20, .font = 2};
    const TextStyle merged = base.merge(TextStyle{.bold = true});

    EXPECT_EQ(merged.fg, Color::WHITE);
    EXPECT_EQ(merged.size, 20);
    EXPECT_EQ(merged.font, 2);
    EXPECT_EQ(merged.bold, true);
}

TEST(KollectionTextStyle, merge_lets_set_override_fields_win)
{
    const TextStyle base{.fg = Color::WHITE, .size = 20, .bold = true};
    const TextStyle merged =
        base.merge(TextStyle{.fg = Color::BLUE, .size = 32, .bold = false});

    EXPECT_EQ(merged.fg, Color::BLUE);
    EXPECT_EQ(merged.size, 32);
    EXPECT_EQ(merged.bold, false);
}

TEST(KollectionTextStyle, merge_onto_an_empty_base_gives_the_override)
{
    const TextStyle overrides{.bg = Color::TRANSPARENT, .underline = true};

    EXPECT_EQ(TextStyle{}.merge(overrides), overrides);
}

TEST(KollectionTextStyle, merge_with_an_empty_override_gives_the_base)
{
    const TextStyle base{.fg = Color::BLUE, .size = 12, .underline = false};

    EXPECT_EQ(base.merge(TextStyle{}), base);
}

TEST(KollectionTextStyle, merge_covers_every_field)
{
    const TextStyle base{.fg = Color::WHITE, .bg = Color::BLACK, .size = 10,
        .bold = true, .underline = true, .font = 1};

    EXPECT_EQ(base.merge(TextStyle{}), base);
    EXPECT_EQ(TextStyle{}.merge(base), base);
}

TEST(KollectionTextStyle, merge_leaves_both_inputs_unchanged)
{
    const TextStyle base{.fg = Color::WHITE};
    const TextStyle overrides{.fg = Color::BLUE, .bold = true};
    const TextStyle baseCopy = base;
    const TextStyle overridesCopy = overrides;

    (void)base.merge(overrides);
    EXPECT_EQ(base, baseCopy);
    EXPECT_EQ(overrides, overridesCopy);
}

TEST(KollectionTextStyle, merges_layer_from_outside_in)
{
    const TextStyle ambient{.fg = Color::WHITE, .size = 20};
    const TextStyle section{.size = 24};
    const TextStyle word{.bold = true};
    const TextStyle effective = ambient.merge(section).merge(word);

    EXPECT_EQ(effective,
        (TextStyle{.fg = Color::WHITE, .size = 24, .bold = true}));
}

TEST(KollectionTextRun, equality_compares_text_and_style)
{
    const TextRun run{.text = "Play", .style = {.bold = true}};

    EXPECT_EQ(run, (TextRun{.text = "Play", .style = {.bold = true}}));
    EXPECT_NE(run, (TextRun{.text = "Quit", .style = {.bold = true}}));
    EXPECT_NE(run, (TextRun{.text = "Play", .style = {.bold = false}}));
}
