/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** TypeIdTests
*/

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <latch>
#include <thread>

#include "engine/ecs/TypeId.hpp"

using engine::ecs::detail::nextTypeId;
using engine::ecs::detail::typeId;

namespace {
struct First {};
struct Second {};
struct Third {};
struct Repeated {};
struct SharedByThreads {};

template <class T>
concept HasTypeId = requires { typeId<T>(); };
} // namespace

static_assert(HasTypeId<int>);
static_assert(HasTypeId<First>);
static_assert(!HasTypeId<const int>);
static_assert(!HasTypeId<volatile int>);
static_assert(!HasTypeId<int &>);
static_assert(!HasTypeId<const int &>);
static_assert(!HasTypeId<int &&>);

TEST(TypeId, SameTypeGivesSameId)
{
    const std::size_t first = typeId<Repeated>();

    EXPECT_EQ(typeId<Repeated>(), first);
    EXPECT_EQ(typeId<Repeated>(), first);
}

TEST(TypeId, DifferentTypesGiveDifferentIds)
{
    EXPECT_NE(typeId<First>(), typeId<Second>());
    EXPECT_NE(typeId<Second>(), typeId<Third>());
    EXPECT_NE(typeId<First>(), typeId<Third>());
}

TEST(TypeId, NextTypeIdCountsUpByOne)
{
    const std::size_t first = nextTypeId();

    EXPECT_EQ(nextTypeId(), first + 1);
    EXPECT_EQ(nextTypeId(), first + 2);
}

TEST(TypeId, IdsAreBelowTheNumberOfIdsHandedOut)
{
    const std::size_t first = typeId<First>();
    const std::size_t second = typeId<Second>();
    const std::size_t third = typeId<Third>();
    const std::size_t handedOut = nextTypeId();

    EXPECT_LT(first, handedOut);
    EXPECT_LT(second, handedOut);
    EXPECT_LT(third, handedOut);
}

TEST(TypeId, ThreadsAskingAtTheSameTimeGetTheSameId)
{
    constexpr std::size_t THREAD_COUNT = 8;
    std::array<std::size_t, THREAD_COUNT> ids{};
    std::latch start{1};

    {
        std::array<std::jthread, THREAD_COUNT> threads;

        for (std::size_t i = 0; i < THREAD_COUNT; ++i)
            threads[i] = std::jthread([&ids, &start, i] {
                start.wait();
                ids[i] = typeId<SharedByThreads>();
            });
        start.count_down();
    }

    for (const std::size_t id : ids)
        EXPECT_EQ(id, ids[0]);
}
