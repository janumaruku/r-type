add_library(kollection_core STATIC
        src/Dimensions.cpp
        src/Styles.cpp
        src/BoxConstraints.cpp
)

target_include_directories(kollection_core PUBLIC include)

# Widget configs are built with partial designated initializers, which
# Clang 19+ rejects under -Wextra -Werror. The flag lives in its own target,
# linked after project_options, so it lands after -Wextra on the command line.
add_library(kollection_warnings INTERFACE)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-Wno-missing-designated-field-initializers KOLLECTION_HAS_NO_MISSING_DESIGNATED_WARNING)
if (KOLLECTION_HAS_NO_MISSING_DESIGNATED_WARNING)
    target_compile_options(kollection_warnings INTERFACE -Wno-missing-designated-field-initializers)
endif ()

target_link_libraries(kollection_core PUBLIC project_options kollection_warnings)
