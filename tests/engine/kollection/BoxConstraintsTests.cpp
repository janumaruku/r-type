/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** BoxConstraintsTests.cpp
*/

#include <gtest/gtest.h>

#include "kollection/core/BoxConstraints.hpp"

using kollection::core::BoxConstraints;
using kollection::core::EdgeInsets;
using kollection::core::INF;
using kollection::core::Size;

TEST(KollectionBoxConstraints, default_is_unbounded_and_loose)
{
    const BoxConstraints box;

    EXPECT_EQ(box.minWidth, 0);
    EXPECT_EQ(box.maxWidth, INF);
    EXPECT_EQ(box.minHeight, 0);
    EXPECT_EQ(box.maxHeight, INF);
}

TEST(KollectionBoxConstraints, tight_sets_min_and_max_to_the_size)
{
    EXPECT_EQ(BoxConstraints::tight({20, 10}),
        (BoxConstraints{.minWidth = 20, .maxWidth = 20, .minHeight = 10, .maxHeight = 10}));
}

TEST(KollectionBoxConstraints, loose_goes_from_zero_to_the_size)
{
    EXPECT_EQ(BoxConstraints::loose({20, 10}),
        (BoxConstraints{.minWidth = 0, .maxWidth = 20, .minHeight = 0, .maxHeight = 10}));
}

TEST(KollectionBoxConstraints, expand_is_infinite_everywhere)
{
    EXPECT_EQ(BoxConstraints::expand(),
        (BoxConstraints{.minWidth = INF, .maxWidth = INF, .minHeight = INF, .maxHeight = INF}));
}

TEST(KollectionBoxConstraints, is_tight_only_when_both_axes_are_fixed)
{
    EXPECT_TRUE(BoxConstraints::tight({20, 10}).isTight());
    EXPECT_TRUE(BoxConstraints::tight({0, 0}).isTight());
    EXPECT_FALSE(BoxConstraints::loose({20, 10}).isTight());
    EXPECT_FALSE((BoxConstraints{.minWidth = 20, .maxWidth = 20, .minHeight = 0, .maxHeight = 10}).isTight());
    EXPECT_FALSE(BoxConstraints{}.isTight());
}

TEST(KollectionBoxConstraints, bounded_checks_look_at_the_max)
{
    const BoxConstraints loose = BoxConstraints::loose({20, 10});
    const BoxConstraints wideOnly{.maxWidth = INF, .maxHeight = 10};

    EXPECT_TRUE(loose.hasBoundedWidth());
    EXPECT_TRUE(loose.hasBoundedHeight());
    EXPECT_FALSE(wideOnly.hasBoundedWidth());
    EXPECT_TRUE(wideOnly.hasBoundedHeight());
    EXPECT_FALSE(BoxConstraints{}.hasBoundedWidth());
    EXPECT_FALSE(BoxConstraints{}.hasBoundedHeight());
}

TEST(KollectionBoxConstraints, constrain_keeps_a_size_already_in_range)
{
    const BoxConstraints box{.minWidth = 10, .maxWidth = 100, .minHeight = 5, .maxHeight = 50};

    EXPECT_EQ(box.constrain({40, 20}), (Size{40, 20}));
    EXPECT_EQ(box.constrain({10, 50}), (Size{10, 50}));
}

TEST(KollectionBoxConstraints, constrain_raises_a_size_below_the_min)
{
    const BoxConstraints box{.minWidth = 10, .maxWidth = 100, .minHeight = 5, .maxHeight = 50};

    EXPECT_EQ(box.constrain({0, 0}), (Size{10, 5}));
}

TEST(KollectionBoxConstraints, constrain_lowers_a_size_above_the_max)
{
    const BoxConstraints box{.minWidth = 10, .maxWidth = 100, .minHeight = 5, .maxHeight = 50};

    EXPECT_EQ(box.constrain({500, 500}), (Size{100, 50}));
}

TEST(KollectionBoxConstraints, constrain_clamps_each_axis_on_its_own)
{
    const BoxConstraints box{.minWidth = 10, .maxWidth = 100, .minHeight = 5, .maxHeight = 50};

    EXPECT_EQ(box.constrain({0, 500}), (Size{10, 50}));
}

TEST(KollectionBoxConstraints, constrain_with_tight_always_returns_the_size)
{
    const BoxConstraints box = BoxConstraints::tight({20, 10});

    EXPECT_EQ(box.constrain({0, 0}), (Size{20, 10}));
    EXPECT_EQ(box.constrain({999, 999}), (Size{20, 10}));
}

TEST(KollectionBoxConstraints, constrain_with_unbounded_keeps_large_sizes)
{
    EXPECT_EQ(BoxConstraints{}.constrain({1e6, 1e6}), (Size{1e6, 1e6}));
}

TEST(KollectionBoxConstraints, smallest_is_the_min)
{
    EXPECT_EQ(BoxConstraints::tight({20, 10}).smallest(), (Size{20, 10}));
    EXPECT_EQ(BoxConstraints::loose({20, 10}).smallest(), (Size{0, 0}));
    EXPECT_EQ((BoxConstraints{.minWidth = 3, .maxWidth = 9, .minHeight = 4, .maxHeight = 8}).smallest(),
        (Size{3, 4}));
}

