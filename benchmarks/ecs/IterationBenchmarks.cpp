/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** IterationBenchmarks
*/

#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <vector>

#include "BenchmarkHelpers.hpp"
#include "engine/ecs/Registry.hpp"

using engine::ecs::Entity;
using engine::ecs::Registry;

// The registry holds one storage per type, so the second set of the join needs
// a type of its own, with T's layout.
template <class T>
struct Second: T {
};

// The same data as BM_Join_SparseSetManual<T>: the same ids, values and
// sizes, so the two benchmarks can be compared case by case.
template <class T>
static void fillJoin(Registry &registry, double density)
{
    registry.registerComponent<T>();
    registry.registerComponent<Second<T>>();
    std::vector<Entity> entities;
    entities.reserve(ENTITY_COUNT);
    for (std::size_t e = 0; e < ENTITY_COUNT; ++e)
        entities.push_back(registry.spawnEntity());
    for (const auto id: pickEntities(density, 42))
        registry.emplaceComponent<T>(entities[id]).value = 1.0F;
    for (const auto id: pickEntities(density, 1337))
        registry.emplaceComponent<Second<T>>(entities[id]).value = 2.0F;
}

template <class T>
static void BM_Join_View(benchmark::State &state) // NOLINT
{
    Registry registry;
    fillJoin<T>(registry, densityOf(state));

    for (auto _: state) {
        float sum = 0.0F;
        for (auto &&[first, second]: registry.view<T, Second<T>>())
            sum += first.value + second.value;
        benchmark::DoNotOptimize(sum);
    }

    state.SetItemsProcessed(state.iterations()
        * static_cast<std::int64_t>(registry.getComponents<T>().size()));
}

BENCHMARK_TEMPLATE(BM_Join_View, Small)
    ->Arg(1)->Arg(10)->Arg(100)->Arg(500)->Arg(1000);
BENCHMARK_TEMPLATE(BM_Join_View, Big)
    ->Arg(1)->Arg(10)->Arg(100)->Arg(500)->Arg(1000);

// The same join, also yielding each entity. The indices are summed so the
// compiler can't drop the entity read.
template <class T>
static void BM_Join_IndexedView(benchmark::State &state) // NOLINT
{
    Registry registry;
    fillJoin<T>(registry, densityOf(state));

    for (auto _: state) {
        float sum = 0.0F;
        std::uint64_t indices = 0;
        for (auto &&[entity, first, second]:
            registry.indexedView<T, Second<T>>()) {
            sum += first.value + second.value;
            indices += entity.index();
        }
        benchmark::DoNotOptimize(sum);
        benchmark::DoNotOptimize(indices);
    }

    state.SetItemsProcessed(state.iterations()
        * static_cast<std::int64_t>(registry.getComponents<T>().size()));
}

BENCHMARK_TEMPLATE(BM_Join_IndexedView, Small)
    ->Arg(1)->Arg(10)->Arg(100)->Arg(500)->Arg(1000);
BENCHMARK_TEMPLATE(BM_Join_IndexedView, Big)
    ->Arg(1)->Arg(10)->Arg(100)->Arg(500)->Arg(1000);
