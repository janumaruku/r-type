rtype_add_test(client_tests
    LIBS client_logic
    SOURCES
        ClientDispatchTests.cpp
        ConnectingSceneTests.cpp
        GameEventBusTests.cpp
        EventFilterTests.cpp
        ClientNetworkAdapterTests.cpp
        SnapshotAssemblerTests.cpp
        ClientWorldTests.cpp
        ClientConfigTests.cpp
        ClientSettingsTests.cpp
        KeybindingsFileTests.cpp
)

target_compile_definitions(client_tests PRIVATE
    RTYPE_CONFIG_DIR="${PROJECT_SOURCE_DIR}/config")
