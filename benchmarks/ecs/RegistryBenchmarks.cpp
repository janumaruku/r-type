/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RegistryBenchmarks
*/

#include <benchmark/benchmark.h>

#include <cstddef>
#include <utility>

#include "BenchmarkHelpers.hpp"
#include "engine/ecs/Registry.hpp"

using engine::ecs::Registry;

static void BM_Registry_GetComponents(benchmark::State &state) // NOLINT
{
    Registry registry;
    [&registry]<std::size_t... I>(std::index_sequence<I...>) {
        (registry.registerComponent<Tag<I>>(), ...);
    }(std::make_index_sequence<20>{});
    registry.registerComponent<Position>();

    for (auto _ : state)
        benchmark::DoNotOptimize(&registry.getComponents<Position>());
}
BENCHMARK(BM_Registry_GetComponents);
