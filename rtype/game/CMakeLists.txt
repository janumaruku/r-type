add_library(rtype_game STATIC
        src/misc.cpp
        src/Components.cpp
        src/NetIdAllocator.cpp
)

target_include_directories(rtype_game PUBLIC include)

target_link_libraries(rtype_game PUBLIC engine_core engine_ecs engine_runtime project_options)
