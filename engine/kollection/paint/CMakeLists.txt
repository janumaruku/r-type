add_library(kollection_paint STATIC
        src/PaintCommands.cpp
)

target_include_directories(kollection_paint PUBLIC include)

target_link_libraries(kollection_paint PUBLIC kollection_core project_options)
