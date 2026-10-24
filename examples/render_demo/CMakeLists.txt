add_executable(render_demo src/main.cpp
        src/Actions.cpp
)

target_link_libraries(render_demo PRIVATE engine_render engine_input engine_core project_options)

target_compile_definitions(render_demo PRIVATE RENDER_DEMO_ASSETS="${CMAKE_CURRENT_SOURCE_DIR}/assets")
