add_library(server_logic STATIC
    src/ServerConfig.cpp
    src/RateLimits.cpp
    src/Session.cpp
)

target_link_libraries(server_logic PUBLIC project_options network engine_runtime PRIVATE CLI11::CLI11)
target_include_directories(server_logic PUBLIC include)

add_executable(r-type_server src/main.cpp)
target_link_libraries(r-type_server PUBLIC server_logic project_options)
target_include_directories(r-type_server PUBLIC include)
