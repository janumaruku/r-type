add_executable(ecs_benchmarks SetupBenchmarks.cpp)

target_include_directories(ecs_benchmarks PRIVATE .)

target_link_libraries(ecs_benchmarks PRIVATE
        engine_ecs
        benchmark::benchmark_main
        project_options)