TEST(KollectionBoxConstraints, loosen_drops_the_min_and_keeps_the_max)
{
    EXPECT_EQ(BoxConstraints::tight({20, 10}).loosen(), BoxConstraints::loose({20, 10}));
    EXPECT_EQ(BoxConstraints{}.loosen(), BoxConstraints{});
}

TEST(KollectionBoxConstraints, deflate_removes_the_insets_from_both_bounds)
{
    const BoxConstraints box{.minWidth = 10, .maxWidth = 100, .minHeight = 10, .maxHeight = 50};

    EXPECT_EQ(box.deflate(EdgeInsets::symmetric(2, 1)),
        (BoxConstraints{.minWidth = 6, .maxWidth = 96, .minHeight = 8, .maxHeight = 48}));
}

TEST(KollectionBoxConstraints, deflate_loose_by_one_on_each_side)
{
    EXPECT_EQ(BoxConstraints::loose({20, 5}).deflate(EdgeInsets::all(1)), BoxConstraints::loose({18, 3}));
}

TEST(KollectionBoxConstraints, deflate_tight_stays_tight)
{
    EXPECT_EQ(BoxConstraints::tight({20, 10}).deflate(EdgeInsets::all(2)), BoxConstraints::tight({16, 6}));
}

TEST(KollectionBoxConstraints, deflate_never_goes_below_zero)
{
    EXPECT_EQ(BoxConstraints::tight({20, 20}).deflate(EdgeInsets::all(15)), BoxConstraints::tight({0, 0}));
    EXPECT_EQ(BoxConstraints::loose({4, 4}).deflate(EdgeInsets::all(10)), BoxConstraints::tight({0, 0}));
}

TEST(KollectionBoxConstraints, deflate_keeps_min_under_max)
{
    const BoxConstraints box{.minWidth = 50, .maxWidth = 60, .minHeight = 50, .maxHeight = 60};
    const BoxConstraints deflated = box.deflate(EdgeInsets::all(40));

    EXPECT_LE(deflated.minWidth, deflated.maxWidth);
    EXPECT_LE(deflated.minHeight, deflated.maxHeight);
    EXPECT_EQ(deflated, BoxConstraints::tight({0, 0}));
}

TEST(KollectionBoxConstraints, deflate_keeps_unbounded_unbounded)
{
    const BoxConstraints deflated = BoxConstraints{}.deflate(EdgeInsets::all(10));

    EXPECT_EQ(deflated.maxWidth, INF);
    EXPECT_EQ(deflated.maxHeight, INF);
    EXPECT_EQ(deflated.minWidth, 0);
    EXPECT_EQ(deflated.minHeight, 0);
}

TEST(KollectionBoxConstraints, deflate_by_nothing_changes_nothing)
{
    const BoxConstraints box{.minWidth = 10, .maxWidth = 100, .minHeight = 5, .maxHeight = 50};

    EXPECT_EQ(box.deflate(EdgeInsets{}), box);
}

TEST(KollectionBoxConstraints, enforce_keeps_constraints_already_inside_the_parent)
{
    const BoxConstraints parent = BoxConstraints::loose({100, 100});
    const BoxConstraints child{.minWidth = 10, .maxWidth = 50, .minHeight = 20, .maxHeight = 60};

    EXPECT_EQ(child.enforce(parent), child);
}

TEST(KollectionBoxConstraints, enforce_lowers_a_child_bigger_than_the_parent)
{
    EXPECT_EQ(BoxConstraints::tight({200, 200}).enforce(BoxConstraints::loose({150, 150})),
        BoxConstraints::tight({150, 150}));
}

TEST(KollectionBoxConstraints, enforce_raises_a_child_smaller_than_the_parent)
{
    const BoxConstraints parent{.minWidth = 100, .maxWidth = 300, .minHeight = 100, .maxHeight = 300};

    EXPECT_EQ(BoxConstraints::tight({50, 50}).enforce(parent), BoxConstraints::tight({100, 100}));
}

TEST(KollectionBoxConstraints, enforce_expand_fills_the_parent)
{
    EXPECT_EQ(BoxConstraints::expand().enforce(BoxConstraints::loose({100, 50})), BoxConstraints::tight({100, 50}));
}

TEST(KollectionBoxConstraints, enforce_with_a_tight_parent_is_that_parent)
{
    const BoxConstraints parent = BoxConstraints::tight({80, 40});

    EXPECT_EQ(BoxConstraints{}.enforce(parent), parent);
    EXPECT_EQ(BoxConstraints::tight({10, 10}).enforce(parent), parent);
}

TEST(KollectionBoxConstraints, enforce_keeps_min_under_max)
{
    const BoxConstraints parent{.minWidth = 40, .maxWidth = 60, .minHeight = 40, .maxHeight = 60};
    const BoxConstraints child{.minWidth = 0, .maxWidth = 500, .minHeight = 55, .maxHeight = 58};
    const BoxConstraints enforced = child.enforce(parent);

    EXPECT_LE(enforced.minWidth, enforced.maxWidth);
    EXPECT_LE(enforced.minHeight, enforced.maxHeight);
    EXPECT_EQ(enforced, (BoxConstraints{.minWidth = 40, .maxWidth = 60, .minHeight = 55, .maxHeight = 58}));
}
