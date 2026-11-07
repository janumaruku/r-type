rtype_add_test(client_tests
    LIBS client_logic raylib
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
        AnimationTests.cpp
        RenderSystemTests.cpp
        StarfieldTests.cpp
        AudioReactorTests.cpp
        EffectsReactorTests.cpp
        HudTests.cpp
)

target_compile_definitions(client_tests PRIVATE
    RTYPE_CONFIG_DIR="${PROJECT_SOURCE_DIR}/config")