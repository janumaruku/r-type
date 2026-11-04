add_library(kollection_core STATIC
    src/Dimensions.cpp
)

target_include_directories(kollection_core PUBLIC include)

target_link_libraries(kollection_core PUBLIC project_options)
