add_library(engine_runtime STATIC
        src/StopWatch.cpp
        src/FixedTimestep.cpp
        src/FixedRateTicker.cpp
)

target_include_directories(engine_runtime PUBLIC include)
target_link_libraries(engine_runtime PUBLIC engine_core project_options)
