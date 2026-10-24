cmake_minimum_required(VERSION 3.28.3...4.0)

project(r-type VERSION 1.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (UNIX AND NOT APPLE)
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libstdc++.so.6
        OUTPUT_VARIABLE CXX_RUNTIME_LIBRARY
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (IS_ABSOLUTE "${CXX_RUNTIME_LIBRARY}")
        get_filename_component(CXX_RUNTIME_DIRECTORY "${CXX_RUNTIME_LIBRARY}" DIRECTORY)
        list(APPEND CMAKE_BUILD_RPATH "${CXX_RUNTIME_DIRECTORY}")
    endif ()
endif ()

if (WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif ()

option(RTYPE_BUILD_TESTS "Build r-type unit tests" ON)
option(RTYPE_BUILD_CLIENT "Build the client and Raylib" ON)
option(WITH_COVERAGE "Instrument the code and add the coverage target (Clang only)" OFF)
option(RTYPE_BUILD_BENCHMARKS "Build the benchmarks (use a Release build)" OFF)
option(RTYPE_BUILD_EXAMPLES "Build the example programs (needs RTYPE_BUILD_CLIENT)" ON)
set(RTYPE_SANITIZER "" CACHE STRING "Sanitizer to build with: empty, address or thread (GCC and Clang only)")
set_property(CACHE RTYPE_SANITIZER PROPERTY STRINGS "" address thread)

if (RTYPE_BUILD_BENCHMARKS AND NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    message(WARNING "Benchmark stats are only useful in Release. Use --preset linux-bench to build the targets")
endif ()

include(Dependencies)
include(ProjectOptions)
include(CTest)

add_subdirectory(network)

add_subdirectory(apps/server)
if (RTYPE_BUILD_CLIENT)
    add_subdirectory(apps/client)
    add_subdirectory(apps/ecs_example)
endif ()

if (RTYPE_BUILD_TESTS)
    include(Tests)
    add_subdirectory(tests)
endif ()

add_subdirectory(engine)

if (RTYPE_BUILD_CLIENT AND RTYPE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

if (RTYPE_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif ()

include(Coverage)
