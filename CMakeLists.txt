cmake_minimum_required(VERSION 3.28.3...4.0)

project(r-type VERSION 1.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif ()

option(RTYPE_BUILD_TESTS "Build r-type unit tests" ON)
option(RTYPE_BUILD_CLIENT "Build the client and Raylib" ON)
option(WITH_COVERAGE "Instrument the code and add the coverage target (Clang only)" OFF)

include(Dependencies)
include(ProjectOptions)
include(CTest)

add_subdirectory(network)
add_subdirectory(apps/server)
if (RTYPE_BUILD_CLIENT)
    add_subdirectory(apps/client)
endif ()

if (RTYPE_BUILD_TESTS)
    include(Tests)
    add_subdirectory(tests)
endif ()

include(Coverage)

add_subdirectory(engine)
