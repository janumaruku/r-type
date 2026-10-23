/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Vec2Tests.cpp
*/

#include <gtest/gtest.h>
#include "../../../engine/core/include/engine/core/Vec2.hpp"
#include <cmath>

TEST(Vec2, constuct)
{
    engine::core::Vec2f v;
    (void)v;
    SUCCEED();
}

TEST(Vec2, constuct_params)
{
    engine::core::Vec2f v{1, 2};
    SUCCEED();
}

TEST(Vec2, getX)
{
    engine::core::Vec2f v{1, 2};
    EXPECT_EQ(v.x(), 1);
}

TEST(Vec2, getY)
{
    engine::core::Vec2f v{1, 2};
    EXPECT_EQ(v.y(), 2);
}

TEST(Vec2, add)
{
    engine::core::Vec2f v1{1, 2};
    engine::core::Vec2f v2{1, 2};
    engine::core::Vec2f v3 = v1 + v2;
    
    auto x = v3.x();
    auto y = v3.y();

    EXPECT_FLOAT_EQ(x, 2);
    EXPECT_FLOAT_EQ(y, 4);

}

TEST(Vec2, sub)
{
    engine::core::Vec2f v1{1, 2};
    engine::core::Vec2f v2{1, 2};
    engine::core::Vec2f v3 = v1 - v2;
    
    auto x = v3.x();
    auto y = v3.y();

    EXPECT_FLOAT_EQ(x, 0);
    EXPECT_FLOAT_EQ(y, 0);

}

TEST(Vec2, mul)
{
    engine::core::Vec2f v1{1, 2};
    engine::core::Vec2f v3 = v1 * 2;

    EXPECT_FLOAT_EQ(v3.x(), 2);
    EXPECT_FLOAT_EQ(v3.y(), 4);
}


TEST(Vec2, div)
{
    engine::core::Vec2f v1{1, 2};
    engine::core::Vec2f v3 = v1 / 2;

    EXPECT_FLOAT_EQ(v3.x(), .5);
    EXPECT_FLOAT_EQ(v3.y(), 1);
}

TEST(Vec2, selfadd)
{
    engine::core::Vec2f v1{1, 2};
    engine::core::Vec2f v3{2, 3};

    v1 += v3;
    EXPECT_FLOAT_EQ(v1.x(), 3);
    EXPECT_FLOAT_EQ(v1.y(), 5);
}

TEST(Vec2, selfsub)
{
    engine::core::Vec2f v1{1, 2};
    engine::core::Vec2f v3{2, 3};

    v1 -= v3;
    EXPECT_FLOAT_EQ(v1.x(), -1);
    EXPECT_FLOAT_EQ(v1.y(), -1);
}

TEST(Vec2, eq)
{
    engine::core::Vec2f v1{1, 2};
    engine::core::Vec2f v3{1, 2};

    EXPECT_TRUE(v1 == v3);
}

TEST(Vec2, neq)
{
    engine::core::Vec2f v1{55, 2};
    engine::core::Vec2f v3{1, 2};

    EXPECT_TRUE(v1 != v3);
}

TEST(Vec2, length)
{
    engine::core::Vec2f v1{0, 7};
    EXPECT_FLOAT_EQ(v1.length(), 7);
}

TEST(Vec2, normalized)
{
    engine::core::Vec2f v1{20, 7};
    const auto actual = v1.normalized();

    const engine::core::Vec2f expected{
        20.0F / std::sqrt(449.0F),
        7.0F / std::sqrt(449.0F)
    };

    EXPECT_FLOAT_EQ(actual.x(), expected.x());
    EXPECT_FLOAT_EQ(actual.y(), expected.y());
}