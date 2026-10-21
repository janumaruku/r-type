/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RegistryBenchmarks
*/

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <random>
#include <utility>
#include <vector>

#include "BenchmarkHelpers.hpp"
#include "engine/ecs/Registry.hpp"

using engine::ecs::Entity;
using engine::ecs::Registry;

static void BM_Registry_GetComponents(benchmark::State &state) // NOLINT
{
    Registry registry;
    [&registry]<std::size_t... I>(std::index_sequence<I...>) {
        (registry.registerComponent<Tag<I>>(), ...);
    }(std::make_index_sequence<20>{});
    registry.registerComponent<Position>();

    for (auto _: state)
        benchmark::DoNotOptimize(&registry.getComponents<Position>());
}

BENCHMARK(BM_Registry_GetComponents);

// Half the handles are stale: their entity was killed and its index reused
// with the next generation. Shuffled so the branch predictor can't learn the
// pattern and the reads land at random places in the registry's vectors.
static void BM_Registry_IsAlive(benchmark::State &state) // NOLINT
{
    const auto count = static_cast<std::size_t>(state.range(0));
    Registry registry;
    std::mt19937 rng(7);
    std::bernoulli_distribution stale(0.5);
    std::vector<Entity> handles;
    std::vector<Entity> toKill;
    handles.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        const Entity entity = registry.spawnEntity();
        if (stale(rng))
            toKill.push_back(entity);
        else
            handles.push_back(entity);
    }
    for (const Entity &entity: toKill)
        registry.killEntity(entity);
    for (std::size_t i = 0; i < toKill.size(); ++i)
        registry.spawnEntity();
    handles.insert(handles.end(), toKill.begin(), toKill.end());
    std::ranges::shuffle(handles, rng);

    for (auto _: state) {
        std::size_t alive = 0;
        for (const Entity &handle: handles)
            alive += registry.isAlive(handle) ? 1 : 0;
        benchmark::DoNotOptimize(alive);
    }

    state.SetItemsProcessed(state.iterations()
        * static_cast<std::int64_t>(handles.size()));
}

BENCHMARK(BM_Registry_IsAlive)->Arg(1000)->Arg(100000)->Arg(1000000);
