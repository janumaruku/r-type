add_library(engine_render STATIC
	src/RenderQueue.cpp
	src/Window.cpp
)

target_include_directories(engine_render PUBLIC include)
target_link_libraries(engine_render PUBLIC project_options engine_core PRIVATE raylib)