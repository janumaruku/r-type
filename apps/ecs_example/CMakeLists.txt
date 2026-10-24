add_executable(r-type_ecs_example src/main.cpp
        src/World.cpp
        src/GameplaySystems.cpp
        src/RenderSystems.cpp
)

target_link_libraries(r-type_ecs_example PRIVATE engine_ecs raylib project_options)

set_target_properties(r-type_ecs_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}")
