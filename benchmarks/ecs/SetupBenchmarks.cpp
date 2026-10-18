/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SetupBenchmarks
*/

#include <benchmark/benchmark.h>

#include "BenchmarkHelpers.hpp"

static void BM_Setup(benchmark::State &state)
{
    for (auto _ : state) {
        benchmark::DoNotOptimize(pickEntities(densityOf(state)));
    }
}
BENCHMARK(BM_Setup)->Arg(100);
