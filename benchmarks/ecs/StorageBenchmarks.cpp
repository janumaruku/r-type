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
