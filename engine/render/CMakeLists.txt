add_library(engine_render STATIC
	src/RenderQueue.cpp
	src/Window.cpp
	src/TextureCache.cpp
	src/FontCache.cpp
)

target_include_directories(engine_render PUBLIC include)
target_link_libraries(engine_render PUBLIC engine_input engine_core project_options PRIVATE raylib)