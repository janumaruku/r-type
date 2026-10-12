add_library(project_options INTERFACE)

target_compile_features(project_options INTERFACE cxx_std_20)

if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    target_compile_options(project_options INTERFACE /W4 /WX)
else()
    target_compile_options(project_options INTERFACE -Wall -Wextra -Werror)
endif()

if (WITH_COVERAGE)
    if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(FATAL_ERROR "WITH_COVERAGE needs Clang (got ${CMAKE_CXX_COMPILER_ID})")
    endif ()

    target_compile_options(project_options INTERFACE -fprofile-instr-generate -fcoverage-mapping)
    target_link_options(project_options INTERFACE -fprofile-instr-generate)
endif ()
