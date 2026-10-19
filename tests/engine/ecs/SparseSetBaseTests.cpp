/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SparseSetBaseTests
*/

#include <gtest/gtest.h>

#include <cstdint>
#include <type_traits>
#include <vector>

#include "engine/ecs/SparseSetBase.hpp"

using engine::ecs::Entity;
using engine::ecs::SparseSetBase;
using engine::ecs::detail::EntityAccess;

namespace {
class TestSet : public SparseSetBase {
public:
    using SparseSetBase::append;
    using SparseSetBase::positionOf;
    using SparseSetBase::swapAndPop;

    void erase(const Entity &entity) override
    {
        if (contains(entity))
            swapAndPop(entity);
    }

    [[nodiscard]] std::size_t sparseSize() const noexcept
    {
        return _sparse.size();
    }

    [[nodiscard]] testing::AssertionResult invariantsHold() const
    {
        for (std::size_t position = 0; position < _dense.size(); ++position)
            if (_sparse.at(_dense[position].index()) != position)
                return testing::AssertionFailure()
                    << "_sparse doesn't point back to position " << position;
        for (std::size_t index = 0; index < _sparse.size(); ++index)
            if (_sparse[index] != NPOS
                && _dense.at(_sparse[index]).index() != index)
                return testing::AssertionFailure()
                    << "_sparse[" << index << "] points at another entity";
        return testing::AssertionSuccess();
    }
};

Entity entity(std::uint32_t index, std::uint32_t generation = 0)
{
    return EntityAccess::make(index, generation);
}

std::vector<std::uint32_t> indicesOf(const TestSet &set)
{
    std::vector<std::uint32_t> indices;
    for (const auto &held : set.entities())
        indices.push_back(held.index());
    return indices;
}
} // namespace

static_assert(!std::is_copy_constructible_v<SparseSetBase>);
static_assert(!std::is_copy_assignable_v<SparseSetBase>);
static_assert(!std::is_move_constructible_v<SparseSetBase>);
static_assert(!std::is_move_assignable_v<SparseSetBase>);

TEST(SparseSetBase, AppendingIndexFiveGrowsSparseToSix)
{
    TestSet set;

    set.append(entity(5));

    EXPECT_EQ(set.sparseSize(), 6U);
    EXPECT_EQ(set.size(), 1U);
    EXPECT_TRUE(set.contains(entity(5)));
    EXPECT_FALSE(set.contains(entity(3)));
    EXPECT_TRUE(set.invariantsHold());
}

TEST(SparseSetBase, ContainsRejectsAnotherGeneration)
{
    TestSet set;
    set.append(entity(2, 0));

    EXPECT_FALSE(set.contains(entity(2, 1)));
    EXPECT_TRUE(set.contains(entity(2, 0)));
}

TEST(SparseSetBase, SwapAndPopOfAMiddleElementMovesTheLastIntoItsPlace)
{
    TestSet set;
    set.append(entity(7));
    set.append(entity(3));
    set.append(entity(1));

    set.swapAndPop(entity(3));

    EXPECT_EQ(indicesOf(set), (std::vector<std::uint32_t>{7, 1}));
    EXPECT_EQ(set.positionOf(entity(1)), 1U);
    EXPECT_FALSE(set.contains(entity(3)));
    EXPECT_TRUE(set.invariantsHold());
}

TEST(SparseSetBase, SwapAndPopOfTheLastElement)
{
    TestSet set;
    set.append(entity(0));
    set.append(entity(5));

    set.swapAndPop(entity(5));

    EXPECT_EQ(indicesOf(set), (std::vector<std::uint32_t>{0}));
    EXPECT_FALSE(set.contains(entity(5)));
    EXPECT_TRUE(set.invariantsHold());
}

TEST(SparseSetBase, SwapAndPopOfTheOnlyElement)
{
    TestSet set;
    set.append(entity(4));

    set.swapAndPop(entity(4));

    EXPECT_TRUE(set.empty());
    EXPECT_FALSE(set.contains(entity(4)));
    EXPECT_TRUE(set.invariantsHold());
}

TEST(SparseSetBase, EntitiesReflectTheOrderAfterSwaps)
{
    TestSet set;
    for (std::uint32_t index = 0; index < 5; ++index)
        set.append(entity(index));

    set.swapAndPop(entity(0));
    EXPECT_TRUE(set.invariantsHold());
    set.swapAndPop(entity(2));
    EXPECT_TRUE(set.invariantsHold());

    EXPECT_EQ(indicesOf(set), (std::vector<std::uint32_t>{4, 1, 3}));
}

TEST(SparseSetBase, ErasingAnAbsentOrStaleEntityDoesNothing)
{
    TestSet set;
    set.append(entity(1, 0));

    set.erase(entity(1, 1));
    set.erase(entity(9));

    EXPECT_EQ(set.size(), 1U);
    EXPECT_TRUE(set.contains(entity(1, 0)));
    EXPECT_EQ(set.sparseSize(), 2U);
    EXPECT_TRUE(set.invariantsHold());
}
