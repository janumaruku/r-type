/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** WidgetKeyTests.cpp
*/

#include <cstdint>
#include <string>
#include <variant>

#include <gtest/gtest.h>

#include "kollection/core/WidgetKey.hpp"

using kollection::core::WidgetKey;

TEST(KollectionWidgetKey, default_holds_no_key)
{
    EXPECT_TRUE(std::holds_alternative<std::monostate>(WidgetKey{}.value));
}

TEST(KollectionWidgetKey, two_empty_keys_are_equal)
{
    EXPECT_EQ(WidgetKey{}, WidgetKey{});
}

TEST(KollectionWidgetKey, numeric_keys_compare_by_value)
{
    EXPECT_EQ(WidgetKey{std::int64_t{42}}, WidgetKey{std::int64_t{42}});
    EXPECT_NE(WidgetKey{std::int64_t{42}}, WidgetKey{std::int64_t{7}});
}

TEST(KollectionWidgetKey, named_keys_compare_by_value)
{
    EXPECT_EQ(WidgetKey{std::string{"play"}}, WidgetKey{std::string{"play"}});
    EXPECT_NE(WidgetKey{std::string{"play"}}, WidgetKey{std::string{"quit"}});
}

TEST(KollectionWidgetKey, keys_of_different_kinds_differ)
{
    EXPECT_NE(WidgetKey{std::int64_t{42}}, WidgetKey{std::string{"42"}});
    EXPECT_NE(WidgetKey{std::int64_t{0}}, WidgetKey{});
    EXPECT_NE(WidgetKey{std::string{}}, WidgetKey{});
}
