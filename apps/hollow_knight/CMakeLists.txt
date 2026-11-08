add_executable(hollow_knight
        src/Actions.cpp
        src/main.cpp
        src/Scene.cpp
        src/Systems.cpp
        src/World.cpp
)

target_include_directories(hollow_knight PRIVATE include)
target_link_libraries(hollow_knight PRIVATE
        engine_core
        engine_ecs
        engine_audio
        engine_input
        engine_render
        engine_runtime
        network
        rtype_protocol
        project_options
)

set_target_properties(hollow_knight PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}")
