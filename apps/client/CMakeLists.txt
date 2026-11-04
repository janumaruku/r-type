add_library(client_logic STATIC
        src/ServerLink.cpp
        src/ClientDispatch.cpp
        src/ConnectingScene.cpp
        src/GameEventBus.cpp
        src/GameCommands.cpp
        src/SnapshotAssembler.cpp
        src/ClientNetworkAdapter.cpp
        src/ClientWorld.cpp
        src/SpriteCatalog.cpp
        src/ClientConfig.cpp
        src/ClientSettings.cpp
)

target_include_directories(client_logic PUBLIC include)

target_link_libraries(client_logic PUBLIC
        engine_core
        engine_input
        engine_render
        engine_audio
        network
        rtype_protocol
        rtype_game
        nlohmann_json
        project_options
)

target_link_libraries(client_logic PRIVATE CLI11::CLI11)

add_executable(r-type_client src/main.cpp)

target_link_libraries(r-type_client PRIVATE client_logic project_options)

set_target_properties(r-type_client PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}")
