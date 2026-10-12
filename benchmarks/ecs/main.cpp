#include <benchmark/benchmark.h>

static void BM_EcsPlaceholder(benchmark::State& state)
{
	for (auto state_iteration : state) {
		(void)state_iteration;
		benchmark::DoNotOptimize(state.iterations());
	}
}

BENCHMARK(BM_EcsPlaceholder);
BENCHMARK_MAIN();