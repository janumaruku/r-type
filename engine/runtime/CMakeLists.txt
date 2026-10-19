add_library(engine_runtime STATIC
    src/StopWatch.cpp
    src/FixedTimestep.cpp
)

target_include_directories(engine_runtime PUBLIC include)
target_link_libraries(engine_core PUBLIC project_options)
