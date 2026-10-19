add_library(engine_input STATIC
        src/Key.cpp
        src/InputState.cpp
        src/ActionMap.cpp
)

target_include_directories(engine_input PUBLIC include)

target_link_libraries(engine_input PUBLIC engine_core project_options)