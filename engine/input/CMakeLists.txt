add_library(engine_input STATIC
        src/Key.cpp
)

target_include_directories(engine_input PUBLIC include)

target_link_libraries(engine_input PUBLIC project_options)