add_library(server_logic STATIC
    src/ServerConfig.cpp
    src/RateLimits.cpp
    src/Session.cpp
    src/Conversions.cpp
    src/ServerNetworkAdapter.cpp
    src/ServerCommands.cpp
    src/SnapshotBuilder.cpp
    src/EventRepeater.cpp
    src/Goodbyes.cpp
)

target_link_libraries(server_logic
    PUBLIC project_options network engine_runtime rtype_game rtype_protocol
    PRIVATE CLI11::CLI11
)

target_include_directories(server_logic PUBLIC include)

add_executable(r-type_server src/main.cpp)
target_link_libraries(r-type_server PUBLIC server_logic project_options)
target_include_directories(r-type_server PUBLIC include)

set_target_properties(r-type_server PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}")