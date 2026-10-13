/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** BenchmarksHelpers
*/

#pragma once

#include <cstddef>
#include <random>
#include <vector>
#include <benchmark/benchmark.h>

inline constexpr std::size_t ENTITY_COUNT = 1'000'000;

struct Position { float x; float y; };
struct Small { float value; float pad; };       // 8 bytes
struct Big { float value; float pad[15]; };     // 64 bytes
static_assert(sizeof(Small) == 8);
static_assert(sizeof(Big) == 64);
template <std::size_t N> struct Tag {};         // as many distinct types as needed

inline std::vector<std::size_t> pickEntities(double density)
{
    std::mt19937 rng(42);
    std::bernoulli_distribution owns(density);
    std::vector<std::size_t> ids;

    for (std::size_t e = 0; e < ENTITY_COUNT; ++e)
        if (owns(rng))
            ids.push_back(e);
    return ids;
}

inline double densityOf(const benchmark::State &state)
{
    return static_cast<double>(state.range(0)) / 1000.0;
}
