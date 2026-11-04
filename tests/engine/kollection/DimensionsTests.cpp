/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** DimensionsTests.cpp
*/

#include <gtest/gtest.h>

#include "kollection/core/Dimensions.hpp"

using kollection::core::Alignment;
using kollection::core::EdgeInsets;
using kollection::core::INF;
using kollection::core::Offset;
using kollection::core::Rect;
using kollection::core::Size;

TEST(KollectionSize, default_is_zero)
{
    EXPECT_EQ(Size{}, (Size{.width = 0, .height = 0}));
}

TEST(KollectionSize, equality_compares_both_dimensions)
{
    EXPECT_EQ((Size{4, 2}), (Size{4, 2}));
    EXPECT_NE((Size{4, 2}), (Size{2, 4}));
}

TEST(KollectionInf, is_larger_than_any_finite_length)
{
    EXPECT_GT(INF, 1e300);
    EXPECT_EQ(INF + 1, INF);
}

TEST(KollectionOffset, addition_is_per_component)
{
    EXPECT_EQ((Offset{10, 20} + Offset{1, -2}), (Offset{11, 18}));
}

TEST(KollectionOffset, subtraction_is_per_component)
{
    EXPECT_EQ((Offset{10, 20} - Offset{1, -2}), (Offset{9, 22}));
}

TEST(KollectionOffset, subtraction_undoes_addition)
{
    const Offset origin{3.5, -7};
    const Offset delta{1.25, 4};

    EXPECT_EQ((origin + delta) - delta, origin);
}

TEST(KollectionRect, contains_its_top_left_corner)
{
    const Rect box{.x = 2, .y = 3, .width = 10, .height = 4};

    EXPECT_TRUE(box.contains({2, 3}));
}

TEST(KollectionRect, contains_an_inner_point)
{
    const Rect box{.x = 2, .y = 3, .width = 10, .height = 4};

    EXPECT_TRUE(box.contains({7, 5}));
    EXPECT_TRUE(box.contains({11.999, 6.999}));
}

TEST(KollectionRect, excludes_its_right_and_bottom_edges)
{
    const Rect box{.x = 2, .y = 3, .width = 10, .height = 4};

    EXPECT_FALSE(box.contains({12, 5}));
    EXPECT_FALSE(box.contains({7, 7}));
    EXPECT_FALSE(box.contains({12, 7}));
}

TEST(KollectionRect, excludes_points_before_its_left_and_top_edges)
{
    const Rect box{.x = 2, .y = 3, .width = 10, .height = 4};

    EXPECT_FALSE(box.contains({1.999, 5}));
    EXPECT_FALSE(box.contains({7, 2.999}));
}

TEST(KollectionRect, touching_rectangles_never_share_a_point)
{
    const Rect left{.x = 0, .y = 0, .width = 5, .height = 5};
    const Rect right{.x = 5, .y = 0, .width = 5, .height = 5};
    const Offset border{5, 2};

    EXPECT_FALSE(left.contains(border));
    EXPECT_TRUE(right.contains(border));
}

TEST(KollectionRect, empty_rectangle_contains_nothing)
{
    const Rect line{.x = 0, .y = 0, .width = 0, .height = 5};

    EXPECT_FALSE(line.contains({0, 0}));
    EXPECT_FALSE(Rect{}.contains({0, 0}));
}

TEST(KollectionEdgeInsets, all_sets_every_side)
{
    EXPECT_EQ(EdgeInsets::all(3),
        (EdgeInsets{.left = 3, .top = 3, .right = 3, .bottom = 3}));
}

TEST(KollectionEdgeInsets, symmetric_pairs_opposite_sides)
{
    EXPECT_EQ(EdgeInsets::symmetric(16, 8),
        (EdgeInsets{.left = 16, .top = 8, .right = 16, .bottom = 8}));
}

TEST(KollectionEdgeInsets, horizontal_and_vertical_sum_opposite_sides)
{
    const EdgeInsets insets{.left = 1, .top = 2, .right = 3, .bottom = 4};

    EXPECT_EQ(insets.horizontal(), 4);
    EXPECT_EQ(insets.vertical(), 6);
}

TEST(KollectionEdgeInsets, default_is_zero_on_every_side)
{
    EXPECT_EQ(EdgeInsets{}.horizontal(), 0);
    EXPECT_EQ(EdgeInsets{}.vertical(), 0);
}

TEST(KollectionAlignment, named_alignments)
{
    EXPECT_EQ(Alignment::center(), (Alignment{0, 0}));
    EXPECT_EQ(Alignment::topLeft(), (Alignment{-1, -1}));
}

TEST(KollectionAlignment, named_alignments_are_constant_expressions)
{
    static_assert(Alignment::center().x == 0);
    static_assert(Alignment::topLeft().y == -1);
}

TEST(KollectionAlignment, inscribe_top_left_is_the_origin)
{
    EXPECT_EQ(Alignment::topLeft().inscribe({4, 2}, {10, 6}), (Offset{0, 0}));
}

TEST(KollectionAlignment, inscribe_center_splits_the_free_space)
{
    EXPECT_EQ(Alignment::center().inscribe({4, 2}, {10, 6}), (Offset{3, 2}));
}

TEST(KollectionAlignment, inscribe_bottom_right_uses_all_the_free_space)
{
    EXPECT_EQ((Alignment{1, 1}).inscribe({4, 2}, {10, 6}), (Offset{6, 4}));
}

TEST(KollectionAlignment, inscribe_mixes_axes)
{
    EXPECT_EQ((Alignment{1, -1}).inscribe({4, 2}, {10, 6}), (Offset{6, 0}));
}

TEST(KollectionAlignment, inscribe_same_size_is_the_origin)
{
    EXPECT_EQ((Alignment{1, 1}).inscribe({10, 6}, {10, 6}), (Offset{0, 0}));
}

TEST(KollectionAlignment, inscribe_larger_child_overflows_when_centred)
{
    EXPECT_EQ(Alignment::center().inscribe({14, 10}, {10, 6}),
        (Offset{-2, -2}));
}
