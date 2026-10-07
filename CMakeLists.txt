cmake_minimum_required(VERSION 3.28.3)

project(r-type VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(warning_options INTERFACE)

if(MSVC)
    target_compile_options(warning_options INTERFACE /W4 /WX)
else()
    target_compile_options(warning_options INTERFACE -Wall -Wextra -Werror)
endif()

add_executable(r-type main.cpp)
