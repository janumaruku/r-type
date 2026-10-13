/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SetupBenchmarks
*/

#include <benchmark/benchmark.h>

#include <string>

static void BM_Setup(benchmark::State &state)
{
    for (auto _ : state) {
        std::string temp = "a very, very long string";
        benchmark::DoNotOptimize(temp);
    }
}
BENCHMARK(BM_Setup);
