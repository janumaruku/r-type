add_library(engine_core STATIC 
    src/Rect2.cpp
    src/Vec2.cpp
)

target_include_directories(engine_core PUBLIC include)
target_link_libraries(engine_core PUBLIC project_options)