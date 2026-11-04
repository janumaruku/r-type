/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** EventTests.cpp
*/

#include <gtest/gtest.h>

#include "kollection/core/Event.hpp"

using kollection::core::Event;
using kollection::core::KeyCode;
using kollection::core::KeyEvent;
using kollection::core::PointerEvent;
using kollection::core::ResizeEvent;
using kollection::core::Size;

TEST(KollectionKeyEvent, default_is_a_char_press_without_modifiers)
{
    const KeyEvent event;

    EXPECT_EQ(event.key, KeyCode::Char);
    EXPECT_EQ(event.phase, KeyEvent::Phase::Down);
    EXPECT_EQ(event.codepoint, U'\0');
    EXPECT_FALSE(event.ctrl);
    EXPECT_FALSE(event.alt);
    EXPECT_FALSE(event.shift);
}

TEST(KollectionPointerEvent, default_is_a_left_press_at_the_origin)
{
    const PointerEvent event;

    EXPECT_EQ(event.pos.x, 0);
    EXPECT_EQ(event.pos.y, 0);
    EXPECT_EQ(event.phase, PointerEvent::Phase::Down);
    EXPECT_EQ(event.button, 0);
}

TEST(KollectionEvent, holds_each_kind_of_event)
{
    const Event key = KeyEvent{.key = KeyCode::Tab, .shift = true};
    const Event pointer = PointerEvent{.pos = {120, 40}, .phase = PointerEvent::Phase::Up};
    const Event resize = ResizeEvent{.newSize = {1280, 720}};

    ASSERT_TRUE(std::holds_alternative<KeyEvent>(key));
    EXPECT_EQ(std::get<KeyEvent>(key).key, KeyCode::Tab);
    EXPECT_TRUE(std::get<KeyEvent>(key).shift);

    ASSERT_TRUE(std::holds_alternative<PointerEvent>(pointer));
    EXPECT_EQ(std::get<PointerEvent>(pointer).pos.x, 120);
    EXPECT_EQ(std::get<PointerEvent>(pointer).phase, PointerEvent::Phase::Up);

    ASSERT_TRUE(std::holds_alternative<ResizeEvent>(resize));
    EXPECT_EQ(std::get<ResizeEvent>(resize).newSize, (Size{1280, 720}));
}
