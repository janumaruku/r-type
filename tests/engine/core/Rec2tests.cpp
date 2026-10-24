/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Rec2tests.cpp
*/

#include <gtest/gtest.h>
#include "engine/core/Rect2.hpp"

TEST(Rect2, constuct)
{
    engine::core::FloatRect r;
    (void)r;
    SUCCEED();
}

TEST(Rect2, constuct_params)
{
    engine::core::FloatRect r{engine::core::Vec2f{1, 2}, 3, 4};
    SUCCEED();
}

TEST(Rect2, getOrigin)
{
    engine::core::FloatRect r{engine::core::Vec2f{1, 2}, 3, 4};
    EXPECT_FLOAT_EQ(r.origin().x(), 1);
    EXPECT_FLOAT_EQ(r.origin().y(), 2);
}

TEST(Rect2, getSize)
{
    engine::core::FloatRect r{engine::core::Vec2f{1, 2}, 3, 4};
    EXPECT_FLOAT_EQ(r.width(), 3);
    EXPECT_FLOAT_EQ(r.height(), 4);
}

TEST(Rect2, contains)
{
    engine::core::FloatRect r{engine::core::Vec2f{1, 2}, 3, 4};
    EXPECT_TRUE(r.contains(engine::core::Vec2f{2, 3}));
    EXPECT_FALSE(r.contains(engine::core::Vec2f{0, 0}));
}

TEST(Rect2, intersects)
{
    engine::core::FloatRect r1{engine::core::Vec2f{1, 2}, 3, 4};
    engine::core::FloatRect r2{engine::core::Vec2f{2, 3}, 3, 4};
    engine::core::FloatRect r3{engine::core::Vec2f{5, 6}, 3, 4};

    EXPECT_TRUE(r1.intersects(r2));
    EXPECT_FALSE(r1.intersects(r3));
}

TEST(Rect2, center)
{
    engine::core::FloatRect r{engine::core::Vec2f{1, 2}, 3, 4};
    engine::core::Vec2f center = r.center();

    EXPECT_FLOAT_EQ(center.x(), 2.5);
    EXPECT_FLOAT_EQ(center.y(), 4);
}

TEST(Rect2, fromCenter)
{
    engine::core::Vec2f center{2.5, 4};
    engine::core::Vec2f halfsize{1.5, 2};
    engine::core::FloatRect r = engine::core::FloatRect::fromCenter(center, halfsize);

    EXPECT_FLOAT_EQ(r.origin().x(), 1);
    EXPECT_FLOAT_EQ(r.origin().y(), 2);
    EXPECT_FLOAT_EQ(r.width(), 3);
    EXPECT_FLOAT_EQ(r.height(), 4);
}