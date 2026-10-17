/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** StorageBenchmarks
*/

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>

#include "BenchmarkHelpers.hpp"
#include "engine/ecs/SparseSet.hpp"

using engine::ecs::Entity;
using engine::ecs::SparseSet;
using engine::ecs::detail::EntityAccess;

template <class T>
static void BM_SparseSet_Iterate(benchmark::State &state) // NOLINT
{
    SparseSet<T> set;
    const auto ids = pickEntities(densityOf(state));
    for (const auto id : ids)
        set.emplaceAt(EntityAccess::make(static_cast<std::uint32_t>(id), 0))
            .value = 1.0F;

    for (auto _ : state) {
        float sum = 0.0F;
        for (const T &component : set.components())
            sum += component.value;
        benchmark::DoNotOptimize(sum);
    }

    state.SetItemsProcessed(state.iterations()
        * static_cast<std::int64_t>(set.size()));
    state.counters["MB"] = memoryMB<T>(ids);
}
BENCHMARK_TEMPLATE(BM_SparseSet_Iterate, Small)
    ->Arg(1)->Arg(10)->Arg(100)->Arg(500)->Arg(1000);
BENCHMARK_TEMPLATE(BM_SparseSet_Iterate, Big)
    ->Arg(1)->Arg(10)->Arg(100)->Arg(500)->Arg(1000);

// Half the handles are stale (generation 1), chosen at random so the branch
// predictor can't learn the pattern in either order.
static void runContains(benchmark::State &state, bool shuffled)
{
    SparseSet<Small> set;
    const auto ids = pickEntities(densityOf(state));
    std::mt19937 rng(7);
    std::bernoulli_distribution stale(0.5);
    std::vector<Entity> handles;
    handles.reserve(ids.size());
    for (const auto id : ids) {
        const auto index = static_cast<std::uint32_t>(id);
        set.emplaceAt(EntityAccess::make(index, 0));
        handles.push_back(EntityAccess::make(index, stale(rng) ? 1 : 0));
    }
    if (shuffled)
        std::ranges::shuffle(handles, rng);

    for (auto _ : state) {
        std::size_t found = 0;
        for (const Entity &handle : handles)
            found += set.contains(handle) ? 1 : 0;
        benchmark::DoNotOptimize(found);
    }

    state.SetItemsProcessed(state.iterations()
        * static_cast<std::int64_t>(handles.size()));
}

static void BM_SparseSet_Contains(benchmark::State &state) // NOLINT
{
    runContains(state, true);
}
BENCHMARK(BM_SparseSet_Contains)
    ->Arg(1)->Arg(10)->Arg(100)->Arg(500)->Arg(1000);

static void BM_SparseSet_ContainsSorted(benchmark::State &state) // NOLINT
{
    runContains(state, false);
}
BENCHMARK(BM_SparseSet_ContainsSorted)
    ->Arg(1)->Arg(10)->Arg(100)->Arg(500)->Arg(1000);

// A bullet's life in steady state: each iteration kills the oldest of the N
// live entities and spawns a new one in its index with the next generation,
// as the registry will. The oldest is rarely last in storage, so the erase
// also exercises the swap-and-pop move.
static void BM_SparseSet_InsertErase(benchmark::State &state) // NOLINT
{
    const auto count = static_cast<std::size_t>(state.range(0));
    SparseSet<Small> set;
    std::vector<Entity> live;
    live.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        live.push_back(EntityAccess::make(static_cast<std::uint32_t>(i), 0));
        set.emplaceAt(live.back());
    }

    std::size_t oldest = 0;
    for (auto _ : state) {
        const Entity dead = live[oldest];
        set.erase(dead);
        live[oldest] = EntityAccess::make(dead.index(), dead.generation() + 1);
        set.emplaceAt(live[oldest]);
        oldest = (oldest + 1) % count;
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_SparseSet_InsertErase)->Arg(1000)->Arg(10000)->Arg(100000);

// The hand-written two-set join that View must stay within 10% of: walk the
// first set and look each entity up in the second. The second set owns other
// entities at the same density, so only part of the lookups succeed.
template <class T>
static void BM_Join_SparseSetManual(benchmark::State &state) // NOLINT
{
    SparseSet<T> first;
    SparseSet<T> second;
    const auto firstIds = pickEntities(densityOf(state), 42);
    const auto secondIds = pickEntities(densityOf(state), 1337);
    for (const auto id : firstIds)
        first.emplaceAt(EntityAccess::make(static_cast<std::uint32_t>(id), 0))
            .value = 1.0F;
    for (const auto id : secondIds)
        second.emplaceAt(EntityAccess::make(static_cast<std::uint32_t>(id), 0))
            .value = 2.0F;

    for (auto _ : state) {
        const auto entities = first.entities();
        const auto components = first.components();
        float sum = 0.0F;
        for (std::size_t i = 0; i < entities.size(); ++i)
            if (const T *other = second.tryGet(entities[i]))
                sum += components[i].value + other->value;
        benchmark::DoNotOptimize(sum);
    }

    state.SetItemsProcessed(state.iterations()
        * static_cast<std::int64_t>(first.size()));
    state.counters["MB"] = memoryMB<T>(firstIds) + memoryMB<T>(secondIds);
}
BENCHMARK_TEMPLATE(BM_Join_SparseSetManual, Small)
    ->Arg(1)->Arg(10)->Arg(100)->Arg(500)->Arg(1000);
BENCHMARK_TEMPLATE(BM_Join_SparseSetManual, Big)
    ->Arg(1)->Arg(10)->Arg(100)->Arg(500)->Arg(1000);
