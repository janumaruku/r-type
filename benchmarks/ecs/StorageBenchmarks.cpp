/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** StorageBenchmarks
*/

#include <benchmark/benchmark.h>

#include <cstdint>

#include "BenchmarkHelpers.hpp"
#include "engine/ecs/SparseSet.hpp"

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
