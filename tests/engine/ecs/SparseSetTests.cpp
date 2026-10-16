/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SparseSetTests
*/

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>

#include "engine/ecs/SparseSet.hpp"

using engine::ecs::Entity;
using engine::ecs::SparseSet;
using engine::ecs::detail::EntityAccess;

namespace {
Entity entity(std::uint32_t index, std::uint32_t generation = 0)
{
    return EntityAccess::make(index, generation);
}

struct Counted {
    static inline int copies = 0;
    static inline int moves = 0;
    int value = 0;

    Counted() = default;

    explicit Counted(int initial): value(initial)
    {
    }

    Counted(const Counted &other): value(other.value)
    {
        ++copies;
    }

    Counted(Counted &&other) noexcept: value(other.value)
    {
        ++moves;
    }

    Counted &operator=(const Counted &other)
    {
        value = other.value;
        ++copies;
        return *this;
    }

    Counted &operator=(Counted &&other) noexcept
    {
        value = other.value;
        ++moves;
        return *this;
    }

    static void reset()
    {
        copies = 0;
        moves = 0;
    }
};

struct Fragile {
    static inline bool failNext = false;
    int value;

    explicit Fragile(int initial): value(initial)
    {
        if (failNext)
            throw std::runtime_error("Fragile: construction failed on demand");
    }
};

template <class C>
testing::AssertionResult aligned(SparseSet<C> &set)
{
    const auto entities = set.entities();
    const auto components = set.components();

    if (entities.size() != components.size())
        return testing::AssertionFailure()
            << "entities and components differ in size";
    for (std::size_t i = 0; i < entities.size(); ++i)
        if (set.tryGet(entities[i]) != &components[i])
            return testing::AssertionFailure()
                << "position " << i << " is misaligned";
    return testing::AssertionSuccess();
}
} // namespace

TEST(SparseSet, InsertingIndexThousandHoldsOneComponent)
{
    SparseSet<int> set;

    set.insertAt(entity(1000), 7);

    EXPECT_EQ(set.size(), 1U);
    EXPECT_EQ(set.get(entity(1000)), 7);
}

TEST(SparseSet, InsertingAgainReplacesTheValueAndKeepsTheSize)
{
    SparseSet<int> set;
    set.insertAt(entity(3), 1);

    int &replaced = set.insertAt(entity(3), 2);

    EXPECT_EQ(set.size(), 1U);
    EXPECT_EQ(replaced, 2);
    EXPECT_EQ(&replaced, set.tryGet(entity(3)));
}

TEST(SparseSet, EmplaceOnAnExistingEntityMakesNoCopy)
{
    SparseSet<Counted> set;
    set.emplaceAt(entity(0), 1);
    Counted::reset();

    set.emplaceAt(entity(0), 2);

    EXPECT_EQ(Counted::copies, 0);
    EXPECT_EQ(set.get(entity(0)).value, 2);
}

TEST(SparseSet, InsertingAnLvalueCopiesIt)
{
    SparseSet<Counted> set;
    const Counted original(5);
    Counted::reset();

    set.insertAt(entity(0), original);

    EXPECT_EQ(Counted::copies, 1);
}

TEST(SparseSet, InsertingAnRvalueNeverCopies)
{
    SparseSet<Counted> set;
    Counted::reset();

    set.insertAt(entity(0), Counted(5));
    set.insertAt(entity(0), Counted(6));

    EXPECT_EQ(Counted::copies, 0);
}

TEST(SparseSet, AcceptsAMoveOnlyComponent)
{
    SparseSet<std::unique_ptr<int>> set;

    set.insertAt(entity(0), std::make_unique<int>(42));

    ASSERT_NE(set.get(entity(0)), nullptr);
    EXPECT_EQ(*set.get(entity(0)), 42);
}

TEST(SparseSet, TryGetReturnsNullForAbsentAndStaleEntities)
{
    SparseSet<int> set;
    set.insertAt(entity(2, 0), 1);

    EXPECT_EQ(set.tryGet(entity(5)), nullptr);
    EXPECT_EQ(set.tryGet(entity(2, 1)), nullptr);
    EXPECT_NE(set.tryGet(entity(2, 0)), nullptr);
}

TEST(SparseSet, ComponentsAreAlignedWithEntities)
{
    SparseSet<int> set;
    for (std::uint32_t index = 0; index < 5; ++index)
        set.insertAt(entity(index * 3), static_cast<int>(index));
    set.insertAt(entity(6), 99);

    EXPECT_TRUE(aligned(set));
    EXPECT_EQ(set.get(entity(6)), 99);
}

TEST(SparseSet, AFailedAppendLeavesTheSetUnchanged)
{
    SparseSet<Fragile> set;
    Fragile::failNext = false;
    set.emplaceAt(entity(0), 1);
    set.emplaceAt(entity(1), 2);

    Fragile::failNext = true;
    EXPECT_THROW(set.emplaceAt(entity(2), 3), std::runtime_error);
    Fragile::failNext = false;

    EXPECT_EQ(set.size(), 2U);
    EXPECT_FALSE(set.contains(entity(2)));
    EXPECT_TRUE(aligned(set));
}

TEST(SparseSet, AFailedReplacementKeepsTheOldValue)
{
    SparseSet<Fragile> set;
    Fragile::failNext = false;
    set.emplaceAt(entity(0), 1);

    Fragile::failNext = true;
    EXPECT_THROW(set.emplaceAt(entity(0), 2), std::runtime_error);
    Fragile::failNext = false;

    EXPECT_EQ(set.get(entity(0)).value, 1);
    EXPECT_TRUE(aligned(set));
}

TEST(SparseSet, InsertingFromAComponentOfTheSameSetIsSafe)
{
    SparseSet<std::string> set;
    const std::string longText(100, 'x');
    set.insertAt(entity(0), longText);

    set.insertAt(entity(1), set.get(entity(0)));

    EXPECT_EQ(set.get(entity(1)), longText);
    EXPECT_EQ(set.get(entity(0)), longText);
}
