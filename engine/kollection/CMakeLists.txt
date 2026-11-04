add_library(engine_kollection STATIC
        src/core/Color.cpp
)

target_include_directories(engine_kollection PUBLIC include)
target_link_libraries(engine_kollection PUBLIC project_options)

add_subdirectory(core)
