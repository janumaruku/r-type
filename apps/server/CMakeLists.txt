add_library(server_logic STATIC
    src/ServerConfig.cpp
    src/RateLimits.cpp
    src/Session.cpp
)

target_link_libraries(server_logic PUBLIC project_options network engine_runtime)
target_include_directories(server_logic PUBLIC include)
