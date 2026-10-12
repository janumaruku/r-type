add_library(project_options INTERFACE)

target_compile_features(project_options INTERFACE cxx_std_20)

if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    target_compile_options(project_options INTERFACE /W4 /WX)
else()
    target_compile_options(project_options INTERFACE -Wall -Wextra -Werror)
endif()