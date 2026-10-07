cmake_minimum_required(VERSION 3.28.3)

project(r-type VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/Dependencies.cmake)
include(CTest)

add_library(project_options INTERFACE)

target_compile_options(project_options INTERFACE -Wall -Wextra -Werror)

add_subdirectory(src/server)
add_subdirectory(src/client)
option(RTYPE_BUILD_TESTS "Build r-type unit tests" ON)

if(RTYPE_BUILD_TESTS)
	add_subdirectory(tests)
endif()
