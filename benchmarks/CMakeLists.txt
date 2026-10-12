option(RTYPE_BUILD_BENCHMARKS "Build ECS benchmarks" OFF)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
	return()
endif()

if(RTYPE_BUILD_BENCHMARKS)
	CPMAddPackage(
		NAME benchmark
		GITHUB_REPOSITORY google/benchmark
		GIT_TAG v1.9.1
		OPTIONS
			"BENCHMARK_ENABLE_TESTING OFF"
			"BENCHMARK_ENABLE_GTEST_TESTS OFF")

	add_executable(rtype_ecs_benchmark ecs/main.cpp)
	target_link_libraries(rtype_ecs_benchmark PRIVATE benchmark::benchmark rtype_engine_ecs)
endif()