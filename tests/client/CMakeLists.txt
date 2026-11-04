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
)
