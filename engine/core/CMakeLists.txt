add_library(engine_core STATIC
    src/Color.cpp
    src/Logger.cpp
    src/Rng.cpp
    src/Rect2.cpp
    src/Vec2.cpp
    src/AssetLocator.cpp
)

target_include_directories(engine_core PUBLIC include)
target_link_libraries(engine_core PUBLIC project_options)